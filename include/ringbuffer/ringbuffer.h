/*************************************************************************/
/* ringbuffer - a multi-reader, lock-free ringbuffer lib                 */
/* Copyright (C) 2014-2019                                               */
/* Johannes Lorenz (j.git@lorenz-ho.me, $$$=@)                           */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

#ifndef NO_CLASH_RINGBUFFER_H
#define NO_CLASH_RINGBUFFER_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <algorithm>

// let CMake define RINGBUFFER_EXPORT
#include "ringbuffer_export.h"

template<class T>
class ringbuffer_t;

//! common variables for both reader and writer
class RINGBUFFER_EXPORT ringbuffer_common_t
{
private:
	static std::size_t calc_size(std::size_t sz);
protected:
	const std::size_t size; //!< buffer size (2^n for some n)
	const std::size_t size_mask; //!< = size - 1
public:
	ringbuffer_common_t(std::size_t sz);
};

// note: the base classes contain the logic without any buffers
//       especially, they are template-free and their code can easily
//       be moved into cpp files

class RINGBUFFER_EXPORT ringbuffer_base : protected ringbuffer_common_t
{
	bool mlocked = false;

	template<class T>
	class rb_atomic
	{
		std::atomic<T> var;
	public:
		T load(std::memory_order mo = std::memory_order_acquire) const
		{
			return var.load(mo);
		}
		void store(const T& t, std::memory_order mo =
			std::memory_order_release) {
			var.store(t, mo);
		}
		rb_atomic() {}
		//! this shall only be used for construction
		rb_atomic(rb_atomic&& other) { store(other.load()); }

		rb_atomic<T>& operator--() {
			var.fetch_sub(1,
				std::memory_order_acq_rel); // TODO: ??
			return *this;
		}
	};

protected:
	rb_atomic<std::size_t> w_ptr; //!< writer at buf[w_ptr]
	//! counts number of readers left in previous buffer half
	rb_atomic<std::size_t> readers_left;
	std::size_t num_readers = 0; //!< to be const after initialisation

	using ringbuffer_common_t::ringbuffer_common_t;

	void munlock(const void* const buf, std::size_t each);
	bool mlock(const void* const buf, std::size_t each);
	void init_atomic_variables();

	void init_variables_for_write(std::size_t cnt,
		std::size_t& w, std::size_t& to_write,
		std::size_t& n1, std::size_t& n2);

public:
	//! returns number of bytes that can be written at least
	std::size_t write_space() const;
private:
	//! version for preloaded write ptr
	std::size_t write_space_preloaded(std::size_t w,
		std::size_t rl) const;
};

//! TODO: specialization for only one reader
template<class T>
class ringbuffer_t : public ringbuffer_base
{
public:
	using value_type = T;
private:

	T* const buf; // TODO: std::vector?

	template<class _T>
	friend class ringbuffer_reader_t;

public:
	// TODO: auto mlock for all allocating functions?
	// (bool auto_mlock param)
	ringbuffer_t(const ringbuffer_t& ) = delete;

	//! move ctor. should only be used in sequential mode,
	//! i.e. for initialization
	ringbuffer_t(ringbuffer_t&& other) :
		ringbuffer_base(other),
		buf(std::move(other.buf))
	{
		other.buf = nullptr;
	}

	//! allocating constructor
	//! @param sz size of buffer being allocated
	ringbuffer_t(std::size_t sz) :
		ringbuffer_base(sz),
		buf(new T[ringbuffer_common_t::size])
	{
		if(! buf)
		 throw std::bad_alloc(); // TODO: should new not throw this?
		//if(! buf)
		// throw "Error allocting ringbuffer.";
		init_atomic_variables();
	}
	~ringbuffer_t() { munlock(buf, size * sizeof(T)); delete[] buf; }

	// TODO: make constexpr if size is
	//! size that is guaranteed to be writable one all readers
	//! are up to date
	std::size_t maximum_eventual_write_space() const {
		// TODO: might be (size >> 1 + 1), not sure
		return size >> 1;
	}

	//! writes max(cnt, write_space) of src into the buffer
	//! overwrite this function in your subclass if you do not use
	//! the standard copier `std_copy`
	//! @return number of bytes successfully written
	std::size_t write(const T *src, size_t cnt) {
		std_copy func(src);
		return write_func<std_copy>(func, cnt);
	}

	template<class Func>
	std::size_t write_func(Func& f, size_t cnt)
	{
		std::size_t w, to_write;
		std::size_t n1, n2;
		init_variables_for_write(cnt, w, to_write, n1, n2);

		//std::copy_n(src, n1, &(buf[w]));
		f(0, n1, buf + w);
		w = (w + n1) & size_mask;
		// update so readers are already informed:
		w_ptr.store(w);

		if (n2) {
			//std::copy_n(src + n1, n2, &(buf[w]));
			f(n1, n2, buf + w);
			w = (w + n2) & size_mask;
			w_ptr.store(w);
		}

		return to_write;
	}

	//! Standard copier for `write_func`
	//! Works for all `T` that are copyable (e.g. that have a copy CTOR)
	class std_copy
	{
		const T* const src;
	public:
		void operator()(std::size_t src_off, std::size_t amnt,
			T* dest)
		{
			std::copy_n(src + src_off, amnt, dest);
		}
		std_copy(const T* arg_src) : src(arg_src) {}
	};

	//! try to lock the data block using the syscall @a block
	//! @return true iff mlock() succeeded, i.e. pages are in RAM
	bool mlock() { return ringbuffer_base::mlock(buf, size * sizeof(T)); }

	//! overwrite the whole buffer with zeros
	//! this prevents page faults
	//! only allowed on startup (this is not fully checked!)
	void touch()
	{
		// exclude most situations where the ringbuffer is already filled
		assert(w_ptr.load() == 0);
		assert(readers_left.load() == 0);
		std::fill_n(reinterpret_cast<char*>(buf), size*sizeof(T), '\0');
	}
};

namespace detail
{

//! returns @a i2 if @a i1 is true, otherwise 0
template<class T2>
constexpr T2 if_than_or_zero(const bool& i1, const T2& i2) {
	return (-(static_cast<int>(i1))) & i2;
}

}

class RINGBUFFER_EXPORT ringbuffer_reader_base : protected ringbuffer_common_t
{
protected:
	std::size_t read_ptr = 0; //!< reader at buf[read_ptr]

	ringbuffer_reader_base(std::size_t sz);

	//! returns number of bytes that can be read at least
	std::size_t read_space(std::size_t w) const;
	//! returns read space of first halve (the one starting at read ptr)
	std::size_t read_space_1(std::size_t range) const;
	//! returns read space of second halve
	std::size_t read_space_2(std::size_t range) const;
};

template<class T>
class ringbuffer_reader_t : public ringbuffer_reader_base
{
	const T* buf; // This is only read by seq_base // TODO: redundant to ref->buf?
	ringbuffer_t<T>* ref;

	//! sequences help reading by providing a ringbuffer-suited operator[]
	template<class rb_ptr_type>
	class seq_base
	{
		const T* const buf;
		std::size_t range;
	protected:
		rb_ptr_type reader_ref;
	public:
		//! requests a read sequence of size range
		//! TODO: two are invalid -> ???
		seq_base(rb_ptr_type rb, std::size_t arg_range) :
			buf(rb->buf),
			range(arg_range),
			reader_ref(rb)
		{
		}

		seq_base(const seq_base& other) = delete;
		seq_base(seq_base&& ) = default;

		//! single member access
		const T& operator[](std::size_t idx) const {
			return *(buf + ((reader_ref->read_ptr + idx) &
				reader_ref->size_mask));
		}

		std::size_t size() const { return range; }

		const T* first_half_ptr() const {
			return buf + reader_ref->read_ptr; }
		const T* second_half_ptr() const { return buf; }
		std::size_t first_half_size() const {
			//const ringbuffer_t<T>& rb = *reader_ref->ref;
			return reader_ref->read_space_1(range);
		}
		std::size_t second_half_size() const {
			//const ringbuffer_t<T>& rb = *reader_ref->ref;
			return reader_ref->read_space_2(range);
		}

		//! copy the current sequence into a buffer
		bool copy(char* buffer, size_t bsize)
		{
			std::size_t h1 = first_half_size(),
				h2 = second_half_size();
			if(h1 + h2 < bsize) {
				return false;
			} else {
				std::copy(first_half_ptr(),
					first_half_ptr() + h1, buffer);
				std::copy(second_half_ptr(),
					second_half_ptr() + h2, buffer + h1);
				return true;
			}
		}
	};

	std::size_t _read_max_spc(std::size_t range) const {
		return std::min(read_space(), range);
	}

	static_assert(detail::if_than_or_zero(1, 42) == 42,
		"if_than_or_zero fails with i1 == true");
	static_assert(detail::if_than_or_zero(0, 42) == 0,
		"if_than_or_zero fails with i1 == false");

	std::size_t _read_spc(std::size_t range) const {
		// equal to: read_space() >= range ? range : 0;
		return detail::if_than_or_zero(read_space() >= range, range);
	}

	//! increases the @a read_ptr after reading from the buffer
	void try_inc(std::size_t range)
	{
		const std::size_t old_read_ptr = read_ptr;

		read_ptr = (read_ptr + range) & size_mask;
		// TODO: inefficient xor
		// checks if highest bit flipped:
		if((read_ptr ^ old_read_ptr) & (size >> 1))
		{
			--ref->readers_left;
		}
	}

public:
	class peak_sequence_t : public seq_base<const ringbuffer_reader_t<T>*>
	{
	public:
		// TODO: are template args required here?
		using seq_base<const ringbuffer_reader_t<T>*>::seq_base;

		peak_sequence_t(peak_sequence_t&& ) = default;
	};

	class read_sequence_t : public seq_base<ringbuffer_reader_t<T>*>
	{
	public:
		using seq_base<ringbuffer_reader_t<T>*>::seq_base;

		//! increases the read_ptr after reading
		~read_sequence_t() {
			seq_base<ringbuffer_reader_t<T>*>::reader_ref->
				try_inc(seq_base<ringbuffer_reader_t<T>*>::size());
		}

		read_sequence_t(read_sequence_t&& ) = default;
	};

	//! constuctor. registers this reader at the ringbuffer
	//! @note careful: this function is @a not thread-safe
	ringbuffer_reader_t(ringbuffer_t<T> &arg_ref) :
		ringbuffer_reader_base(arg_ref.size), buf(arg_ref.buf), ref(&arg_ref)
	{
		++arg_ref.num_readers; // register at the writer
	}

	//! constuctor. no registration yet
	//! thread safe
	ringbuffer_reader_t(std::size_t sz) :
		ringbuffer_reader_base(sz),
		buf(nullptr),
		ref(nullptr) {}

	//! @note careful: this function is @a not thread-safe
	void connect(ringbuffer_t<T>& _ref)
	{
		if(size != _ref.size)
		 throw "connecting ringbuffers of incompatible sizes";
		else {
			buf = _ref.buf;
			ref = &_ref;
			++_ref.num_readers; // register at the writer
		}
	}

	//! reads min(@a range, @a read_space()) bytes
	read_sequence_t read_max(std::size_t range =
		std::numeric_limits<std::size_t>::max()) {
		return read_sequence_t(this, _read_max_spc(range));
	}
	
	//! reads @a range bytes if @a range <= @a read_space(), otherwise 0
	read_sequence_t read(std::size_t range) {
		return read_sequence_t(this, _read_spc(range));
	}

	//! peaks min(@a range, @a read_space()) bytes
	peak_sequence_t peak_max(std::size_t range =
		std::numeric_limits<std::size_t>::max()) const {
		return peak_sequence_t(this, _read_max_spc(range));
	}

	//! peaks @a range bytes if @a range <= @a read_space(), otherwise 0
	peak_sequence_t peak(std::size_t range) const {
		return peak_sequence_t(this, _read_spc(range));
	}

	//! returns number of bytes that can be read at least
	std::size_t read_space() const {
		std::size_t w =
			ref->w_ptr.load();
		return ringbuffer_reader_base::read_space(w);
	}

	//! return the size that the reader expects from the ringbuffer
	std::size_t get_size() const { return size; }
};

#endif // NO_CLASH_RINGBUFFER_H

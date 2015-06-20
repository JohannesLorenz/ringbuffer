/*************************************************************************/
/* ringbuffer - a multi-reader, lock-free ringbuffer lib                 */
/* Copyright (C) 2014-2015                                               */
/* Johannes Lorenz (jlsf2013 @ sourceforge)                              */
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

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <atomic>
#include <cstddef>
#include <algorithm>

#include "config.h"

class ringbuffer_t;

//! common variables for both reader and writer
class ringbuffer_common_t
{
private:
	static std::size_t calc_size(std::size_t sz);
protected:
	const std::size_t size; //!< buffer size (2^n for some n)
	const std::size_t size_mask; //!< = size - 1
public:
	ringbuffer_common_t(std::size_t sz);
};

//! TODO: specialization for only one reader
class ringbuffer_t : protected ringbuffer_common_t
{
	template<class T>
	class rb_atomic
	{
		std::atomic<T> var;
	public:
		T load() const { return var.load(std::memory_order_relaxed); }
		void store(const T& t) {
			var.store(t, std::memory_order_relaxed); }
		rb_atomic() {}
		rb_atomic(rb_atomic&& other) { store(other.load()); }
		T operator--() { return --var; }
	};

	rb_atomic<std::size_t> w_ptr; //!< writer at buf[w_ptr]
	//! counts number of readers left in previous buffer half
	rb_atomic<std::size_t> readers_left;
	std::size_t num_readers = 0; //!< to be const after initialisation

#ifdef USE_MLOCK
	bool mlocked = false;
#endif
	char* const buf; // TODO: std::vector?

	friend class ringbuffer_reader_t;

	//! version for preloaded write ptr
	std::size_t write_space_preloaded(std::size_t w,
		std::size_t rl) const;

public:
	ringbuffer_t(const ringbuffer_t& other) = delete;

	//! move ctor. should only be used in sequential mode,
	//! i.e. for initialization
	ringbuffer_t(ringbuffer_t&& ) = default;

	//! allocating constructor
	//! @param sz size of buffer being allocated
	ringbuffer_t(std::size_t sz);
	~ringbuffer_t();

	// TODO: make constexpr if size is
	//! size that is guaranteed to be writable one all readers
	//! are up to date
	std::size_t maximum_eventual_write_space() const {
		// TODO: might be (size >> 1 + 1), not sure
		return size >> 1;
	}

	//! returns number of bytes that can be written at least
	std::size_t write_space() const;
	//! writes max(cnt, write_space) of src into the buffer
	//! @return number of bytes successfully written
	std::size_t write(const char *src, size_t cnt);

	//! trys to lock the data block using the syscall @a block
	//! @return true iff mlock() succeeded, i.e. pages are in RAM
	bool mlock();

	//! overwrites the whole buffer with zeros
	//! this prevents page faults
	//! only allowed on startup (this is not checked!)
	void touch();
};

namespace detail
{

//! returns @a i2 if @a i1 is true, otherwise 0
template<class T2>
constexpr T2 if_than_or_zero(const bool& i1, const T2& i2) {
	return (-((int)i1)) & i2;
}

}

class ringbuffer_reader_t : protected ringbuffer_common_t
{
	const char* buf;
	ringbuffer_t* ref;
	std::size_t read_ptr = 0; //!< reader at buf[read_ptr]

	//! increases the @a read_ptr after reading from the buffer
	void try_inc(std::size_t range);

	// TODO: offer first_half_ptr(), first_half_size(), ...
	template<class rb_ptr_type>
	class seq_base
	{
		const char* const buf;
		std::size_t range;
	protected:
		rb_ptr_type reader_ref;
	public:
		//! requests a read sequence of size range
		//! TODO: two are invalid
		seq_base(rb_ptr_type rb, std::size_t range) :
			buf(rb->buf),
			range(range),
			reader_ref(rb)
		{
		}

		//! single member access
		const char& operator[](std::size_t idx) {
			return *(buf + ((reader_ref->read_ptr + idx) &
				reader_ref->size_mask));
		}

		std::size_t size() const { return range; }

		//const char* first_half_ptr() const { return TODO; }
		const char* second_half_ptr() const { return buf; }
		//std::size_t second_half_size() const { return TODO }
	};

	class peak_sequence_t : public seq_base<const ringbuffer_reader_t*> {
	public:
		using seq_base::seq_base;
	};

	class read_sequence_t : public seq_base<ringbuffer_reader_t*> {
	public:
		using seq_base::seq_base;
		//! increases the read_ptr after reading
		~read_sequence_t();
	};

	std::size_t _read_max_spc(std::size_t range) const {
		std::size_t rs = read_space();
		return std::min(rs, range);
	}

	static_assert(detail::if_than_or_zero(1, 42) == 42,
		"if_than_or_zero fails with i1 == true");
	static_assert(detail::if_than_or_zero(0, 42) == 0,
		"if_than_or_zero fails with i1 == false");

	std::size_t _read_spc(std::size_t range) const {
		std::size_t rs = read_space();
		// equal to: rs < range ? 0 : range;
		return detail::if_than_or_zero(rs >= range, range);
	}

public:
	//! constuctor. registers this reader at the ringbuffer
	//! @note careful: this function is @a not thread-safe
	ringbuffer_reader_t(ringbuffer_t& ref);

	//! constuctor. no registration yet
	//! thread safe
	ringbuffer_reader_t(std::size_t sz);

	//! @note careful: this function is @a not thread-safe
	void connect(ringbuffer_t &ref);
	
	//! reads max(@a range, @a read_space()) bytes
	read_sequence_t read_max(std::size_t range) {
		return read_sequence_t(this, _read_max_spc(range));
	}
	
	//! reads @a range bytes if @a range <= @a read_space(), otherwise 0
	read_sequence_t read(std::size_t range) {
		return read_sequence_t(this, _read_spc(range));
	}

	//! peaks max(@a range, @a read_space()) bytes
	peak_sequence_t peak_max(std::size_t range) const {
		return peak_sequence_t(this, _read_max_spc(range));
	}

	//! peaks @a range bytes if @a range <= @a read_space(), otherwise 0
	peak_sequence_t peak(std::size_t range) const {
		return peak_sequence_t(this, _read_spc(range));
	}

	//! returns number of bytes that can be read at least
	std::size_t read_space() const;
};

#endif // RINGBUFFER_H

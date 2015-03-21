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

#define USE_MLOCK // TODO: config.h

class ringbuffer_t;

class ringbuffer_common_t
{
private:
	static std::size_t calc_size(std::size_t sz);
protected:
	const std::size_t size;
	const std::size_t size_mask;
public:
	ringbuffer_common_t(std::size_t sz);
	~ringbuffer_common_t();
};

//! TODO: specialization for only one reader
class ringbuffer_t : protected ringbuffer_common_t
{
	std::atomic<std::size_t> w_ptr;
	//! counts number of readers left in previous buffer half
	std::atomic<std::size_t> readers_left;
	std::size_t num_readers = 0; //!< to be const after initialisation

#ifdef USE_MLOCK
	bool mlocked = false;
#endif
	char* const buf;

	friend class ringbuffer_reader_t;

	//! version for preloaded write ptr
	std::size_t write_space_preloaded(std::size_t w,
		std::size_t rl) const;

public:
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

	//! returns how much space is left to write at least
	std::size_t write_space() const;
	//! writes max(cnt, write_space) of src into the buffer
	//! @return number of bytes successfully written
	std::size_t write(const char *src, size_t cnt);

	//! trys to lock the data block using the syscall @a block
	//! @return true iff mlock() succeeded, i.e. pages are in RAM
	bool mlock();
};

class ringbuffer_reader_t : protected ringbuffer_common_t
{
	const char* const buf;
	ringbuffer_t* const ref;
	std::size_t read_ptr = 0;

	std::size_t read(char *dest, std::size_t cnt);

	void try_inc(std::size_t range);

	// TODO: offer first_half_ptr(), first_half_size(), ...
	class seq_base
	{
		const char* const buf;
		std::size_t range;
	protected:
		ringbuffer_reader_t* reader_ref;
	public:
		//! requests a read sequence of size range
		//! TODO: must check read_space!
		//! TODO: two are invalid
		seq_base(ringbuffer_reader_t& rb, std::size_t range) :
			buf(rb.buf),
			range(range),
			reader_ref(&rb)
		{
		}

		//! single member access
		const char& operator[](std::size_t idx) {
			return *(buf + ((reader_ref->read_ptr + idx) &
				reader_ref->size_mask));
		}

		std::size_t size() const { return range; }
	};

	class peak_sequence_t : public seq_base {
	public:
		using seq_base::seq_base;
	};
	class read_sequence_t : public seq_base {
	public:
		using seq_base::seq_base;
		~read_sequence_t();
	};

public:
	//! constuctor. registers this reader at the ringbuffer
	//! @note careful: this function is @a not thread-safe
	ringbuffer_reader_t(ringbuffer_t& ref);

	template<class Sequence>
	Sequence _read_max(std::size_t range) {
		std::size_t rs = read_space();
		std::size_t rs2 = rs < range ? rs : range;
		return Sequence(*this, rs2);
	}

	template<class Sequence>
	Sequence _read(std::size_t range) {
		std::size_t rs = read_space();
		std::size_t rs2 = rs < range ? 0 : range;
		return Sequence(*this, rs2);
	}

	read_sequence_t read_max(std::size_t range) {
		return _read_max<read_sequence_t>(range);
	}

	read_sequence_t read(std::size_t range) {
		return _read<read_sequence_t>(range);
	}

	peak_sequence_t peak_max(std::size_t range) {
		return _read_max<peak_sequence_t>(range);
	}

	peak_sequence_t peak(std::size_t range) {
		return _read<peak_sequence_t>(range);
	}

	std::size_t read_space() const;
};

#endif // RINGBUFFER_H

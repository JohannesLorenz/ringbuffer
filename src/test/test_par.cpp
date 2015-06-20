/*************************************************************************/
/* test.cpp - test files for minimal                                     */
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

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <thread>
#include <algorithm>
#include "../lib/ringbuffer.h"

void init_random() { srandom(42); }

char random_number(char max) {
	return (char)(random() % max);
}

#ifdef __clang__
	#define REALTIME __attribute__((annotate("realtime")))
#else
	#define REALTIME // replace with "nothing"
#endif

// TODO: test ints (4 bytes)
using m_reader_t = ringbuffer_reader_t<char>;
using m_buffer_t = ringbuffer_t<char>;

void REALTIME read_messages(m_reader_t* _rd)
{
	m_reader_t& rd = *_rd;
	unsigned char r = 0;
	do
	{
		while(!rd.read_space())
		;

		{
			r = rd.read_max(1)[0];
		}

		while(rd.read_space() < r)
		;

		{
			auto seq = rd.read_max(r);
			for(std::size_t x = 0; x < r; ++x) {
				//std::cerr << x << ": " << seq[x] << std::endl;
				assert(seq[x] == r);
			}
		}
	} while(r);

}

//[[annotate("realtime")]] // TODO
void REALTIME write_messages(m_buffer_t* rb, const std::vector<char>& random_numbers)
{
	char tmp_buf[64];
	for(std::size_t count = 0; count < random_numbers.size(); ++count)
	{
		char r = random_numbers[count]; // TODO: itr
		//random_number(rb->maximum_eventual_write_space() - 1) + 1;

		// spin locks are no good idea here
		// this is just for demonstration
		while(rb->write_space() <= (unsigned char)r)
		;

		std::fill_n(tmp_buf, r+1, r);
		assert(rb->write(tmp_buf, r+1) == (unsigned char)(r+1)); // write r r+1 times
	}
	char r = 0;
	rb->write(&r, 1);
}

int main()
{
	init_random();

	m_buffer_t rb(64);
	constexpr std::size_t n_readers = 2;
	m_reader_t rd[n_readers] = { rb, rb };
	rb.mlock();

	constexpr std::size_t max = 10000;
	std::vector<char> random_numbers(max + 1);
	for(std::size_t count = 0; count < max; ++count)
	{
		random_numbers[count] = random_number(rb.maximum_eventual_write_space() - 1) + 1;
	}
	random_numbers[max] = 0;

	try
	{
		std::thread t1(write_messages, &rb, random_numbers);
		std::thread t2(read_messages, rd);
		std::thread t3(read_messages, rd + 1);
		t1.join(); t2.join(); t3.join();
	}
	catch(const char* s)
	{
		std::cerr << s << std::endl;
		return 1;
	}

	return 0;
}



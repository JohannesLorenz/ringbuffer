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

#include <cassert>
#include "ringbuffer.h"

int main()
{
	try {
		// test the ringbuffer
		ringbuffer_t rb(4);
		ringbuffer_reader_t rd(rb);

		std::size_t n = rb.write("abcd", 5);
		std::cerr << "written: " << n << std::endl;
		assert(n==3);
		assert(!rb.write_space());
		// simulate impossible write
		assert(!rb.write("xyz", 4));
		{
			auto s = rd.read_sequence(3); // TODO: what if rs is too high?
			std::cerr << +s[0] << ' ' << +s[1] << ' '  << +s[2] << std::endl;
			assert(s[0] == 97 && s[1] == 98 && s[2] == 99);

			// TODO: check read space == 0
		}
		assert(rb.write("ab", 2) == 2);
		assert(rb.write_space() == 0);
		{
			auto s = rd.read_sequence(1);
			std::cerr << +s[0] << std::endl;
			assert(s[0] == 97);
		}
		{
			auto s2 = rd.read_sequence(1);
			std::cerr << +s2[0] << std::endl;
			assert(s2[0] == 98);
		}
		assert(rb.write_space() == 2);
		rb.write("x", 1);
		assert(rb.write_space() == 1);
		{
			auto s = rd.read_sequence(1);
			std::cerr << +s[0] << std::endl;
			assert(s[0] == 120);
		}
		assert(rb.write_space() == 3);

	} catch (const char* s)
	{
		std::cerr << s << std::endl;
	}

	return 0;
}


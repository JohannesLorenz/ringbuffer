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

#include <algorithm>

#include <stdlib.h>
#include <string.h>
#include <limits>

#include "ringbuffer.h"

ringbuffer_t::ringbuffer_t(std::size_t sz) :
	ringbuffer_common_t(sz),
	buf(new char[ringbuffer_common_t::size])
{
	w_ptr.store(0, std::memory_order_relaxed);
	readers_left.store(0, std::memory_order_relaxed);
//	iteration.store(0, std::memory_order_relaxed);
	//buf = new char[size];
//	mlocked = 0;
}

ringbuffer_t::~ringbuffer_t()
{
/*	if (rb->mlocked) {
		munlock (rb->buf, rb->size);
	}*/
	delete[] buf;
}

std::size_t ringbuffer_reader_t::read_space() const // TODO: unused?! correct?
{
	const std::size_t
		w = ref->w_ptr.load(std::memory_order_relaxed),
		r = read_ptr;

	if (w > r) {
		return w - r;
	} else {
		return (w - r + ref->size) & ref->size_mask;
	}
}

// TODO: w_ptr: variable names

std::size_t ringbuffer_t::write_space() const
{
	// case 1: at least one r is still in the other half
	//  => write_space is end of this half

	// case 2: all r are already in the half of w
	//  => write_space is end of this half + other half

	std::cerr << "size: " << size << ", wl: " << w_ptr.load(std::memory_order_relaxed) << ", size_mask: " << size_mask << std::endl;
	std::cerr << "rl: " << readers_left.load(std::memory_order_relaxed) << std::endl;
	// TODO: was: ((size - w_ptr.load(std::memory_order_relaxed) & (size_mask)))
	std::cerr << "WS: " <<
		(((size_mask - w_ptr.load(std::memory_order_relaxed)) & (size_mask >> 1)))
		<< "+" << ((readers_left.load(std::memory_order_relaxed) == false) * (size >> 1)) << std::endl;

	return (((size_mask - w_ptr.load(std::memory_order_relaxed)) & (size_mask >> 1))) // = way to before next half
		+ ((readers_left.load(std::memory_order_relaxed) == false) * (size >> 1)) // one more block
		;
}

std::size_t ringbuffer_t::write (const char *src, size_t cnt)
{

	std::size_t free_cnt;
	std::size_t cnt2;
	std::size_t to_write;
	std::size_t n1, n2;

	if ((free_cnt = write_space()) == 0) {
		return 0;
	}

	to_write = cnt > free_cnt ? free_cnt : cnt;

	cnt2 = w_ptr.load(std::memory_order_relaxed) + to_write;

	if (cnt2 > size) {
		n1 = size - w_ptr.load(std::memory_order_relaxed);
		n2 = cnt2 & size_mask;
	} else {
		n1 = to_write;
		n2 = 0;
	}

	// here starts the writing

	const std::size_t old_w_ptr = w_ptr;

	memcpy (&(buf[w_ptr]), src, n1);
	w_ptr.store((w_ptr + n1) & size_mask, std::memory_order_relaxed);

	if (n2) {
		memcpy (&(buf[w_ptr]), src + n1, n2);
		w_ptr.store((w_ptr + n2) & size_mask, std::memory_order_relaxed);
	}

	std::cerr << "wl, old wl: " << w_ptr << old_w_ptr << std::endl;

	// TODO: inefficient or:
	if((w_ptr ^ old_w_ptr) & (size >> 1) || to_write == size) // highest bit flipped
	{
		if(readers_left)
		 throw "impossible";
		std::cerr << "resetting readers left..." << std::endl;
		readers_left = num_readers;
	}

	return to_write;
}

std::size_t ringbuffer_common_t::calc_size(std::size_t sz)
{
	std::size_t power_of_two;
	for (power_of_two = 1;
		((std::size_t)1 << power_of_two) < sz; power_of_two++);
	return 1 << power_of_two;
}

ringbuffer_common_t::ringbuffer_common_t(std::size_t sz) :
	size(calc_size(sz)),
	size_mask(size - 1)/*,
	buf(new char[size])*/
{}

ringbuffer_common_t::~ringbuffer_common_t() { /*delete[] buf;*/ }



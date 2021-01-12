/*
 * libopenraw - endianutils.hpp
 *
 * Copyright (C) 2006-2020 Hubert Figuiere
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#define EL16(b) \
  ((b)[0] | ((b)[1] << 8))

#define BE16(b) \
  ((b)[1] | ((b)[0] << 8))

#define EL32(b) \
  ((b)[0] | ((b)[1] << 8) | ((b)[2] << 16) | ((b)[3] << 24))

#define BE32(b) \
  ((b)[3] | ((b)[2] << 8) | ((b)[1] << 16) | ((b)[0] << 24))

#ifdef __APPLE__

// Linux and BSD have these.
// macOS need its own.

#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)

#include <sys/endian.h>

#else

#include <endian.h>

#endif

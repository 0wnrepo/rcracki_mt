/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010 James Nobis <frt@quelrod.net>
 *
 * This file is part of rcracki_mt.
 *
 * rcracki_mt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * rcracki_mt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H

//#include <stdint.h>

#if defined(_WIN32) && !defined(__GNUC__)
	#define uint64 unsigned __int64
#else
	#ifndef u_int64_t
		#define uint64 unsigned long long
	#else
		#define uint64 u_int64_t
	#endif
#endif

#if defined(_WIN32) && !defined(__GNUC__)
	#define UINT4 unsigned __int32
#else
	#ifndef u_int32_t
		#define UINT4 unsigned int
	#else
		#define UINT4 u_int32_t
	#endif
#endif

#endif /* !GLOBAL_H */

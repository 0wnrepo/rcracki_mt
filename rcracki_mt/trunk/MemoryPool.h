/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010 James Nobis <frt@quelrod.net>
 * Copyright 2010 uroskn
 *
 * This file is part of racrcki_mt.
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

#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include "global.h"

class CMemoryPool  
{
public:
	CMemoryPool(unsigned int bytesSaved, bool bDebug, uint64 maxMem);
	virtual ~CMemoryPool();

private:
	bool debug;
	unsigned char* m_pMem;
	uint64 m_nMemSize;

	uint64 m_nMemMax;

public:
	unsigned char* Allocate(unsigned int nFileLen, uint64& nAllocatedSize);
};

#endif

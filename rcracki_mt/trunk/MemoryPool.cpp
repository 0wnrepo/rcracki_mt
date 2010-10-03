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

#include "MemoryPool.h"
#include "Public.h"

CMemoryPool::CMemoryPool(unsigned int bytesSaved, bool bDebug, uint64 maxMem)
{
	m_pMem = NULL;
	m_nMemSize = 0;
	debug = bDebug;

	uint64 nAvailPhys = GetAvailPhysMemorySize();

	if ( debug )
	{
		printf( "Debug: nAvailPhys: %llu\n", nAvailPhys );
		printf( "Debug: bytesSaved: %d\n", bytesSaved );
	}

	if ( maxMem > 0 && maxMem < nAvailPhys )
		nAvailPhys = maxMem;
	
	m_nMemMax = nAvailPhys - bytesSaved;	// Leave memory for CChainWalkSet	

	if (m_nMemMax < 16 * 1024 * 1024)
		m_nMemMax = 16 * 1024 * 1024;
}

CMemoryPool::~CMemoryPool()
{
	if (m_pMem != NULL)
	{
		delete [] m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}
}

unsigned char* CMemoryPool::Allocate(unsigned int nFileLen, uint64& nAllocatedSize)
{
	if (nFileLen <= m_nMemSize)
	{
		nAllocatedSize = nFileLen;
		return m_pMem;
	}

	unsigned int nTargetSize;
	if (nFileLen < m_nMemMax)
		nTargetSize = nFileLen;
	else
		nTargetSize = m_nMemMax;

	// Free existing memory
	if (m_pMem != NULL)
	{
		delete [] m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}

	// Allocate new memory
	//printf("allocating %u bytes memory\n", nTargetSize);
	m_pMem = new (nothrow) unsigned char[nTargetSize];
	while (m_pMem == NULL && nTargetSize >= 32 * 1024 * 1024 )
	{
	   nTargetSize -= 16 * 1024 * 1024;
	   m_pMem = new (nothrow) unsigned char[nTargetSize];
	}

	if (m_pMem != NULL)
	{
		m_nMemSize = nTargetSize;
		nAllocatedSize = nTargetSize;
		return m_pMem;
	}
	else
	{
		m_nMemSize = 0;
		nAllocatedSize = 0;
		return NULL;
	}
}

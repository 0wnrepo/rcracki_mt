/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2010 James Nobis <frt@quelrod.net>
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

#include "RTI2Reader.h"

#include <math.h>

RTI2Header *RTI2Reader::m_pHeader = NULL;
RTI2Reader::RTI2Reader(string Filename)
{
	//m_pIndexPos = NULL, m_pChainPos = NULL;;
	m_pIndex = NULL;
	m_pFile = fopen(Filename.c_str(), "rb");
	if(m_pFile == NULL)
	{
		printf("Unable to open file %s", Filename.c_str());
		exit(1);
	}
	FILE *pFileIndex = fopen(Filename.append(".index").c_str(), "rb");
	if(pFileIndex == NULL)
	{
		printf("Unable to open file %s", Filename.append(".index").c_str());
		exit(1);
	}
	m_chainPosition = 0;

	long len = GetFileLen(pFileIndex);
	fseek(pFileIndex, 0, SEEK_SET);

	m_pIndex = new (nothrow) unsigned char[len];
	if(m_pIndex == NULL) {
		printf("Error allocating %ld MB memory for index in RTI2Reader::RTI2Reader()", len / (1024 * 1024));
		exit(-2);
	}
	if(fread(m_pIndex, 1, len, pFileIndex) != (unsigned long)len)
	{
		printf("Error while reading index file");
		exit(1);
	}
	fclose(pFileIndex);
	m_pHeader = new RTI2Header();	
	memcpy(m_pHeader, m_pIndex, sizeof(RTI2Header));
	m_pHeader->m_cppos = (unsigned int*)(m_pIndex + 8);
	m_pHeader->prefixstart = *(uint64*)(m_pIndex + 8 + (m_pHeader->rti_cplength * 4));
	m_chainsizebytes = (uint32)ceil((float)(m_pHeader->rti_startptlength + m_pHeader->rti_endptlength + m_pHeader->rti_cplength) / 8); // Get the size of each chain in bytes
	m_indexrowsizebytes = (uint32)ceil((float)m_pHeader->rti_index_numchainslength / 8);
	// Check the filesize
	fseek(m_pFile, 0, SEEK_END);
	len = ftell(m_pFile);
	fseek(m_pFile, 0, SEEK_SET);
	if(len % m_chainsizebytes > 0)
	{
		printf("Invalid filesize %ld\n", len);
		return;
	}
	

}

RTI2Reader::~RTI2Reader(void)
{
	if(m_pIndex != NULL) delete m_pIndex;
	if(m_pFile != NULL) fclose(m_pFile);

}

unsigned int RTI2Reader::GetChainsLeft()
{
	long len = GetFileLen(m_pFile);
	return len / m_chainsizebytes - m_chainPosition;
}

int RTI2Reader::ReadChains(unsigned int &numChains, RainbowChainO *pData)
{
	if(strncmp(m_pHeader->header, "RTI2", 4) != 0)
	{
		numChains = 0;
		return -1;
	}
	unsigned char *pNumChains = m_pIndex + (m_pHeader->rti_cplength * 4) + 16; // Pointer into the index containing info about how many numbers are in the first chain prefix
	unsigned int i = 0;
	unsigned int indexRow = 0; // Current offset into the index
	unsigned int curRowPosition = 0;
	
	while(true) // Fast forward to current position
	{
		// ALERT: Possible problem here if m_indexrowsizebytes > 1 as pNumChains is a unsigned char.
		unsigned int NumChainsInRow = (unsigned int)*(pNumChains + indexRow * m_indexrowsizebytes);
		if(m_indexrowsizebytes > 1)
		{
			//XXX Have to find a solution to this problem
			printf( "FATAL: m_indexrowsizebytes > 1: %d\n", m_indexrowsizebytes ); 
			exit(2);
		}
		if(i + NumChainsInRow > m_chainPosition)
		{
			curRowPosition = m_chainPosition - i;
			break; // The current position is somewhere within this prefix
		}
		indexRow++;		
		i += NumChainsInRow;
	}
	
	uint64 chainrow = 0; // Buffer to store a single read chain
	unsigned int chainsProcessed = 0; // Number of chains processed

	// XXX: same problem with unsigned char here.
	unsigned int NumChainsInRow = *(pNumChains + indexRow);
	while(chainsProcessed < numChains && fread(&chainrow, 1, m_chainsizebytes, m_pFile) == m_chainsizebytes)
	{
		if(curRowPosition >= NumChainsInRow)
		{ // Skip to next index row position
			indexRow++;
			curRowPosition = 0;
			NumChainsInRow = *(pNumChains + indexRow);
		}
		while(NumChainsInRow == 0) // We skip forward until we hit a index with > 0 chains
		{
			indexRow++;
			NumChainsInRow = *(pNumChains + indexRow);
			curRowPosition = 0;
		}
		// Load the starting point from the data
		pData[chainsProcessed].nIndexS = chainrow << ( 64 - m_pHeader->rti_startptlength );
		pData[chainsProcessed].nIndexS = pData[chainsProcessed].nIndexS >> ( 64 - m_pHeader->rti_startptlength );

		// Load the ending point prefix	
		pData[chainsProcessed].nIndexE = ( m_pHeader->prefixstart + indexRow ) << m_pHeader->rti_endptlength;
		// Append the ending point suffix
#if defined(_WIN32) && !defined(__GNUC__)
		pData[chainsProcessed].nIndexE |= (chainrow & (0xFFFFFFFFFFFFFFFFI64 >> m_pHeader->rti_cplength)) >> m_pHeader->rti_startptlength;
#else
		pData[chainsProcessed].nIndexE |= (chainrow & (0xFFFFFFFFFFFFFFFFllu >> m_pHeader->rti_cplength)) >> m_pHeader->rti_startptlength;
#endif
		//pData[chainsProcessed].nCheckPoint = (chainrow >> m_pHeader->rti_startptlength + m_pHeader->rti_endptlength);
		curRowPosition++;
		chainsProcessed++;
	}
	numChains = chainsProcessed;
	m_chainPosition += numChains;
	return 0;
}

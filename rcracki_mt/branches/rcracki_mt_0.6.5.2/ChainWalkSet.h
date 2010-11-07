/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
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

#ifndef _CHAINWALKSET_H
#define _CHAINWALKSET_H

#include "Public.h"

struct ChainWalk
{
	unsigned char Hash[MAX_HASH_LEN];
	//int nHashLen;		// Implied
	uint64* pIndexE;	// mapStartPosIndexE, Len = nRainbowChainLen - 1
};

class CChainWalkSet
{
public:
	CChainWalkSet();
	virtual ~CChainWalkSet();

private:
	string m_sHashRoutineName;		// Discard all if not match
	string m_sPlainCharsetName;		// Discard all if not match
	int    m_nPlainLenMin;			// Discard all if not match
	int    m_nPlainLenMax;			// Discard all if not match
	int    m_nRainbowTableIndex;	// Discard all if not match
	int    m_nRainbowChainLen;		// Discard all if not match
	list<ChainWalk> m_lChainWalk;
	bool   debug;
	string sPrecalcPathName;
	int    preCalcPart;
	vector<string> vPrecalcFiles;

private:
	void DiscardAll();
	bool FindInFile(uint64* pIndexE, unsigned char* pHash, int nHashLen);
	string CheckOrRotatePreCalcFile();
	void updateUsedPrecalcFiles();

public:
	uint64* RequestWalk(unsigned char* pHash, int nHashLen,
						string sHashRoutineName,
						string sPlainCharsetName, int nPlainLenMin, int nPlainLenMax, 
						int nRainbowTableIndex, 
						int nRainbowChainLen,
						bool& fNewlyGenerated,
						bool setDebug,
						string sPrecalc);
	void DiscardWalk(uint64* pIndexE);
	void StoreToFile(uint64* pIndexE, unsigned char* pHash, int nHashLen);
	void removePrecalcFiles();
};

#endif

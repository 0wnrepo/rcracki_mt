/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
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

#ifndef _CHAINWALKCONTEXT_H
#define _CHAINWALKCONTEXT_H

#include "HashRoutine.h"
#include "Public.h"

typedef struct 
{
	unsigned char m_PlainCharset[255];
	unsigned int m_nPlainCharsetLen;
	int m_nPlainLenMin;
	int m_nPlainLenMax;
	string m_sPlainCharsetName;
	string m_sPlainCharsetContent;
} stCharset;
class CChainWalkContext 
{
public:
	CChainWalkContext();
	virtual ~CChainWalkContext();

private:
	static string m_sHashRoutineName;	
	static HASHROUTINE m_pHashRoutine;							// Configuration
	static int m_nHashLen;										// Configuration
	static bool isOldRtFormat;
	static bool isRti2RtFormat;
	static vector<stCharset> m_vCharset;
	static int m_nPlainLenMinTotal, m_nPlainLenMaxTotal;
	static uint64 m_nPlainSpaceUpToX[MAX_PLAIN_LEN + 1];		// Performance consideration
	static uint64 m_nPlainSpaceTotal;							// Performance consideration
	static int m_nHybridCharset;
	static int m_nRainbowTableIndex;							// Configuration
	static uint64 m_nReduceOffset;								// Performance consideration

	// Context
	uint64 m_nIndex;
	unsigned char m_Plain[MAX_PLAIN_LEN];
	int m_nPlainLen;
	unsigned char m_Hash[MAX_HASH_LEN];
	static unsigned char m_Salt[MAX_SALT_LEN];
	static int m_nSaltLen;
private:
	static bool LoadCharset(string sCharset);

public:
	static bool SetHashRoutine(string sHashRoutineName);												// Configuration
	static bool SetPlainCharset(string sCharsetName, int nPlainLenMin, int nPlainLenMax);				// Configuration
	static bool SetRainbowTableIndex(int nRainbowTableIndex);	
	static bool SetSalt(unsigned char *Salt, int nSaltLength);// Configuration
	static bool SetupWithPathName(string sPathName, int& nRainbowChainLen, int& nRainbowChainCount);	// Wrapper
	static string GetHashRoutineName();
	static int GetHashLen();
	static string GetPlainCharsetName();
	static string GetPlainCharsetContent();
	static int GetPlainLenMin();
	static int GetPlainLenMax();
	static uint64 GetPlainSpaceTotal();
	static int GetRainbowTableIndex();
	static void Dump();
	static bool isOldFormat();
	static bool isRti2Format();


	void SetIndex(uint64 nIndex);
	void SetHash(unsigned char* pHash);		// The length should be m_nHashLen

	void IndexToPlain();
	void PlainToHash();
	void HashToIndex(int nPos);

	uint64 GetIndex();
	const uint64* GetIndexPtr();
	string GetPlain();
	string GetBinary();
	string GetHash();
	bool CheckHash(unsigned char* pHash);	// The length should be m_nHashLen
};

#endif

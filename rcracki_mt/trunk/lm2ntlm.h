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

#include <stdio.h>
#include <string>
#include <map>
#ifdef _WIN32
	#include <conio.h>
#endif
//#include "openssl/md4.h"
#include <time.h>
#include "signal.h"
#include "Public.h"
#include "md4.h"

using namespace std;

class LM2NTLMcorrector
{
public:
	LM2NTLMcorrector();

private:
	map<unsigned char, map<int, unsigned char> > m_mapChar;
	uint64 progressCurrentCombination;
	uint64 totalCurrentCombination;
	uint64 counterOverall;
	unsigned char NTLMHash[16];
	clock_t startClock;
	int countCombinations;
	int countTotalCombinations;
	int counter;
	clock_t previousClock;
	unsigned char currentCharmap[16][128];
	bool aborting;
	string sBinary;

private:
	bool checkNTLMPassword(unsigned char* pLMPassword, int nLMPasswordLen, string& sNTLMPassword);
	bool startCorrecting(string sLMPassword, string& sNTLMPassword, unsigned char* pLMPassword);
	void printString(unsigned char* muteThis, int length);
	void setupCombinationAtPositions(int length, unsigned char* pMuteMe, unsigned char* pTempMute, int* jAtPos, bool* fullAtPos, int* sizeAtPos);
	bool checkPermutations(int length, unsigned char* pTempMute, int* jAtPos, int* sizeAtPos, unsigned char* pLMPassword, string& sNTLMPassword);

	int calculateTotalCombinations(int length, int setSize);
	int factorial (int num);

	bool parseHexPassword(string hexPassword, string& sPlain);
	bool NormalizeHexString(string& sHash);
	void ParseHash(string sHash, unsigned char* pHash, int& nHashLen);
	string ByteToStr(const unsigned char* pData, int nLen);
	void addToMapW(unsigned char key, unsigned char value1, unsigned char value2);
	void fillMapW();
	void checkAbort();
	void writeEndStats();
public:
	bool LMPasswordCorrectUnicode(string hexPassword, unsigned char* NTLMHash, string& sNTLMPassword);
	string getBinary();
};


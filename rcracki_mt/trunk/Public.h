/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010 James Nobis <frt@quelrod.net>
 *
 * This file is part of racrcki_mt.
 *
 * rcracki_mt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * rcracki_mt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <list>

#include "global.h"

using namespace std;


struct RainbowChainO
{
	uint64 nIndexS;
	uint64 nIndexE;
};
//struct RainbowChain
//{
//	//uint64 nChain;
//	uint64 nIndexS;
//	int nIndexE;
//	int nCheckPoint;
//};
union RainbowChain
{
	//uint64 nChain;
	uint64 nIndexS;	
	struct
	{
		unsigned short foo[3];
		unsigned short nIndexE;
	};
	//int nCheckPoint;
};
//struct RainbowChain
//{
//	uint64 nIndexS : 48;
//	unsigned short nIndexE : 16;
//};
//struct IndexChain
//{
//	uint64 nPrefix;
//	int nFirstChain;
//	int nChainCount;
//	//unsigned short nChainCount; //(maybe union with nPrefix, 1 byte spoiled)
//};
#pragma pack(1)
union IndexChain
{
	uint64 nPrefix; //5
	struct
	{
		unsigned char foo[5];
		int nFirstChain; //4
		unsigned short nChainCount; //2
	};
	//unsigned short nChainCount; (maybe union with nPrefix, 1 byte spoiled, no pack(1) needed)
};
#pragma pack()
typedef struct
{
	string sName;
	int nPlainLenMin;
	int nPlainLenMax;
} tCharset;

#define MAX_PLAIN_LEN 256
#define MIN_HASH_LEN  8
#define MAX_HASH_LEN  256
#define MAX_SALT_LEN  256

// XXX nmap is GPL2, will check newer releases regarding license
// Code comes from nmap, used for the linux implementation of kbhit()
#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int tty_getchar();
void tty_done();
void tty_init();
void tty_flush(void);
// end nmap code

#include <sys/time.h>

#else
	int gettimeofday( struct timeval *tv, struct timezone *tz );
#endif

timeval sub_timeofday( timeval tv2, timeval tv );

unsigned int GetFileLen(FILE* file);
string TrimString(string s);
bool ReadLinesFromFile(string sPathName, vector<string>& vLine);
bool SeperateString(string s, string sSeperator, vector<string>& vPart);
string uint64tostr(uint64 n);
string uint64tohexstr(uint64 n);
string HexToStr(const unsigned char* pData, int nLen);
uint64 GetAvailPhysMemorySize();
string GetApplicationPath();
void ParseHash(string sHash, unsigned char* pHash, int& nHashLen);
bool GetHybridCharsets(string sCharset, vector<tCharset>& vCharset);
void Logo();
bool writeResultLineToFile(string sOutputFile, string sHash, string sPlain, string sBinary);

#endif

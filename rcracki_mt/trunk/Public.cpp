/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
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

#ifdef _WIN32
	#pragma warning(disable : 4786 4267 4018)
#endif

#include "Public.h"

#ifdef _WIN32
	#include <windows.h>
	#include <time.h>

	#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
	#else
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
	#endif
 
	struct timezone
	{
		int  tz_minuteswest; /* minutes W of Greenwich */
		int  tz_dsttime;     /* type of dst correction */
	};
 
	int gettimeofday(struct timeval *tv, struct timezone *tz)
	{
		// Define a structure to receive the current Windows filetime
		FILETIME ft;
 
		// Initialize the present time to 0 and the timezone to UTC
  		unsigned __int64 tmpres = 0;
		static int tzflag = 0;
 
		if (NULL != tv)
		{
			GetSystemTimeAsFileTime(&ft);
 
			// The GetSystemTimeAsFileTime returns the number of 100 nanosecond 
			// intervals since Jan 1, 1601 in a structure. Copy the high bits to 
			// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
			tmpres |= ft.dwHighDateTime;
			tmpres <<= 32;
			tmpres |= ft.dwLowDateTime;
 
			// Convert to microseconds by dividing by 10
			tmpres /= 10;
 
			// The Unix epoch starts on Jan 1 1970.  Need to subtract the difference 
			// in seconds from Jan 1 1601.
			tmpres -= DELTA_EPOCH_IN_MICROSECS;
	 
			// Finally change microseconds to seconds and place in the seconds value. 
			// The modulus picks up the microseconds.
			tv->tv_sec = (long)(tmpres / 1000000UL);
			tv->tv_usec = (long)(tmpres % 1000000UL);
		}
	 
		if (NULL != tz)
		{
			if (!tzflag)
			{
				_tzset();
				tzflag++;
			}
	  
			// Adjust for the timezone west of Greenwich
			tz->tz_minuteswest = _timezone / 60;
			tz->tz_dsttime = _daylight;
		}
	 
		return 0;
	}

#elif defined(__APPLE__) || \
	((defined(__unix__) || defined(unix)) && !defined(USG))

	#include <sys/param.h>

	#if defined(BSD)
		#include <sys/sysctl.h>
	#elif defined(__linux__)
		#include <sys/sysinfo.h>
	#else
		#error Unsupported Operating System
	#endif
#endif

//////////////////////////////////////////////////////////////////////

timeval sub_timeofday( timeval tv2, timeval tv )
{
	timeval final;

	final.tv_usec = tv2.tv_usec - tv.tv_usec;
	final.tv_sec = tv2.tv_sec - tv.tv_sec;

	if ( final.tv_usec < 0 )
	{
		final.tv_usec += 1000000;
		--final.tv_sec;
	}

	return final;
}

unsigned int GetFileLen(FILE* file)
{
	long int pos = ftell(file);
	fseek(file, 0, SEEK_END);
	long int len = ftell(file);
	fseek(file, pos, SEEK_SET);

	return len;
}

string TrimString(string s)
{
	while (s.size() > 0)
	{
		if (s[0] == ' ' || s[0] == '\t')
			s = s.substr(1);
		else
			break;
	}

	while (s.size() > 0)
	{
		if (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')
			s = s.substr(0, s.size() - 1);
		else
			break;
	}

	return s;
}
bool GetHybridCharsets(string sCharset, vector<tCharset>& vCharset)
{
	// Example: hybrid(mixalpha-numeric-all-space#1-6,numeric#1-4)
	if(sCharset.substr(0, 6) != "hybrid") // Not hybrid charset
		return false;

	UINT4 nEnd = (int) sCharset.rfind(')');
	UINT4 nStart = (int) sCharset.rfind('(');
	string sChar = sCharset.substr(nStart + 1, nEnd - nStart - 1);
	vector<string> vParts;
	SeperateString(sChar, ",", vParts);
	for(UINT4 i = 0; i < vParts.size(); i++)
	{
		tCharset stCharset;
		vector<string> vParts2;
		SeperateString(vParts[i], "#", vParts2);
		stCharset.sName = vParts2[0];
		vector<string> vParts3;
		SeperateString(vParts2[1], "-", vParts3);
		stCharset.nPlainLenMin = atoi(vParts3[0].c_str());
		stCharset.nPlainLenMax = atoi(vParts3[1].c_str());
		vCharset.push_back(stCharset);
	}
	return true;
}
bool ReadLinesFromFile(string sPathName, vector<string>& vLine)
{
	vLine.clear();

	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file != NULL)
	{
		unsigned int len = GetFileLen(file);
		char* data = new char[len + 1];
		fread(data, 1, len, file);
		data[len] = '\0';
		string content = data;
		content += "\n";
		delete [] data;

		unsigned int i;
		for (i = 0; i < content.size(); i++)
		{
			if (content[i] == '\r')
				content[i] = '\n';
		}

		int n;
		while ((n = content.find("\n", 0)) != -1)
		{
			string line = content.substr(0, n);
			line = TrimString(line);
			if (line != "")
				vLine.push_back(line);
			content = content.substr(n + 1);
		}

		fclose(file);
	}
	else
		return false;

	return true;
}

bool writeResultLineToFile(string sOutputFile, string sHash, string sPlain, string sBinary)
{
	FILE* file = fopen(sOutputFile.c_str(), "a");
	if (file!=NULL)
	{
		string buffer = sHash + ":" + sPlain + ":" + sBinary + "\n";
		fputs (buffer.c_str(), file);
		fclose (file);
		return true;
	}
	else
		return false;
}

bool SeperateString(string s, string sSeperator, vector<string>& vPart)
{
	vPart.clear();

	unsigned int i;
	for (i = 0; i < sSeperator.size(); i++)
	{
		int n = s.find(sSeperator[i]);
		if (n != -1)
		{
			vPart.push_back(s.substr(0, n));
			s = s.substr(n + 1);
		}
		else
		{
			printf("not found: %c\n", sSeperator[i]);
			printf("s: %s\n", s.c_str());
			return false;
		}
	}
	vPart.push_back(s);

	return true;
}

string uint64tostr(uint64 n)
{
	char str[32];

#ifdef _WIN32
	sprintf(str, "%I64u", n);
#else
	sprintf(str, "%llu", n);
#endif

	return str;
}

string uint64tohexstr(uint64 n)
{
	char str[32];

#ifdef _WIN32
	sprintf(str, "%016I64x", n);
#else
	sprintf(str, "%016llx", n);
#endif

	return str;
}

string HexToStr(const unsigned char* pData, int nLen)
{
	string sRet;
	int i;
	for (i = 0; i < nLen; i++)
	{
		char szByte[3];
		sprintf(szByte, "%02x", pData[i]);
		sRet += szByte;
	}

	return sRet;
}

uint64 GetAvailPhysMemorySize()
{
#ifdef _WIN32
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	return ms.dwAvailPhys;
#elif defined(BSD)
	int mib[2] = { CTL_HW, HW_PHYSMEM };
	uint64 physMem;
	//XXX warning size_t isn't portable
	size_t len;
	len = sizeof(physMem);
	sysctl(mib, 2, &physMem, &len, NULL, 0);
	return physMem;
#elif defined(__linux__)
	struct sysinfo info;
	sysinfo(&info);
	return ( info.freeram + info.bufferram ) * (unsigned long) info.mem_unit;
#else
	return 0;
	#error Unsupported Operating System
#endif
}

string GetApplicationPath()
{
	char fullPath[FILENAME_MAX];

#ifdef _WIN32
	GetModuleFileName(NULL, fullPath, FILENAME_MAX);
#else
	char szTmp[32];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = readlink(szTmp, fullPath, FILENAME_MAX);
	if(bytes >= 0)
		fullPath[bytes] = '\0';
#endif

	string sApplicationPath = fullPath;
#ifdef _WIN32
	int nIndex = sApplicationPath.find_last_of('\\');
#else
	int nIndex = sApplicationPath.find_last_of('/');
#endif

	if (nIndex != -1)
		sApplicationPath = sApplicationPath.substr(0, nIndex+1);

	//printf ("\n\nDebug: The application directory is %s\n", sApplicationPath.c_str());
	return sApplicationPath;
}

void ParseHash(string sHash, unsigned char* pHash, int& nHashLen)
{
	UINT4 i;
	for (i = 0; i < sHash.size() / 2; i++)
	{
		string sSub = sHash.substr(i * 2, 2);
		int nValue;
		sscanf(sSub.c_str(), "%02x", &nValue);
		pHash[i] = (unsigned char)nValue;
	}

	nHashLen = (int) sHash.size() / 2;
}

void Logo()
{
	printf("RainbowCrack (improved, multi-threaded) - Making a Faster Cryptanalytic Time-Memory Trade-Off\n");
	printf("by Martin Westergaard <martinwj2005@gmail.com>\n");
	printf("multi-threaded and enhanced by neinbrucke (version 0.6.3)\n");
	printf("http://www.freerainbowtables.com/\n");
	printf("original code by Zhu Shuanglei <shuanglei@hotmail.com>\n");
	printf("http://www.antsight.com/zsl/rainbowcrack/\n\n");
}

// XXX nmap is GPL2, will check newer releases regarding license
// Code comes from nmap, used for the linux implementation of kbhit()
#ifndef _WIN32

static int tty_fd = 0;
struct termios saved_ti;

int tty_getchar()
{
	int c, numChars;

	if (tty_fd && tcgetpgrp(tty_fd) == getpid()) {
		c = 0;
		numChars = read(tty_fd, &c, 1);
		if (numChars > 0) return c;
	}

	return -1;
}

void tty_done()
{
	if (!tty_fd) return;

	tcsetattr(tty_fd, TCSANOW, &saved_ti);

	close(tty_fd);
	tty_fd = 0;
}

void tty_init()
{
	struct termios ti;

	if (tty_fd)
		return;

	if ((tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK)) < 0) return;

	tcgetattr(tty_fd, &ti);
	saved_ti = ti;
	ti.c_lflag &= ~(ICANON | ECHO);
	ti.c_cc[VMIN] = 1;
	ti.c_cc[VTIME] = 0;
	tcsetattr(tty_fd, TCSANOW, &ti);

	atexit(tty_done);
}

void tty_flush(void)
{
	tcflush(tty_fd, TCIFLUSH);
}
// end nmap code
#endif

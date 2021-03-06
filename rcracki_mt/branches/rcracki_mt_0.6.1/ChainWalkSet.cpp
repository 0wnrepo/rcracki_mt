/*
   RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

   Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif

#include "ChainWalkSet.h"

CChainWalkSet::CChainWalkSet()
{
	m_sHashRoutineName   = "";
	m_sPlainCharsetName  = "";
	m_nPlainLenMin       = 0;
	m_nPlainLenMax       = 0;
	m_nRainbowTableIndex = 0;
	m_nRainbowChainLen   = 0;
	debug = false;
	sPrecalcPathName     = "";
	preCalcPart          = 0;
}

CChainWalkSet::~CChainWalkSet()
{
	DiscardAll();
}

void CChainWalkSet::DiscardAll()
{
	//printf("debug: discarding all walk...\n");

	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
		delete it->pIndexE;
	m_lChainWalk.clear();
}

string CChainWalkSet::CheckOrRotatePreCalcFile()
{
	char sPreCalcFileName[255];

	// 255 files limit to be sure
	for (; preCalcPart < 255; preCalcPart++)
	{
		sprintf(sPreCalcFileName, "%s.%d", sPrecalcPathName.c_str(), preCalcPart);
		string sReturnPreCalcPath(sPreCalcFileName);

		unsigned int fileLen = 0;

		FILE* file = fopen(sReturnPreCalcPath.c_str(), "ab");
		if(file!=NULL)
		{
			fileLen = GetFileLen(file);
			unsigned int nextFileLen = fileLen + (sizeof(uint64) * (m_nRainbowChainLen-1));
			// Rotate to next file if we are going to pass 2GB filesize
			if (nextFileLen < ((unsigned)2 * 1024 * 1024 * 1024))
			{
				// We might want to vPrecalcFiles.push_back(sReturnPreCalcPath) if we just created this file
				// We don't as only newly generated chainwalksets will be stored to this new file, so we don't have to look there
				if (debug) printf("Debug: Using for precalc: %s\n", sReturnPreCalcPath.c_str());
				fclose(file);
				return sReturnPreCalcPath;
			}
			fclose(file);
		}
	}
}

void CChainWalkSet::updateUsedPrecalcFiles()
{
	// we might also use this function to search a wildcard path of precalc files
	vPrecalcFiles.clear();
	char sPreCalcFileName[255];

	int i;
	// 255 files max
	for (i = 0; i < 255; i++)
	{
		sprintf(sPreCalcFileName, "%s.%d", sPrecalcPathName.c_str(), i);
		string sTryPreCalcPath(sPreCalcFileName);
		FILE* file = fopen(sTryPreCalcPath.c_str(), "rb");
		if(file!=NULL) {
			vPrecalcFiles.push_back(sTryPreCalcPath);
			fclose(file);
		}
		else {
			break;
		}
	}
}

void CChainWalkSet::removePrecalcFiles()
{
	if (debug) printf("Debug: Removing precalc files.\n");
	updateUsedPrecalcFiles();
	string sCurrentPrecalcPathName = "";
	string sCurrentPrecalcIndexPathName = "";
	
	int i;
	for (i = 0; i < (int)vPrecalcFiles.size(); i++)
	{
		sCurrentPrecalcPathName = vPrecalcFiles[i];
		sCurrentPrecalcIndexPathName = sCurrentPrecalcPathName + ".index";

		if (debug) printf("Debug: Removing precalc file: %s\n", sCurrentPrecalcPathName.c_str());

		if (remove(sCurrentPrecalcPathName.c_str()) != 0)
			if (debug) printf("Debug: Failed removing precalc file: %s\n", sCurrentPrecalcPathName.c_str());

		if (debug) printf("Debug: Removing precalc index file: %s\n", sCurrentPrecalcIndexPathName.c_str());

		if (remove(sCurrentPrecalcIndexPathName.c_str()) != 0)
			if (debug) printf("Debug: Failed removing precalc index file: %s\n", sCurrentPrecalcIndexPathName.c_str());

	}
}

bool CChainWalkSet::FindInFile(uint64* pIndexE, unsigned char* pHash, int nHashLen)
{
	int gotPrecalcOnLine = -1;
	char precalculationLine[255];
	sprintf(precalculationLine, "%s_%s#%d-%d_%d_%d:%s\n", m_sHashRoutineName.c_str(), m_sPlainCharsetName.c_str(), m_nPlainLenMin, m_nPlainLenMax, m_nRainbowTableIndex, m_nRainbowChainLen, HexToStr(pHash, nHashLen).c_str() );
	string precalcString(precalculationLine);

	string sCurrentPrecalcPathName = "";
	string sCurrentPrecalcIndexPathName = "";
	int offset = 0;

	int i;
	for (i = 0; i < (int)vPrecalcFiles.size() && gotPrecalcOnLine == -1; i++)
	{
		sCurrentPrecalcPathName = vPrecalcFiles[i];
		sCurrentPrecalcIndexPathName = sCurrentPrecalcPathName + ".index";

		int offset = 0;

		vector<string> precalcLines;
		if (ReadLinesFromFile(sCurrentPrecalcIndexPathName.c_str(), precalcLines))
		{
			int i;
			for (i = 0; i < (int)precalcLines.size(); i++)
			{
				if (precalcString.compare(0, precalcString.size()-1, precalcLines[i]) == 0)
				{
					gotPrecalcOnLine = i;
					break;
				}

				// Parse
				vector<string> vPart;
				if (SeperateString(precalcLines[i], "___:", vPart))
				{
					// add to offset
					offset += ((atoi(vPart[3].c_str())-1) * sizeof(uint64));
				}
				else {
					// corrupt file
					printf("Corrupted precalculation file!\n");
					gotPrecalcOnLine = -1;
					break;
				}
			}
		}
	}

	if (gotPrecalcOnLine > -1)
	{
		if (debug) printf("Debug: Reading pre calculations from file, line %d offset %d\n", gotPrecalcOnLine, offset);
		
		FILE* fp = fopen(sCurrentPrecalcPathName.c_str(), "rb");

		if (fp!=NULL) {
			fseek(fp, offset, SEEK_SET);

			// We should do some verification here, for example by recalculating the middle chain, to catch corrupted files
			if(fread(pIndexE, sizeof(uint64), m_nRainbowChainLen-1, fp) != m_nRainbowChainLen-1)
				printf("File read error.");
			fclose(fp);
		}
		else
			printf("Cannot open precalculation file %s.\n", sCurrentPrecalcPathName.c_str());

		//printf("\npIndexE[0]: %s\n", uint64tostr(pIndexE[0]).c_str());
		//printf("\npIndexE[nRainbowChainLen-2]: %s\n", uint64tostr(pIndexE[m_nRainbowChainLen-2]).c_str());

		return true;
	}

	return false;

}

void CChainWalkSet::StoreToFile(uint64* pIndexE, unsigned char* pHash, int nHashLen)
{
	if (debug) printf("\nDebug: Storing precalc\n");
	
	string sCurrentPrecalcPathName = CheckOrRotatePreCalcFile();
	string sCurrentPrecalcIndexPathName = sCurrentPrecalcPathName + ".index";

	FILE* fp = fopen(sCurrentPrecalcPathName.c_str(), "ab");
	if(fp!=NULL)
	{
		if(fwrite(pIndexE, sizeof(uint64), m_nRainbowChainLen-1, fp) != m_nRainbowChainLen-1)
			printf("File write error.");
		else
		{
			FILE* file = fopen(sCurrentPrecalcIndexPathName.c_str(), "a");
			if (file!=NULL)
			{
				char precalculationLine[255];
				sprintf(precalculationLine, "%s_%s#%d-%d_%d_%d:%s\n", m_sHashRoutineName.c_str(), m_sPlainCharsetName.c_str(), m_nPlainLenMin, m_nPlainLenMax, m_nRainbowTableIndex, m_nRainbowChainLen, HexToStr(pHash, nHashLen).c_str() );
				fputs (precalculationLine, file);
				fclose (file);
			}
		}
		fclose(fp);
		}
	else
		printf("Cannot open precalculation file %s\n", sCurrentPrecalcPathName.c_str());

}

uint64* CChainWalkSet::RequestWalk(unsigned char* pHash, int nHashLen,
								   string sHashRoutineName,
								   string sPlainCharsetName, int nPlainLenMin, int nPlainLenMax, 
								   int nRainbowTableIndex, 
								   int nRainbowChainLen,
								   bool& fNewlyGenerated,
								   bool setDebug,
								   string sPrecalc)
{
	debug = setDebug;
	sPrecalcPathName = sPrecalc;

	if (   m_sHashRoutineName   != sHashRoutineName
		|| m_sPlainCharsetName  != sPlainCharsetName
		|| m_nPlainLenMin       != nPlainLenMin
		|| m_nPlainLenMax       != nPlainLenMax
		|| m_nRainbowTableIndex != nRainbowTableIndex
		|| m_nRainbowChainLen   != nRainbowChainLen)
	{
		DiscardAll();

		m_sHashRoutineName   = sHashRoutineName;
		m_sPlainCharsetName  = sPlainCharsetName;
		m_nPlainLenMin       = nPlainLenMin;
		m_nPlainLenMax       = nPlainLenMax;
		m_nRainbowTableIndex = nRainbowTableIndex;
		m_nRainbowChainLen   = nRainbowChainLen;

		ChainWalk cw;
		memcpy(cw.Hash, pHash, nHashLen);
		cw.pIndexE = new uint64[nRainbowChainLen - 1];
		m_lChainWalk.push_back(cw);

		// Only update this list when we search through another rainbow table
		updateUsedPrecalcFiles();

		if (!FindInFile(cw.pIndexE, pHash, nHashLen))
			fNewlyGenerated = true;
		else
			fNewlyGenerated = false;
		return cw.pIndexE;
	}

	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
	{
		if (memcmp(it->Hash, pHash, nHashLen) == 0)
		{
			fNewlyGenerated = false;
			return it->pIndexE;
		}
	}

	ChainWalk cw;
	memcpy(cw.Hash, pHash, nHashLen);
	cw.pIndexE = new uint64[nRainbowChainLen - 1];
	m_lChainWalk.push_back(cw);

	if (!FindInFile(cw.pIndexE, pHash, nHashLen))
			fNewlyGenerated = true;
		else
			fNewlyGenerated = false;
	return cw.pIndexE;
}

void CChainWalkSet::DiscardWalk(uint64* pIndexE)
{
	list<ChainWalk>::iterator it;
	for (it = m_lChainWalk.begin(); it != m_lChainWalk.end(); it++)
	{
		if (it->pIndexE == pIndexE)
		{
			delete it->pIndexE;
			m_lChainWalk.erase(it);
			return;
		}
	}

	printf("debug: pIndexE not found\n");
}

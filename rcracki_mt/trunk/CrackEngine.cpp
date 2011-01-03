/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2009, 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010 James Nobis <frt@quelrod.net>
 * Copyright 2010 uroskn
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

#if defined(_WIN32) && !defined(__GNUC__)
	#pragma warning(disable : 4786 4267 4018)
#endif

#include "CrackEngine.h"
#include "RTI2Reader.h"

#ifndef _WIN32
	#include <sys/resource.h>
#endif

CCrackEngine::CCrackEngine()
{
	ResetStatistics();
	writeOutput = false;
	resumeSession = false;
	debug = false;
	keepPrecalcFiles = false;

	sSessionPathName = "";
	sProgressPathName = "";
}

CCrackEngine::~CCrackEngine()
{
}

//////////////////////////////////////////////////////////////////////

void CCrackEngine::ResetStatistics()
{
	m_fTotalDiskAccessTime               = 0.0f;
	m_fTotalCryptanalysisTime            = 0.0f;
	m_fTotalPrecalculationTime           = 0.0f;
	m_nTotalChainWalkStep                = 0;
	m_nTotalFalseAlarm                   = 0;
	m_nTotalChainWalkStepDueToFalseAlarm = 0;
//	m_nTotalFalseAlarmSkipped			 = 0;
}

int CCrackEngine::BinarySearchOld(RainbowChainO* pChain, int nRainbowChainCount, uint64 nIndex)
{
	int nLow = 0;
	int nHigh = nRainbowChainCount - 1;
	while (nLow <= nHigh)
	{
		int nMid = (nLow + nHigh) / 2;
		if (nIndex == pChain[nMid].nIndexE)
			return nMid;
		else if (nIndex < pChain[nMid].nIndexE)
			nHigh = nMid - 1;
		else
			nLow = nMid + 1;
	}

	return -1;
}

RainbowChain *CCrackEngine::BinarySearch(RainbowChain *pChain, int nChainCountRead, uint64 nIndex, IndexChain *pIndex, int nIndexSize, int nIndexStart)
{
	uint64 nPrefix = nIndex >> 16;
	int nLow, nHigh;	
	bool found = false;
	//int nChains = 0;
	
	if(nPrefix > (pIndex[nIndexSize-1].nPrefix & 0x000000FFFFFFFFFFULL)) // check if its in the index file
	{
		return NULL;
	}

	int nBLow = 0;
	int nBHigh = nIndexSize - 1;
	while (nBLow <= nBHigh)
	{
		int nBMid = (nBLow + nBHigh) / 2;
		if (nPrefix == (pIndex[nBMid].nPrefix & 0x000000FFFFFFFFFFULL))
		{
			//nLow = nChains;
			//int nChains = 0;

			nLow = pIndex[nBMid].nFirstChain;
			nHigh = nLow + pIndex[nBMid].nChainCount;
			if(nLow >= nIndexStart && nLow <= nIndexStart + nChainCountRead) 
			{					
				if(nHigh > nIndexStart + nChainCountRead)
					nHigh = nIndexStart + nChainCountRead;
			}
			else if(nLow < nIndexStart && nHigh >= nIndexStart)
			{
				nLow = nIndexStart;
			}
			else break;					
			found = true;
			break;
		}
		else if (nPrefix < (pIndex[nBMid].nPrefix & 0x000000FFFFFFFFFFULL))
			nBHigh = nBMid - 1;
		else
			nBLow = nBMid + 1;
	}
	if(found == true)
	{
		for(int i = nLow - nIndexStart; i < nHigh - nIndexStart; i++)
		{
			int nSIndex = ((int)nIndex) & 0x0000FFFF;

			if (nSIndex == pChain[i].nIndexE)
			{
				return &pChain[i];
			}				
			else if(pChain[i].nIndexE > nSIndex)
				break;
		}
	}	
	return NULL;
}

// not used currently, leaving code for future checkpoints
//bool CCrackEngine::CheckAlarm(RainbowChain* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs)
//{
//	CChainWalkContext cwc;
//	//uint64 nIndexS = pChain->nIndexS >> 16;
//	uint64 nIndexS = pChain->nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes
//	cwc.SetIndex(nIndexS);
//	int nPos;
//	for (nPos = 0; nPos < nGuessedPos; nPos++)
//	{
//		cwc.IndexToPlain();
//		cwc.PlainToHash();
//		cwc.HashToIndex(nPos);
//		// Not using checkpoints atm
//		/*
//		switch(nPos)
//		{
//		case 5000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000080) >> 7)
//				{
//					m_nTotalFalseAlarmSkipped += 10000 - 5000;
////					printf("CheckPoint caught false alarm at position 7600\n");
//					return false;
//				}
//				break;
//		case 6000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000040) >> 6)
//				{
////					printf("CheckPoint caught false alarm at position 8200\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 6000;
//					return false;
//				}
//				break;
//
//		case 7600:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000020) >> 5)
//				{
////					printf("CheckPoint caught false alarm at position 8700\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 7600;
//					return false;
//				}
//				break;
//
//		case 8200:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000010) >> 4)
//				{
////					printf("CheckPoint caught false alarm at position 9000\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 8200;
//					return false;
//				}
//				break;
//
//			case 8700:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000008) >> 3)
//				{
////					printf("CheckPoint caught false alarm at position 9300\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 8700;
//					return false;
//				}
//
//				break;
//			case 9000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000004) >> 2)
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9000;
//					return false;
//				}
//
//				break;
//			case 9300:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000002) >> 1)
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9300;
//					return false;
//				}
//				break;
//			case 9600:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000001))
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9600;
//					return false;
//				}
//				break;
//
//		}*/
//	}
//	cwc.IndexToPlain();
//	cwc.PlainToHash();
//	if (cwc.CheckHash(pHash))
//	{
//		printf("plaintext of %s is %s\n", cwc.GetHash().c_str(), cwc.GetPlain().c_str());
//		hs.SetPlain(cwc.GetHash(), cwc.GetPlain(), cwc.GetBinary());
//		return true;
//	}
//
//	return false;
//}

//bool CCrackEngine::CheckAlarmOld(RainbowChainO* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs)
//{
//	CChainWalkContext cwc;
//	cwc.SetIndex(pChain->nIndexS);
//	int nPos;
//	for (nPos = 0; nPos < nGuessedPos; nPos++)
//	{
//		cwc.IndexToPlain();
//		cwc.PlainToHash();
//		cwc.HashToIndex(nPos);
//	}
//	cwc.IndexToPlain();
//	cwc.PlainToHash();
//	if (cwc.CheckHash(pHash))
//	{
//		printf("plaintext of %s is %s\n", cwc.GetHash().c_str(), cwc.GetPlain().c_str());
//		hs.SetPlain(cwc.GetHash(), cwc.GetPlain(), cwc.GetBinary());
//		return true;
//	}
//
//	return false;
//}

void CCrackEngine::GetChainIndexRangeWithSameEndpoint(RainbowChainO* pChain,
													  int nRainbowChainCount,
													  int nMatchingIndexE,
													  int& nMatchingIndexEFrom,
													  int& nMatchingIndexETo)
{
	nMatchingIndexEFrom = nMatchingIndexE;
	nMatchingIndexETo   = nMatchingIndexE;
	while (nMatchingIndexEFrom > 0)
	{
		if (pChain[nMatchingIndexEFrom - 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexEFrom--;
		else
			break;
	}
	while (nMatchingIndexETo < nRainbowChainCount - 1)
	{
		if (pChain[nMatchingIndexETo + 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexETo++;
		else
			break;
	}
}

void CCrackEngine::SearchTableChunkOld(RainbowChainO* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs)
{
	vector<string> vHash;
	hs.GetLeftHashWithLen(vHash, CChainWalkContext::GetHashLen());
	printf("searching for %lu hash%s...\n", (unsigned long)vHash.size(),
										   vHash.size() > 1 ? "es" : "");

	int nChainWalkStep = 0;
	int nFalseAlarm = 0;
	int nChainWalkStepDueToFalseAlarm = 0;

	vector<rcrackiThread*> threadPool;
	vector<pthread_t> pThreads;

	#ifndef _WIN32
		/*
		 * On linux you cannot set the priority of a thread in the non real time
		 * scheduling groups.  You can set the priority of the process.  In
		 * windows BELOW_NORMAL represents a 1/8th drop in priority and this would
		 * be 20 * 1/8 on linux or about 2.5
		 */
		setpriority( PRIO_PROCESS, 0, 2 );
	#endif

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	#ifdef _WIN32
	sched_param param;
	/*
	 * windows scheduling is 0 to 32 (low to high) with 8 as normal and 7 as
	 * BELOW_NORMAL
	 */
	param.sched_priority = THREAD_PRIORITY_BELOW_NORMAL;
	pthread_attr_setschedparam (&attr, &param);
	#endif

	bool pausing = false;

	uint32 nHashIndex;
	for (nHashIndex = 0; nHashIndex < vHash.size(); nHashIndex++)
	{
		#ifdef _WIN32
		if (_kbhit())
		{
			int ch = _getch();
			ch = toupper(ch);
			if (ch == 'P')
			{
				pausing = true;
				printf( "\nPausing, press P again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if (_kbhit())
					{
						ch = _getch();
						ch = toupper(ch);
						if (ch == 'P')
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					Sleep(500);
				}
			}
			else
			{
				printf( "\nPress 'P' to pause...\n");
			}
		}
		#else
		int c = tty_getchar();
		if (c >= 0) {
			tty_flush();
			if (c==112) { // = p
				pausing = true;
				printf( "\nPausing, press 'p' again to continue... ");
				
				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );
				
				while (pausing)
				{
					if ((c = tty_getchar()) >= 0)
					{
						tty_flush();
						if (c == 112)
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					usleep(500*1000);
				}
			}
			else {
				printf( "\nPress 'p' to pause...\n");
			}
		}
		#endif
		unsigned char TargetHash[MAX_HASH_LEN];
		int nHashLen;
		ParseHash(vHash[nHashIndex], TargetHash, nHashLen);
		if (nHashLen != CChainWalkContext::GetHashLen())
			printf("debug: nHashLen mismatch\n");

		// Rqeuest ChainWalk
		bool fNewlyGenerated;
		uint64* pStartPosIndexE = m_cws.RequestWalk(TargetHash,
													nHashLen,
													CChainWalkContext::GetHashRoutineName(),
													CChainWalkContext::GetPlainCharsetName(),
													CChainWalkContext::GetPlainLenMin(),
													CChainWalkContext::GetPlainLenMax(),
													CChainWalkContext::GetRainbowTableIndex(),
													nRainbowChainLen,
													fNewlyGenerated,
													debug,
													sPrecalcPathName);
		//printf("debug: using %s walk for %s\n", fNewlyGenerated ? "newly generated" : "existing",
		//										vHash[nHashIndex].c_str());

		// Walk
		if (fNewlyGenerated)
		{
			timeval tv;
			timeval tv2;
			timeval final;

			gettimeofday( &tv, NULL );

			printf("Pre-calculating hash %lu of %lu.%-20s\r",
				(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");
			threadPool.clear();
			pThreads.clear();
			
			uint32 thread_ID;
			for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
			{
				rcrackiThread* r_Thread = new rcrackiThread(TargetHash, thread_ID, nRainbowChainLen, maxThreads, pStartPosIndexE);
				if (r_Thread)
				{
					pthread_t pThread;
					int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

					if( returnValue != 0 )
					{
						printf("pThread creation failed, returnValue: %d\n", returnValue);
					}
					else
					{
						pThreads.push_back(pThread);
					}

					threadPool.push_back(r_Thread);
				}
				else 
				{
					printf("r_Thread creation failed!\n");
				}
			}
			
			//printf("%d r_Threads created\t\t\n", threadPool.size());
			
			for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
			{
				pthread_t pThread = pThreads[thread_ID];
				int returnValue = pthread_join(pThread, NULL);
				if( returnValue != 0 )
				{
					printf("pThread join failed, returnValue: %d\n", returnValue);
				}
					
				rcrackiThread* rThread = threadPool[thread_ID];
				nChainWalkStep += rThread->GetChainWalkStep();
			}

			gettimeofday( &tv2, NULL );
			final = sub_timeofday( tv2, tv );
			
			float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;

			m_fTotalPrecalculationTime += fTime;
			m_fTotalCryptanalysisTime -= fTime;

			printf("%-50s\r", "");

			if ( debug )
				printf("pre-calculation time: %.2f s\n", fTime);
		}

		//printf("Checking false alarms for hash %d of %d.\t\t\r", nHashIndex+1, vHash.size());
		printf("Checking false alarms for hash %lu of %lu.%-20s\r", 
			(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");

		threadPool.clear();
		pThreads.clear();

		int i;
		for (i = 0; i < maxThreads; i++)
		{
			rcrackiThread* r_Thread = new rcrackiThread(TargetHash, true);
			threadPool.push_back(r_Thread);
		}

		uint32 thread_ID = 0;
		int nPos;
		for (nPos = nRainbowChainLen - 2; nPos >= 0; nPos--)
		{
			uint64 nIndexEOfCurPos = pStartPosIndexE[nPos];
		
			// Search matching nIndexE
			int nMatchingIndexE = BinarySearchOld(pChain, nRainbowChainCount, nIndexEOfCurPos);
			if (nMatchingIndexE != -1)
			{
				int nMatchingIndexEFrom, nMatchingIndexETo;
				GetChainIndexRangeWithSameEndpoint(pChain, nRainbowChainCount,
												   nMatchingIndexE,
												   nMatchingIndexEFrom, nMatchingIndexETo);
				int i;
				for (i = nMatchingIndexEFrom; i <= nMatchingIndexETo; i++)
				{
					rcrackiThread* rThread = threadPool[thread_ID];
					rThread->AddAlarmCheckO(pChain + i, nPos);
					if (thread_ID < (unsigned long)maxThreads - 1 ) {
						thread_ID++;
					} else {
						thread_ID = 0;
					}
				}
			}
		}

		for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
		{
			rcrackiThread* r_Thread = threadPool[thread_ID];
			pthread_t pThread;

			int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

			if( returnValue != 0 )
			{
				printf("pThread creation failed, returnValue: %d\n", returnValue);
			}
			else
			{
				pThreads.push_back(pThread);
			}
		}
		
		//printf("%d r_Threads created\t\t\n", threadPool.size());

		bool foundHashInThread = false;
		for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
		{
			rcrackiThread* rThread = threadPool[thread_ID];
			pthread_t pThread = pThreads[thread_ID];

			int returnValue = pthread_join(pThread, NULL);
			if( returnValue != 0 )
			{
				printf("pThread join failed, returnValue: %d\n", returnValue);
			}

			nChainWalkStepDueToFalseAlarm += rThread->GetChainWalkStepDueToFalseAlarm();
			nFalseAlarm += rThread->GetnFalseAlarm();

			if (rThread->FoundHash() && !foundHashInThread) {
				//printf("\t\t\t\t\t\t\r");
				printf("%-50s\r", "");

				printf("plaintext of %s is %s\n", rThread->GetHash().c_str(), rThread->GetPlain().c_str());
				if (writeOutput)
				{
					if (!writeResultLineToFile(outputFile, rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary()))
						printf("Couldn't write this result to file!\n");
				}
				hs.SetPlain(rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary());

				FILE* file = fopen(sSessionPathName.c_str(), "a");
				if (file!=NULL)
				{
					string buffer = "sHash=" + rThread->GetHash() + ":" + rThread->GetBinary() + ":" + rThread->GetPlain() + "\n";
					fputs (buffer.c_str(), file);
					fclose (file);
				}

				m_cws.DiscardWalk(pStartPosIndexE);
				foundHashInThread = true;
			}
		}

		pThreads.clear();
		threadPool.clear();
	}

	//printf("\t\t\t\t\t\t\t\r");
	printf("%-50s\r", "");
	pThreads.clear();
	threadPool.clear();
	pthread_attr_destroy(&attr);

	//printf("debug: chain walk step: %d\n", nChainWalkStep);
	//printf("debug: false alarm: %d\n", nFalseAlarm);
	//printf("debug: chain walk step due to false alarm: %d\n", nChainWalkStepDueToFalseAlarm);

	m_nTotalChainWalkStep += nChainWalkStep;
	m_nTotalFalseAlarm += nFalseAlarm;
	m_nTotalChainWalkStepDueToFalseAlarm += nChainWalkStepDueToFalseAlarm;
}

void CCrackEngine::SearchTableChunk(RainbowChain* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs, IndexChain *pIndex, int nIndexSize, int nChainStart)
{
	vector<string> vHash;
	//vector<uint64 *> vIndices;
	//vector<RainbowChain *> vChains;
	hs.GetLeftHashWithLen(vHash, CChainWalkContext::GetHashLen());
	printf("searching for %lu hash%s...\n", (unsigned long)vHash.size(),
										   vHash.size() > 1 ? "es" : "");

	int nChainWalkStep = 0;
	int nFalseAlarm = 0;
	int nChainWalkStepDueToFalseAlarm = 0;

	vector<rcrackiThread*> threadPool;
	vector<pthread_t> pThreads;
	
	#ifndef _WIN32
		/*
		 * On linux you cannot set the priority of a thread in the non real time
		 * scheduling groups.  You can set the priority of the process.  In
		 * windows BELOW_NORMAL represents a 1/8th drop in priority and this would
		 * be 20 * 1/8 on linux or about 2.5
		 */
		setpriority( PRIO_PROCESS, 0, 2 );
	#endif

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	#ifdef _WIN32
	sched_param param;
	param.sched_priority = THREAD_PRIORITY_BELOW_NORMAL;
	pthread_attr_setschedparam (&attr, &param);
	#endif
	// else set it to 5 or something (for linux)?

	bool pausing = false;

	uint32 nHashIndex;
	for (nHashIndex = 0; nHashIndex < vHash.size(); nHashIndex++)
	{
		#ifdef _WIN32
		if (_kbhit())
		{
			int ch = _getch();
			ch = toupper(ch);
			if (ch == 'P')
			{
				pausing = true;
				printf( "\nPausing, press P again to continue... ");
				
				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if (_kbhit())
					{
						ch = _getch();
						ch = toupper(ch);
						if (ch == 'P')
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					Sleep(500);
				}
			}
			else
			{
				printf( "\nPress 'P' to pause...\n");
			}
		}
		#else
		int c = tty_getchar();
		if (c >= 0) {
			tty_flush();
			if (c==112) { // = p
				pausing = true;
				printf( "\nPausing, press 'p' again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if ((c = tty_getchar()) >= 0)
					{
						tty_flush();
						if (c == 112)
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					usleep(500*1000);
				}
			}
			else {
				printf( "\nPress 'p' to pause...\n");
			}
		}
		#endif
		unsigned char TargetHash[MAX_HASH_LEN];
		int nHashLen;
		ParseHash(vHash[nHashIndex], TargetHash, nHashLen);
		if (nHashLen != CChainWalkContext::GetHashLen())
			printf("debug: nHashLen mismatch\n");

		// Request ChainWalk
		bool fNewlyGenerated;
//		printf("Requesting walk...");
		 

		uint64* pStartPosIndexE = m_cws.RequestWalk(TargetHash,
													nHashLen,
													CChainWalkContext::GetHashRoutineName(),
													CChainWalkContext::GetPlainCharsetName(),
													CChainWalkContext::GetPlainLenMin(),
													CChainWalkContext::GetPlainLenMax(),
													CChainWalkContext::GetRainbowTableIndex(),
													nRainbowChainLen,
													fNewlyGenerated,
													debug,
													sPrecalcPathName);
//		printf("done!\n");
//		printf("debug: using %s walk for %s\n", fNewlyGenerated ? "newly generated" : "existing",
//												vHash[nHashIndex].c_str());

		if (fNewlyGenerated)
		{
			timeval tv;
			timeval tv2;
			timeval final;

			gettimeofday( &tv, NULL );

			printf("Pre-calculating hash %lu of %lu.%-20s\r", 
				(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");
			threadPool.clear();
			pThreads.clear();
			
			uint32 thread_ID;
			for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
			{
				rcrackiThread* r_Thread = new rcrackiThread(TargetHash, thread_ID, nRainbowChainLen, maxThreads, pStartPosIndexE);
				if (r_Thread)
				{
					pthread_t pThread;
					int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

					if( returnValue != 0 )
					{
						printf("pThread creation failed, returnValue: %d\n", returnValue);
					}
					else
					{
						pThreads.push_back(pThread);
					}

					threadPool.push_back(r_Thread);
				}
				else 
				{
					printf("r_Thread creation failed!\n");
				}
			}
			
			//printf("%d r_Threads created\t\t\n", threadPool.size());
			
			for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
			{
				pthread_t pThread = pThreads[thread_ID];
				int returnValue = pthread_join(pThread, NULL);
				if( returnValue != 0 )
				{
					printf("pThread join failed, returnValue: %d\n", returnValue);
				}
					
				rcrackiThread* rThread = threadPool[thread_ID];
				nChainWalkStep += rThread->GetChainWalkStep();
				delete rThread;
			}

			m_cws.StoreToFile(pStartPosIndexE, TargetHash, nHashLen);
			gettimeofday( &tv2, NULL );
			final = sub_timeofday( tv2, tv );
			
			float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;

			m_fTotalPrecalculationTime += fTime;
			m_fTotalCryptanalysisTime -= fTime;

			//printf("\npStartPosIndexE[0]: %s\n", uint64tostr(pStartPosIndexE[0]).c_str());
			//printf("\npStartPosIndexE[nRainbowChainLen-2]: %s\n", uint64tostr(pStartPosIndexE[nRainbowChainLen-2]).c_str());

			printf("%-50s\r", "");

			if ( debug )
				printf("pre-calculation time: %.2f s\n", fTime);
		}

		threadPool.clear();
		pThreads.clear();

		//printf("Checking false alarms for hash %d of %d.\t\t\r", nHashIndex+1, vHash.size());
		printf("Checking false alarms for hash %lu of %lu.%-20s\r",
			(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");

		int i;
		for (i = 0; i < maxThreads; i++)
		{
			rcrackiThread* r_Thread = new rcrackiThread(TargetHash);
			threadPool.push_back(r_Thread);
		}

		uint32 thread_ID = 0;
		int nPos;
		for (nPos = nRainbowChainLen - 2; nPos >= 0; nPos--)
		{
			uint64 nIndexEOfCurPos = pStartPosIndexE[nPos];
		
			// Search matching nIndexE
			RainbowChain *pChainFound = BinarySearch(pChain, nRainbowChainCount, nIndexEOfCurPos, pIndex, nIndexSize, nChainStart);
			if (pChainFound != NULL) // For perfected indexed tables we only recieve 1 result (huge speed increase!)
			{
				rcrackiThread* rThread = threadPool[thread_ID];
				rThread->AddAlarmCheck(pChainFound, nPos);
				if (thread_ID < (unsigned long)maxThreads - 1 ) {
					thread_ID++;
				} else {
					thread_ID = 0;
				}
			}
		}

		for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
		{
			rcrackiThread* r_Thread = threadPool[thread_ID];
			pthread_t pThread;

			int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

			if( returnValue != 0 )
			{
				printf("pThread creation failed, returnValue: %d\n", returnValue);
			}
			else
			{
				pThreads.push_back(pThread);
			}
		}
		
		//printf("%d r_Threads created\t\t\n", threadPool.size());

		bool foundHashInThread = false;
		for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
		{
			rcrackiThread* rThread = threadPool[thread_ID];
			pthread_t pThread = pThreads[thread_ID];

			int returnValue = pthread_join(pThread, NULL);
			if( returnValue != 0 )
			{
				printf("pThread join failed, returnValue: %d\n", returnValue);
			}

			nChainWalkStepDueToFalseAlarm += rThread->GetChainWalkStepDueToFalseAlarm();
			nFalseAlarm += rThread->GetnFalseAlarm();

			if (rThread->FoundHash() && !foundHashInThread) {
				//printf("\t\t\t\t\t\t\r");
				printf("%-50s\r", "");
				printf("plaintext of %s is %s\n", rThread->GetHash().c_str(), rThread->GetPlain().c_str());
				if (writeOutput)
				{
					if (!writeResultLineToFile(outputFile, rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary()))
						printf("Couldn't write this result to file!\n");
				}
				hs.SetPlain(rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary());
				
				FILE* file = fopen(sSessionPathName.c_str(), "a");
				if (file!=NULL)
				{
					string buffer = "sHash=" + rThread->GetHash() + ":" + rThread->GetBinary() + ":" + rThread->GetPlain() + "\n";
					fputs (buffer.c_str(), file);
					fclose (file);
				}

				m_cws.DiscardWalk(pStartPosIndexE);
				foundHashInThread = true;
			}
			//pthread
			delete rThread;
		}

		pThreads.clear();
		threadPool.clear();

		//printf("\t\t\t\t\r");
		//printf("pChainFounds: %d\n", pChainsFound.size());
//NEXT_HASH:;
	}
	//printf("\t\t\t\t\t\t\t\r");
	printf("%-50s\r", "");
	pThreads.clear();
	threadPool.clear();
	pthread_attr_destroy(&attr);

	//printf("debug: chain walk step: %d\n", nChainWalkStep);
	//printf("debug: false alarm: %d\n", nFalseAlarm);
	//printf("debug: chain walk step due to false alarm: %d\n", nChainWalkStepDueToFalseAlarm);

	m_nTotalChainWalkStep += nChainWalkStep;
	m_nTotalFalseAlarm += nFalseAlarm;
	m_nTotalChainWalkStepDueToFalseAlarm += nChainWalkStepDueToFalseAlarm;
}

void CCrackEngine::SearchRainbowTable(string sPathName, CHashSet& hs)
{
	// Did we already go through this file in this session?
	if (resumeSession)
	{
		vector<string> sessionFinishedPathNames;
		if (ReadLinesFromFile(sProgressPathName.c_str(), sessionFinishedPathNames))
		{
			uint32 i;
			for (i = 0; i < sessionFinishedPathNames.size(); i++)
			{
				if (sessionFinishedPathNames[i] == sPathName)
				{
					printf("Skipping %s\n", sPathName.c_str());
					return;
				}
			}
		}
	}

	// FileName
#ifdef _WIN32
	string::size_type nIndex = sPathName.find_last_of('\\');
#else
	string::size_type nIndex = sPathName.find_last_of('/');
#endif
	string sFileName;
	if (nIndex != string::npos)
		sFileName = sPathName.substr(nIndex + 1);
	else
		sFileName = sPathName;

	// Info
	printf("%s:\n", sFileName.c_str());

	// Setup
	int nRainbowChainLen, nRainbowChainCount;
	if (!CChainWalkContext::SetupWithPathName(sPathName, nRainbowChainLen, nRainbowChainCount))
		return;
	//printf("keyspace: %llu\n", CChainWalkContext::GetPlainSpaceTotal());
	// Already finished?
	if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
	{
		printf("this table contains hashes with length %d only\n", CChainWalkContext::GetHashLen());
		return;
	}

	// Open
	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file != NULL)
	{
		// XXX file type detection code needs a serious overhaul
		// File length check
		bool doOldFormat = CChainWalkContext::isOldFormat();
		bool doRti2Format = CChainWalkContext::isRti2Format();
		uint32 sizeOfChain;
		bool fVerified = false;
		long nFileLen = GetFileLen(file);

		if (doOldFormat)
			sizeOfChain = 16;
		else
			sizeOfChain = 8;

		if ( ( (unsigned long)nFileLen % sizeOfChain != 0 || nRainbowChainCount * sizeOfChain != (unsigned long)nFileLen) && doRti2Format == false )
			printf("file length mismatch\n");
		else
		{
			fseek(file, 0, SEEK_SET);
			timeval tv;
			timeval tv2;
			timeval final;

			unsigned int bytesForChainWalkSet = hs.GetStatHashTotal() * (nRainbowChainLen-1) * 8;
			if (debug) printf("Debug: Saving %u bytes of memory for chainwalkset.\n", bytesForChainWalkSet);

			uint64 nAllocatedSize;

			if (doRti2Format || doOldFormat)
			{
				RTI2Reader *pReader = NULL;

				if(doRti2Format)

				if (debug)
				{
					if ( doOldFormat )
						printf("Debug: This is a table in the old .rt format.\n");
					else if ( doRti2Format )
						printf("Debug: This is a table in the .rti2 format.\n");
				}

				static CMemoryPool mp(bytesForChainWalkSet, debug, maxMem);
				RainbowChainO* pChain = NULL;
				if(doRti2Format)
					pChain = (RainbowChainO*)mp.Allocate(pReader->GetChainsLeft() * 16, nAllocatedSize);
				else
					pChain = (RainbowChainO*)mp.Allocate(nFileLen, nAllocatedSize);

				#if defined(_WIN32) && !defined(__GNUC__)
					if (debug) printf("Allocated %I64 bytes, filelen %ld\n", nAllocatedSize, nFileLen);
				#else
					if (debug) printf("Allocated %llu bytes, filelen %ld\n", nAllocatedSize, nFileLen);
				#endif

				if (pChain != NULL)
				{
					nAllocatedSize = nAllocatedSize / sizeOfChain * sizeOfChain;		// Round to sizeOfChain boundary

					//fseek(file, 0, SEEK_SET);
					//bool fVerified = false;
					while (true)	// Chunk read loop
					{
						if (ftell(file) == nFileLen)
							break;

						// Load table chunk
						if (debug) printf("reading...\n");
						unsigned int nDataRead = 0;
						gettimeofday( &tv, NULL );
						if ( doRti2Format )
						{
							nDataRead = nAllocatedSize / 16;
							if(pReader->GetChainsLeft() <= 0) // No more data
								break; 
							pReader->ReadChains(nDataRead, pChain);

							nDataRead *= 8; // Convert from chains read to bytes
						}
						else
						{
							nDataRead = fread(pChain, 1, nAllocatedSize, file);
						}
						gettimeofday( &tv2, NULL );
						final = sub_timeofday( tv2, tv );

						float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
						printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
						m_fTotalDiskAccessTime += fTime;

						int nRainbowChainCountRead;

						if(doRti2Format)
							nRainbowChainCountRead = nDataRead / 8;
						else
							nRainbowChainCountRead = nDataRead / 16;

						// Verify table chunk
						if (!fVerified)
						{
							printf("verifying the file...\n");

							// Chain length test
							int nIndexToVerify = nRainbowChainCountRead / 2;
							CChainWalkContext cwc;
							cwc.SetIndex(pChain[nIndexToVerify].nIndexS);
							int nPos;
							for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
							{
								cwc.IndexToPlain();
								cwc.PlainToHash();
								cwc.HashToIndex(nPos);
							}
							if (cwc.GetIndex() != pChain[nIndexToVerify].nIndexE)
							{
								printf("rainbow chain length verify fail\n");
								break;
							}

							// Chain sort test
							int i;
							for (i = 0; i < nRainbowChainCountRead - 1; i++)
							{
								if (pChain[i].nIndexE > pChain[i + 1].nIndexE)
									break;
							}
							if (i != nRainbowChainCountRead - 1)
							{
								printf("this file is not sorted\n");
								break;
							}

							fVerified = true;
						}

						// Search table chunk
						gettimeofday( &tv, NULL );
						SearchTableChunkOld(pChain, nRainbowChainLen, nRainbowChainCountRead, hs);
						gettimeofday( &tv2, NULL );
						final = sub_timeofday( tv2, tv );
						fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
						printf("cryptanalysis time: %.2f s\n", fTime);
						m_fTotalCryptanalysisTime += fTime;

						// Already finished?
						if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
							break;

/*
	// XXX eliminated by PB - check correctness
						// finished the current table
						if( doRti2Format && nDataToRead > (nDataRead / 8) )
						{
							delete pReader;
							break;
						}
*/
					}
				}
				else
					printf("memory allocation fail\n");
				
				//delete pChain;
			}
			else
			{
				static CMemoryPool mpIndex(bytesForChainWalkSet, debug, maxMem);
				uint64 nAllocatedSizeIndex;

				//int nIndexSize = 0;
				//IndexChain *pIndex = NULL;

				FILE* fIndex = fopen(((string)(sPathName + string(".index"))).c_str(), "rb");
				if(fIndex != NULL)
				{
					// File length check
					long nFileLenIndex = GetFileLen(fIndex);
					//unsigned int nRows = nFileLenIndex / 11;
					//unsigned int nSize = nRows * sizeof(IndexChain);
					//printf("Debug: 8\n");
					if (nFileLenIndex % 11 != 0)
						printf("index file length mismatch (%ld bytes)\n", nFileLenIndex);
					else
					{
						//printf("index nSize: %d\n", nSize);
						//pIndex = (IndexChain*)new unsigned char[nSize];
						IndexChain *pIndex = (IndexChain*)mpIndex.Allocate(nFileLenIndex, nAllocatedSizeIndex);
						#ifdef _WIN32
							if (debug) printf("Debug: Allocated %I64u bytes for index with filelen %ld\n", nAllocatedSizeIndex, nFileLenIndex);
						#else
							if (debug) printf("Debug: Allocated %llu bytes for index with filelen %ld\n", nAllocatedSizeIndex, nFileLenIndex);
						#endif
				
						static CMemoryPool mp(bytesForChainWalkSet + nAllocatedSizeIndex, debug, maxMem);
						
						if (pIndex != NULL && nAllocatedSizeIndex > 0)
						{
							nAllocatedSizeIndex = nAllocatedSizeIndex / sizeof(IndexChain) * sizeof(IndexChain);		// Round to sizeOfIndexChain boundary
						
							fseek(fIndex, 0, SEEK_SET);

							while ( ftell(fIndex) != nFileLenIndex )	// Index chunk read loop
							{
								// Load index chunk
#ifdef _WIN32
								if (debug) printf("Debug: Setting index to 0x00 in memory, %I64u bytes\n", nAllocatedSizeIndex);
#else
								if (debug) printf("Debug: Setting index to 0x00 in memory, %llu bytes\n", nAllocatedSizeIndex);
#endif
								memset(pIndex, 0x00, nAllocatedSizeIndex);
								printf("reading index... ");
								gettimeofday( &tv, NULL );
								unsigned int nDataRead = fread(pIndex, 1, nAllocatedSizeIndex, fIndex);
								gettimeofday( &tv2, NULL );
								final = sub_timeofday( tv2, tv );

								float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
								printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
								m_fTotalDiskAccessTime += fTime;
							
								//nIndexSize = nFileLenIndex / 11;
								int nIndexChainCountRead = nDataRead / sizeof(IndexChain);
								//fclose(fIndex);
								unsigned int nCoveredRainbowTableChains = 0;
								for(int i = 0; i < nIndexChainCountRead; i++)
								{
									nCoveredRainbowTableChains += pIndex[i].nChainCount;
								}

								//RainbowChain* pChain = (RainbowChain*)mp.Allocate(nFileLen, nAllocatedSize);
								RainbowChain* pChain = (RainbowChain*)mp.Allocate(nCoveredRainbowTableChains * sizeOfChain, nAllocatedSize);
								#ifdef _WIN32
									if (debug) printf("Debug: Allocated %I64u bytes for %u chains, filelen %lu\n", nAllocatedSize, nCoveredRainbowTableChains, (unsigned long)nFileLen);
								#else
									if (debug) printf("Debug: Allocated %llu bytes for %u chains, filelen %lu\n", nAllocatedSize, nCoveredRainbowTableChains, (unsigned long)nFileLen);
								#endif

								if (pChain != NULL && nAllocatedSize > 0)
								{
									nAllocatedSize = nAllocatedSize / sizeOfChain * sizeOfChain;		// Round to sizeOfChain boundary

									//bool fVerified = false;
									uint32 nProcessedChains = 0;
									while ( ftell(file) != nFileLen 
										&& nProcessedChains < nCoveredRainbowTableChains )	// Chunk read loop
									{
										// Load table chunk
										if (debug) printf("Debug: Setting pChain to 0x00 in memory\n");
										memset(pChain, 0x00, nAllocatedSize);
										printf("reading table... ");
										gettimeofday( &tv, NULL );
										unsigned int nDataRead = fread(pChain, 1, nAllocatedSize, file);
										gettimeofday( &tv2, NULL );
										final = sub_timeofday( tv2, tv );

										float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
										printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
										m_fTotalDiskAccessTime += fTime;
										int nRainbowChainCountRead = nDataRead / sizeOfChain;
										// Verify table chunk (Too lazy to implement this)
										
										if (!fVerified)
										{
											printf("verifying the file... ");

											// Chain length test
											int nIndexToVerify = nRainbowChainCountRead / 2;
											CChainWalkContext cwc;
											uint64 nIndexS;
											nIndexS = pChain[nIndexToVerify].nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes

											//printf("nIndexS: %s\n", uint64tostr(nIndexS).c_str());
											cwc.SetIndex(nIndexS);
											
											int nPos;
											for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
											{
												cwc.IndexToPlain();
												cwc.PlainToHash();
												cwc.HashToIndex(nPos);
											}

											uint64 nEndPoint = 0;

											//for(int i = 0; i < nIndexSize; i++)
											for(int i = 0; i < nIndexChainCountRead; i++)
											{
												if(nIndexToVerify >= pIndex[i].nFirstChain && nIndexToVerify < pIndex[i].nFirstChain + pIndex[i].nChainCount) // We found the matching index
												{ // Now we need to seek nIndexToVerify into the chains
													nEndPoint += (pIndex[i].nPrefix & 0x000000FFFFFFFFFFULL) << 16; // & 0x000000FFFFFFFFFFULL for first 5 bytes
													//printf("nPrefix: %s\n", uint64tostr(pIndex[i].nPrefix & 0x000000FFFFFFFFFF).c_str());
													//printf("nFirstChain: %d\n", pIndex[i].nFirstChain);
													//printf("nChainCount: %d\n", pIndex[i].nChainCount);
													nEndPoint += pChain[nIndexToVerify].nIndexE;
													break;
												}
											}

											if (cwc.GetIndex() != nEndPoint)
											{
												printf("rainbow chain length verify fail\n");
												break;
											}

											fVerified = true;
											printf("ok\n");
										}

										// Search table chunk
										gettimeofday( &tv, NULL );
										float preTime = m_fTotalCryptanalysisTime;

										SearchTableChunk(pChain, nRainbowChainLen, nRainbowChainCountRead, hs, pIndex, nIndexChainCountRead, nProcessedChains);
										float postTime = m_fTotalCryptanalysisTime;
										gettimeofday( &tv2, NULL );
										final = sub_timeofday( tv2, tv );

										fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
										printf("cryptanalysis time: %.2f s\n", fTime + postTime - preTime);
										m_fTotalCryptanalysisTime += fTime;
										nProcessedChains += nRainbowChainCountRead;
										// Already finished?
										if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
											break;
									}
								}
								else printf("memory allocation failed for rainbow table\n");

								//delete pChain;
							}
						}
						else printf("memory allocation failed for index\n");
					}		
				}
				else 
				{
					printf("Can't load index\n");
					return;
				}
				fclose(fIndex);
				
				//delete pIndex;
			}
		}
		fclose(file);

		if (debug) printf("Debug: writing progress to %s\n", sProgressPathName.c_str());
		FILE* file = fopen(sProgressPathName.c_str(), "a");
		if (file!=NULL)
		{
			string buffer = sPathName + "\n";
			fputs (buffer.c_str(), file);
			fclose (file);
		}
	}
	else
		printf("can't open file\n");
}

void CCrackEngine::Run(vector<string> vPathName, CHashSet& hs, int i_maxThreads, uint64 i_maxMem, bool resume, bool bDebug)
{
#ifndef _WIN32
	tty_init();
#endif
	resumeSession = resume;
	debug = bDebug;

	maxThreads = i_maxThreads;
	maxMem = i_maxMem;
	// Reset statistics
	ResetStatistics();

	// XXX it's not like the STL has a sort method...
	// Sort vPathName (CChainWalkSet need it)
	uint32 i, j;
	for (i = 0; i < vPathName.size() - 1; i++)
		for (j = 0; j < vPathName.size() - i - 1; j++)
		{
			if (vPathName[j] > vPathName[j + 1])
			{
				string sTemp;
				sTemp = vPathName[j];
				vPathName[j] = vPathName[j + 1];
				vPathName[j + 1] = sTemp;
			}
		}

	// Run
	for (i = 0; i < vPathName.size() && hs.AnyhashLeft(); i++)
	{
		SearchRainbowTable(vPathName[i], hs);
		printf("\n");
	}

	// delete precalc files
	if (!keepPrecalcFiles)
		m_cws.removePrecalcFiles();

#ifndef _WIN32
	tty_done();
#endif
}

void CCrackEngine::setOutputFile(string sPathName)
{
	writeOutput = true;
	outputFile = sPathName;
}

void CCrackEngine::setSession(string sSession, string sProgress, string sPrecalc, bool keepPrecalc)
{
	sSessionPathName = sSession;
	sProgressPathName = sProgress;
	sPrecalcPathName = sPrecalc;
	keepPrecalcFiles = keepPrecalc;
}

float CCrackEngine::GetStatTotalDiskAccessTime()
{
	return m_fTotalDiskAccessTime;
}
/*float CCrackEngine::GetWastedTime()
{
	return m_fIndexTime;
}*/
float CCrackEngine::GetStatTotalCryptanalysisTime()
{
	return m_fTotalCryptanalysisTime;
}

float CCrackEngine::GetStatTotalPrecalculationTime()
{
	return m_fTotalPrecalculationTime;
}

uint64 CCrackEngine::GetStatTotalChainWalkStep()
{
	return m_nTotalChainWalkStep;
}

int CCrackEngine::GetStatTotalFalseAlarm()
{
	return m_nTotalFalseAlarm;
}

uint64 CCrackEngine::GetStatTotalChainWalkStepDueToFalseAlarm()
{
	return m_nTotalChainWalkStepDueToFalseAlarm;
}

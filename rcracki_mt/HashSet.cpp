/*
	RainbowCrack - a general propose implementation of Philippe Oechslin's faster time-memory trade-off technique.

	Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
*/

#ifdef _WIN32
	#pragma warning(disable : 4786 4267 4018)
#endif

#include "HashSet.h"

CHashSet::CHashSet()
{
}

CHashSet::~CHashSet()
{
}

void CHashSet::AddHash(string sHash)
{
	if (sHash == "aad3b435b51404ee")
		return;

	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
			return;
	}

	//printf("debug: adding hash %s\n", sHash.c_str());

	m_vHash.push_back(sHash);
	m_vFound.push_back(false);
	m_vPlain.push_back("");
	m_vBinary.push_back("");
}

string CHashSet::GetHashInfo(int i)
{
	string found;
	if (m_vFound[i])
		found = "1";
	else
		found = "0";

	string buffer = m_vHash[i] + ":" + found + ":" + m_vPlain[i] + ":" + m_vBinary[i];

	return buffer;
}

bool CHashSet::AnyhashLeft()
{
	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			return true;
	}

	return false;
}

bool CHashSet::AnyHashLeftWithLen(int nLen)
{
	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			if (m_vHash[i].size() == (unsigned long)nLen * 2)
				return true;
	}

	return false;
}

void CHashSet::GetLeftHashWithLen(vector<string>& vHash, int nLen)
{
	vHash.clear();

	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			if (m_vHash[i].size() == (unsigned long)nLen * 2)
				vHash.push_back(m_vHash[i]);
	}
}

void CHashSet::AddHashInfo(string sHash, bool found, string sPlain, string sBinary)
{
	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
			return;
	}

	m_vHash.push_back(sHash);
	m_vFound.push_back(found);
	m_vPlain.push_back(sPlain);
	m_vBinary.push_back(sBinary);
}

void CHashSet::SetPlain(string sHash, string sPlain, string sBinary)
{
	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
		{
			m_vFound[i]    = true;
			m_vPlain[i]    = sPlain;
			m_vBinary[i]   = sBinary;
			return;
		}
	}
}

bool CHashSet::GetPlain(string sHash, string& sPlain, string& sBinary)
{
	if (sHash == "aad3b435b51404ee")
	{
		sPlain  = "";
		sBinary = "";
		return true;
	}

	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
		{
			if (m_vFound[i])
			{
				sPlain  = m_vPlain[i];
				sBinary = m_vBinary[i];
				return true;
			}
		}
	}

	return false;
}

int CHashSet::GetStatHashFound()
{
	int nHashFound = 0;
	UINT4 i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vFound[i])
			nHashFound++;
	}

	return nHashFound;
}

int CHashSet::GetStatHashTotal()
{
	return m_vHash.size();
}

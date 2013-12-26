/*Copyright (C) 2013 Daniel Sim

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#ifndef UNICODE
	#define UNICODE
#endif

#include <Windows.h>
#include <vector>
#include <string>
#include <set>
#include <cstdint>
#include <cstring>

using namespace std;

#define CODE_LENGTH_MAX 12

struct __declspec(dllexport) UNICODE_T {
	char data[CODE_LENGTH_MAX];

	UNICODE_T(const char *l) {
		strncpy(data, l, CODE_LENGTH_MAX);
	}
	UNICODE_T() {
		data[0] = 0;
	}

	UNICODE_T &operator=(const UNICODE_T &l);

};
typedef int32_t	ssize_t;


public class __declspec(dllexport) UnicodeDB
{

private:
	HANDLE indexHandle, dataHandle;
	HANDLE indexMap, dataMap;
	unsigned char *indexMapView, *dataMapView;
	ssize_t indexMapSize, dataMapSize;

	int findCodes( const char *word, ssize_t lower, ssize_t upper, set<UNICODE_T> &list, set<UNICODE_T> &fuzzy_list );
	ssize_t nextIndexWord(ssize_t search, ssize_t offset, ssize_t &descr_offset, ssize_t &entry_length);
	ssize_t nextIndexData(ssize_t search, ssize_t offset, ssize_t &descr_offset, ssize_t &entry_length);

	
public:
	UnicodeDB(const wchar_t*  indexPath, const wchar_t*  dataPath);
	virtual ~UnicodeDB(void);

	int findCandidates( const vector<char *> &wordlist, set<UNICODE_T> &list, set<UNICODE_T> &fuzzy_list );
	WCHAR * findDescription( UNICODE_T code, ssize_t lower, ssize_t upper );

	
};

bool operator<(const UNICODE_T &l, const UNICODE_T &r);

/*
Copyright (c) 2014 Daniel Sim

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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


class __declspec(dllexport) UnicodeDB
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

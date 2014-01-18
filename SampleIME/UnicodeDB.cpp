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

#include "UnicodeDB.h"

#include <algorithm>
#include <iterator>
#include <tchar.h>

static void copy_matches
	(set<UNICODE_T> &out, unsigned char *buf, ssize_t length) {
	int i;
	ssize_t offset = 0;
	UNICODE_T item;

	for (i = 0; i <= length; i++) {
		if (buf[i] == ' ' || i == length) {
			int code_length = min(CODE_LENGTH_MAX - 1, i - offset);

			if (code_length) {
				strncpy(item.data, (char*)buf + offset, code_length);
				item.data[code_length] = 0;
				out.insert(item);
			}

			offset = i + 1;
		}
	}
}


bool operator<(const UNICODE_T &l, const UNICODE_T &r) {
	return strncmp(l.data, r.data, CODE_LENGTH_MAX) < 0;
}

UNICODE_T &UNICODE_T::operator=(const UNICODE_T &l) {
	if (this == &l) {
		return *this;
	}
	strncpy(this->data, l.data, CODE_LENGTH_MAX);
	return *this;
}

int UnicodeDB::findCodes 
( const char *word, ssize_t lower, ssize_t upper, set<UNICODE_T> &list, set<UNICODE_T> &fuzzy_list )
{
	ssize_t search, j;
	ssize_t entry_offset, entry_length;
	int result;

	ssize_t wordlen = strlen(word);

	if (upper == -1)
		upper = this->indexMapSize;

	if (lower >= upper) {
		return 0;
	}

	search = (lower + upper) / 2;
	while (search > lower) {
		if (this->indexMapView[search] == '\n') {
			search++;
			break;
		}
		search--;
	}

	j = this->nextIndexWord( search, 0, entry_offset, entry_length );

	
	char *LOOK_ENTRY = (char*)this->indexMapView + entry_offset;

	/* assume word has the NULL byte */
	if ( wordlen == entry_length &&
		!strncmp((char*)this->indexMapView + entry_offset, word, wordlen) ) {
		// exact match

		int descr_length, descr_offset;
		
		j = this->nextIndexData(search, j, descr_offset, descr_length);

		copy_matches(list, this->indexMapView + descr_offset, descr_length);

		// check for further matches (fuzzy)
		if (wordlen >= 3) {
			while (true) {
				int entry_length;

				j = this->nextIndexWord(search, j, entry_offset, entry_length);

				if ( entry_length >= wordlen &&
					/* starts with word */
					!strncmp(
						word,
						(char*)this->indexMapView + entry_offset,
						wordlen) ) {
					j = this->nextIndexData(search, j, descr_offset, descr_length);

					copy_matches(fuzzy_list, this->indexMapView + descr_offset, descr_length);
				}
				else {
					break;
				}
			}
		}
		return 0;
	}
	else if ( (result = strncmp((char*)this->indexMapView + entry_offset, word, entry_length)) < 0 ||
		result == 0 && entry_length < wordlen) {
		// entries do not start with $word yet
		set<UNICODE_T> rv, fuz_rv;

		while (this->indexMapView[search + j] != '\n') {
			j++;
		}
		j++;

		// check for further matches
		this->findCodes(word, search + j, upper, rv, fuz_rv);

		if ( !rv.empty() || !fuz_rv.empty() ) {
			swap(rv, list);
			swap(fuz_rv, fuzzy_list);
			return 0;
		}

		if (wordlen >= 3) {

			// copy into fuzzy matches as long as next word starts with word
			while (1) {
				int next_word_length, next_word_offset, descr_offset, descr_length;

				j = this->nextIndexWord(search, j, next_word_offset, next_word_length);
				char *DBG_USE = (char*)this->indexMapView + next_word_offset;

				if ( next_word_length >= wordlen &&
					/* starts with word */
					!strncmp(
						(char *) this->indexMapView + next_word_offset,
						word,
						wordlen) ) {

							
					j = this->nextIndexData(search, j, descr_offset, descr_length);
					copy_matches(fuzzy_list, this->indexMapView + descr_offset, descr_length);
				}
				else {
					break;
				}
			}
		}

		return 0;
	}
	else {
		return this->findCodes(word, lower, search, list, fuzzy_list);
	}	
}

ssize_t UnicodeDB::nextIndexWord(ssize_t search, ssize_t offset, ssize_t &entry_offset, ssize_t &entry_length)
{
	int trailing_spaces = 0;

	while (search + offset < this->indexMapSize &&
		this->indexMapView[search + offset] != '\n') {

		if (isspace( this->indexMapView[search+offset] )) {
			offset ++;
		}
		else {
			break;
		}
	}

	entry_length = 0;
	entry_offset = search + offset;

	while (search + offset < this->indexMapSize &&
		this->indexMapView[search + offset] != ';') {
		entry_length++;
		offset++;
	}
	return (offset + 1);
}
ssize_t UnicodeDB::nextIndexData(ssize_t search, ssize_t offset, ssize_t &descr_offset, ssize_t &descr_length)
{
	int trailing_spaces = 0;

	while (search + offset < this->indexMapSize &&
		this->indexMapView[search + offset] != '\n') {

		if (isspace( this->indexMapView[search+offset] )) {
			offset ++;
		}
		else {
			break;
		}
	}
	
	descr_offset = search + offset;
	descr_length = 0;

	while (search + offset < this->indexMapSize &&
		this->indexMapView[search + offset] != '\n') {
			
		if (isspace( this->indexMapView[search+offset] )) {
			trailing_spaces ++;
		}
		else {
			trailing_spaces = 0;
		}
		offset ++;
		descr_length ++;
	}
	descr_length -= trailing_spaces;

	return (offset + 1);
}

	
/*
findCandidates:
	Given a list of words, returns the list of codes.
	The list is split into exact matches, and fuzzy matches.
*/
int UnicodeDB::findCandidates
( const vector<char *> &wordlist, set<UNICODE_T> &list, set<UNICODE_T> &fuzzy_list )
{
	bool first_time = true; // because we don't support universal set >.<

	for (auto it = wordlist.begin();
		it != wordlist.end();
		++it) {
		const char *word = *it;
		set<UNICODE_T> wlist, wfuzzy_list;

		this->findCodes(word, 0, -1, wlist, wfuzzy_list);

		if (first_time) {
			// if empty list, add everything to set first.

			// list = wlist
			// fuzzy = wfuzzy | wlist
			
			// match set
			list = wlist;
			set_union(wlist.begin(), wlist.end(),
				wfuzzy_list.begin(), wfuzzy_list.end(),
				inserter(fuzzy_list, fuzzy_list.begin()));

			first_time = false;
		}
		else {
			// otherwise...
			// list = list & wlist
			// fuzzy = fuzzy & (wfuzzy | wlist)
			set<UNICODE_T> tmpset1, tmpset2;

			set_intersection(list.begin(), list.end(),
				wlist.begin(), wlist.end(),
				inserter(tmpset1, tmpset1.begin()));
			swap(tmpset1, list);

			tmpset1.clear();
			set_union(wlist.begin(), wlist.end(),
				wfuzzy_list.begin(), wfuzzy_list.end(),
				inserter(tmpset1, tmpset1.begin()));
			set_intersection(tmpset1.begin(), tmpset1.end(),
				fuzzy_list.begin(), fuzzy_list.end(),
				inserter(tmpset2, tmpset2.begin()));
			swap(tmpset2, fuzzy_list);
		}
	}
	// get rid of ''
	{
		UNICODE_T emp("");
		set<UNICODE_T> tmpset;

		list.erase( emp );
		fuzzy_list.erase( emp );

		set_difference( 
			fuzzy_list.begin(), fuzzy_list.end(),
			list.begin(), list.end(),
			inserter(tmpset, tmpset.begin()) );

		swap(tmpset, fuzzy_list);
	}
	return 0;
}

/* Given a code, returns the description of the code. Return value needs to be freed */
WCHAR * UnicodeDB::findDescription
( UNICODE_T code, ssize_t lower, ssize_t upper ) {
	ssize_t j = 0;
	ssize_t search;
	ssize_t entry_length, descr_offset, descr_length;
	ssize_t code_length = strlen(code.data);
	WCHAR *descr_str;
	int result;

	if (upper == -1)
		upper = this->dataMapSize;

	if (lower >= upper) {
		descr_str = L"";
		return 0;
	}

	search = (lower + upper) / 2;

	// find the start of current line.
	while (search > lower) {
		if (this->dataMapView[search] == '\n') {
			search++;
			break;
		}
		search--;
	}

	entry_length = 0;
	while ( search + j < upper &&
		this->dataMapView[search + j] != ';') {
		entry_length++;
		j++;
	}
	
	if (entry_length == code_length &&
		! (result = strncmp( (char*) this->dataMapView + search, code.data, entry_length)) ) {
		int length_required;

		j++; // skip over semicolon

		descr_offset = search + j;
		while (this->dataMapView[search + j] != ';')
			j++;

		descr_length = search + j - descr_offset;

		// convert from UTF-8 (ANSI?)
		length_required = MultiByteToWideChar(
			CP_UTF8,
			0,
			(char*) this->dataMapView + descr_offset,
			descr_length,
			NULL,
			0);
		descr_str = new WCHAR[length_required + 1];
		MultiByteToWideChar(
			CP_UTF8,
			0,
			(char*) this->dataMapView + descr_offset,
			descr_length,
			descr_str,
			length_required);
		descr_str[length_required] = 0;

		return descr_str;
	}

	// we are comparing numbers, so number of digits is significant
	if ( entry_length < code_length ||
		(entry_length == code_length && result < 0)  ) {
		// search upper
		// skip to the newline
		while (this->dataMapView[search + j] != '\n') {
			j++;
		}
		j++; // skip over newline
		return this->findDescription(code, search + j, upper );
	}
	else {
		// search lower
		return this->findDescription(code, lower, search);
	}
}

UnicodeDB::UnicodeDB (const wchar_t* indexPath, const wchar_t* dataPath)
{
	// 1. open files
	// 2. create file mappings
	this->indexHandle = CreateFile(
		indexPath, 
		GENERIC_READ, 
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (this->indexHandle == INVALID_HANDLE_VALUE) {
		goto _FailIndexHandle;
	}
	
	this->dataHandle = CreateFile(
		dataPath, 
		GENERIC_READ, 
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (this->dataHandle == INVALID_HANDLE_VALUE) {
		goto _FailDataHandle;
	}

	// create file mappings
	this->indexMap = CreateFileMapping(
		this->indexHandle,
		NULL,
		PAGE_READONLY,
		0, 0, NULL /* Need NULL label to overcome Internet Explorer protection */
		);

	if (this->indexMap == NULL)
		goto _FailIndexMap;

	this->dataMap = CreateFileMapping(
		this->dataHandle,
		NULL,
		PAGE_READONLY,
		0, 0, NULL /* Need NULL label to overcome Internet Explorer protection */
		);

	if (this->dataMap == NULL)
		goto _FailDataMap;

	this->indexMapView = (unsigned char*) MapViewOfFile(
		this->indexMap,
		FILE_MAP_READ,
		0, 0, 0);

	if (this->indexMapView == NULL)
		goto _FailIndexMapView;

	this->dataMapView = (unsigned char*) MapViewOfFile(
		this->dataMap,
		FILE_MAP_READ,
		0, 0, 0);

	if (this->dataMapView == NULL)
		goto _FailDataMapView;

	// get the file size
	this->indexMapSize = GetFileSize(this->indexHandle, NULL);
	this->dataMapSize = GetFileSize(this->dataHandle, NULL);

	if (this->indexMapSize == INVALID_FILE_SIZE ||
		this->dataMapSize == INVALID_FILE_SIZE)
		goto _Fail;

	return;

_Fail:
	UnmapViewOfFile(this->dataMapView);
	
_FailDataMapView:
	UnmapViewOfFile(this->indexMapView);

_FailIndexMapView:
	CloseHandle(this->dataMap);

_FailDataMap:
	CloseHandle(this->indexMap);

_FailIndexMap:
	DWORD error_code = GetLastError();
	CloseHandle(this->dataHandle);

_FailDataHandle:
	CloseHandle(this->indexHandle);

_FailIndexHandle:

	throw _T("Failed to map the index and/or data files");
}


UnicodeDB::~UnicodeDB(void)
{
	UnmapViewOfFile(this->dataMapView);
	UnmapViewOfFile(this->indexMapView);
	CloseHandle(this->dataMap);
	CloseHandle(this->indexMap);
	CloseHandle(this->dataHandle);
	CloseHandle(this->indexHandle);
}

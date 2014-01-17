// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include <vector>
#include "assert.h"
#include <iostream>

#define _INTSAFE_H_INCLUDED_
#include <cstdint>

using std::cout;
using std::endl;

//---------------------------------------------------------------------
// defined keyword
//---------------------------------------------------------------------
template<class VALUE>
struct _DEFINED_KEYWORD
{
    LPCWSTR _pwszKeyword;
    VALUE _value;
};

enum KEYSTROKE_FUNCTION
{
	FUNCTION_NONE = 0,
	
	// Functions for all modes
    FUNCTION_INPUT,
	FUNCTION_SEARCH_MODE,			// When the apostrophe is selected, when the buffer is empty
	FUNCTION_HEX_MODE,
    FUNCTION_CANCEL,				// Escape
	FUNCTION_SHOW_STRING,			// Refresh and re-show the candidate string

	// Functions for hex mode
	FUNCTION_CONVERT,

	// Functions for search mode
    FUNCTION_SELECT_BY_NUMBER,		// 0-9
    FUNCTION_BACKSPACE,				// Backspace
    FUNCTION_MOVE_LEFT,
    FUNCTION_MOVE_RIGHT,
    FUNCTION_MOVE_UP,
    FUNCTION_MOVE_DOWN,
    FUNCTION_MOVE_PAGE_UP,			// Page up
    FUNCTION_MOVE_PAGE_DOWN,		// Page down
    FUNCTION_MOVE_PAGE_TOP,			// Home
    FUNCTION_MOVE_PAGE_BOTTOM,		// End

	// Function for the inspector (ambiguous) mode
	FUNCTION_INSPECTOR_LEFT,
	FUNCTION_INSPECTOR_RIGHT,
};

//---------------------------------------------------------------------
// candidate list
//---------------------------------------------------------------------
enum CANDIDATE_MODE
{
    CANDIDATE_NONE = 0,
    CANDIDATE_ORIGINAL,
    CANDIDATE_PHRASE,
    CANDIDATE_INCREMENTAL,
    CANDIDATE_WITH_NEXT_COMPOSITION
};

enum InputStates {
	STATE_NORMAL,
	STATE_AMBIGUOUS,
	STATE_HEX,
	STATE_SEARCH,
};

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    InputStates State;
    KEYSTROKE_FUNCTION Function;
};

struct _PUNCTUATION
{
    WCHAR _Code;
    WCHAR _Punctuation;
};

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_ (39) WCHAR *pCLSIDString);

HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);
HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);

BOOL IsSpace(LCID locale, WCHAR wch);

//---------------------------------------------------------------------
// CCandidateListItem
//	_ItemString - candidate string
//	_FindKeyCode - tailing string
//---------------------------------------------------------------------
struct CCandidateListItem
{
//    std::wstring _ItemString;
//    std::wstring _FindKeyCode;

// TODO: remove duplicate code in CSI_KeyHandler.cpp
// TODO: decide whether to store the hex representation or to store the unicode char.
	
	std::wstring _CharDescription;
	std::wstring _CharUnicodeHex;
	wchar_t _Sequence[3];

	CCandidateListItem()  {
		_Sequence[1] = _Sequence[2] = 0;
	}

	CCandidateListItem& CCandidateListItem::operator =( const CCandidateListItem& rhs)
	{
		_CharDescription = rhs._CharDescription;
		_CharUnicodeHex = rhs._CharUnicodeHex;
		
		return *this;
	}

	const wchar_t *GetChar() {
		// Convert hex string to charcode
		uint32_t _CharCode = 0;
		
		for (int i=0; i < this->_CharUnicodeHex.length(); i++) {
			int digitValue = -1;

			if ( this->_CharUnicodeHex[i] >= 'A' && this->_CharUnicodeHex[i] <= 'F') {
				digitValue = this->_CharUnicodeHex[i] - 'A' + 10;
			}
			else if ( this->_CharUnicodeHex[i] >= 'a' && this->_CharUnicodeHex[i] <= 'f') {
				digitValue = this->_CharUnicodeHex[i] - 'a' + 10;
			}
			else if ( this->_CharUnicodeHex[i] >= '0' && this->_CharUnicodeHex[i] <= '9') {
				digitValue = this->_CharUnicodeHex[i] - '0';
			}
			else {
				assert(0);
			}

			if (digitValue == -1)
				continue;

			assert(digitValue >= 0 && digitValue <= 15);
		
			_CharCode = (_CharCode<< 4) + digitValue;
		}

		// Convert charcode to UTF-16
		CCandidateListItem::CharToString(_CharCode, _Sequence, 3);
		return _Sequence;
	}

	static HRESULT CharToString( uint32_t unicode_char, wchar_t *str, int length ) {
		if (length <= 0) return false;
		
		if (length >= 2 && (unicode_char <= 0xD7FF || (unicode_char >= 0xE000 && unicode_char <= 0xFFFF)) ) {
			// No surrogate pair needed
			str[0] = (wchar_t) unicode_char;
			str[1] = L'\0';
			return S_OK;
		}
		else if (length >= 3 && unicode_char >= 0x010000 && unicode_char <= 0x10FFFF) {
			// Surrogate pair needed
			// High surrogates: D800 - DBFF (3FF chars = 10bits)
			// Low surrogates:  DC00 - DFFF (3FF chars = 10bits)
			// Highest UTF-16:  10FFFF = 21bits
			// calculations from http://www.unicode.org/faq/utf_bom.html

			const uint16_t LEAD_OFFSET = 0xD800 - (0x10000 >> 10);

			// computations
			uint16_t lead = LEAD_OFFSET + (unicode_char >> 10);
			uint16_t trail = 0xDC00 + (unicode_char & 0x3FF);

			str[0] = (wchar_t) lead;
			str[1] = (wchar_t) trail;
			str[2] = 0;
			return S_OK;
		}
		else {
			str[0] = 0;
			return S_FALSE;
		}
	}
};

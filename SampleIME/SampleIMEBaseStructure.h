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
	FUNCTION_CLIPBOARD_MODE,
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
	STATE_CLIPBOARD,
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

	CCandidateListItem();

	CCandidateListItem& CCandidateListItem::operator =( const CCandidateListItem& rhs);

	const wchar_t *GetChar();

	void SetCodePoint(uint32_t cp);

	static HRESULT CharToString( uint32_t unicode_char, wchar_t *str, int length );
};

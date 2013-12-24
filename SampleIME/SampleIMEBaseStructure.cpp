// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Globals.h"

//---------------------------------------------------------------------
//
// CLSIDToString
//
//---------------------------------------------------------------------

const BYTE GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
};

const WCHAR HexDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR *pCLSIDString)
{
    WCHAR* pTemp = pCLSIDString;
    const BYTE* pBytes = (const BYTE *) &refGUID;

    DWORD j = 0;
    pTemp[j++] = L'{';
    for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++)
    {
        if (GuidSymbols[i] == '-')
        {
            pTemp[j++] = L'-';
        }
        else
        {
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0xF0) >> 4 ];
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0x0F) ];
        }
    }

    pTemp[j++] = L'}';
    pTemp[j] = L'\0';

    return TRUE;
}

//---------------------------------------------------------------------
//
// SkipWhiteSpace
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && IsSpace(locale, *pwszBuffer) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// FindChar
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && (*pwszBuffer != wch) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// IsSpace
//
//---------------------------------------------------------------------

BOOL IsSpace(LCID locale, WCHAR wch)
{
    WORD wCharType = 0;

    GetStringTypeEx(locale, CT_CTYPE1, &wch, 1, &wCharType);
    return (wCharType & C1_SPACE);
}


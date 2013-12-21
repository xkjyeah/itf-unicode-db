// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "EditSession.h"
#include "Globals.h"

class CKeyHandlerEditSession : public CEditSessionBase
{
public:
    CKeyHandlerEditSession(CSampleIME *pTextService, ITfContext *pContext, UINT uCode, WCHAR wch, _KEYSTROKE_STATE keyState) : CEditSessionBase(pTextService, pContext)
    {
        _uCode = uCode;
        _wch = wch;
        _KeyState = keyState;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    UINT _uCode;    // virtual key code
    WCHAR _wch;      // character code
    _KEYSTROKE_STATE _KeyState;     // key function regarding virtual key
};


typedef struct KeyHandlerEditSessionDTO
{
    KeyHandlerEditSessionDTO::KeyHandlerEditSessionDTO(TfEditCookie tFEC, _In_ ITfContext *pTfContext, UINT virualCode, WCHAR inputChar, KEYSTROKE_FUNCTION arrowKeyFunction)
    {
        ec = tFEC;
        pContext = pTfContext;
        code = virualCode;
        wch = inputChar;
        arrowKey = arrowKeyFunction;
    }

    TfEditCookie ec;
    ITfContext* pContext;
    UINT code;
    WCHAR wch;
    KEYSTROKE_FUNCTION arrowKey;
}KeyHandlerEditSessionDTO;


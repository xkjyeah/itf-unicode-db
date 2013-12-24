// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "EditSession.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"
#include "KeyHandlerEditSession.h"
#include "Compartment.h"

// 0xF003, 0xF004 are the keys that the touch keyboard sends for next/previous
#define THIRDPARTY_NEXTPAGE  static_cast<WORD>(0xF003)
#define THIRDPARTY_PREVPAGE  static_cast<WORD>(0xF004)

// Because the code mostly works with VKeys, here map a WCHAR back to a VKKey for certain
// vkeys that the IME handles specially
__inline UINT VKeyFromVKPacketAndWchar(UINT vk, WCHAR wch)
{
    UINT vkRet = vk;
    if (LOWORD(vk) == VK_PACKET)
    {
        if (wch == L' ')
        {
            vkRet = VK_SPACE;
        }
        else if ((wch >= L'0') && (wch <= L'9'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if ((wch >= L'a') && (wch <= L'z'))
        {
            vkRet = (UINT)(L'A') + ((UINT)(L'z') - static_cast<UINT>(wch));
        }
        else if ((wch >= L'A') && (wch <= L'Z'))
        {
            vkRet = static_cast<UINT>(wch);
        }
        else if (wch == THIRDPARTY_NEXTPAGE)
        {
            vkRet = VK_NEXT;
        }
        else if (wch == THIRDPARTY_PREVPAGE)
        {
            vkRet = VK_PRIOR;
        }
    }
    return vkRet;
}

//+---------------------------------------------------------------------------
//
// _IsKeyEaten
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_IsKeyEaten(_In_ ITfContext *pContext, UINT codeIn, _Out_ UINT *pCodeOut, _Out_writes_(1) WCHAR *pwch, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    pContext;
    *pCodeOut = codeIn;

    if (pKeyState)
    {
		pKeyState->State = this->_inputState;
        pKeyState->Function = FUNCTION_NONE;
    }
    if (pwch)
    {
        *pwch = L'\0';
    }

    //
    // Map virtual key to character code
    //
    BOOL isTouchKeyboardSpecialKeys = FALSE;
    WCHAR wch = ConvertVKey(codeIn);
    *pCodeOut = VKeyFromVKPacketAndWchar(codeIn, wch); // TODO: This uses ToUnicode -- is that correct? Or do I need ToUnicodeEx

    if (pwch)
    {
        *pwch = wch;
    }

	/*
		two states:
		1. normal state (not composing)
			lets all keys pass
		2. ambiguous state
			eats all keys
			valid hexadecimal alphanum --> unicode state (with character)
		3. unicode state
			eats all keys
		4. search state
			eats all keys
		*/
    if (_IsComposing() && _inputState != STATE_NORMAL) {
		if (_inputState == STATE_AMBIGUOUS) { /* When I only have my initial 'u' */
			switch (*pCodeOut)
			{
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
							if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			case VK_RETURN: if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			case VK_ESCAPE: if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			case VK_SPACE:  if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			case VK_BACK:   if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			}
			/* If alphanumeric, we would want to add it to the keyboard buffer */
			if ( (*pCodeOut >= 0x30 && *pCodeOut <= 0x39) ||
				(*pCodeOut >= 0x41 && *pCodeOut <= 0x5A) ) {
				if (pKeyState) {
					pKeyState->Function = FUNCTION_INPUT;
				}
				return TRUE;
			}

			return TRUE; // eat but do nothing
		}
		else if (_inputState == STATE_HEX) {
			switch (*pCodeOut)
			{
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;

			case VK_ESCAPE: if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;

			case VK_RETURN:
			case VK_SPACE:  if (pKeyState) { pKeyState->Function = FUNCTION_CONVERT; } return TRUE;

			case VK_BACK:   if (pKeyState) { pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
			}
			/* If alphanumeric, we would want to add it to the keyboard buffer.
				Otherwise we eat it and don't care.
			*/
			if ( (*pCodeOut >= 0x30 && *pCodeOut <= 0x39) ||
				(*pCodeOut >= VK_NUMPAD0 && *pCodeOut <= VK_NUMPAD9 ) ||
				(*pCodeOut >= 0x41 && *pCodeOut <= 0x5A) ) {
				if (pKeyState) {
					pKeyState->Function = FUNCTION_INPUT;
				}
				return TRUE;
			}

			return TRUE; // eat but do nothing
		}
		else if (_inputState == STATE_SEARCH) {

			switch (*pCodeOut)
			{
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;

			case VK_ESCAPE: if (pKeyState) { pKeyState->Function = FUNCTION_CANCEL; } return TRUE;
			case VK_BACK:   if (pKeyState) { pKeyState->Function = FUNCTION_BACKSPACE; } return TRUE;
			}
			/* Eat everything, including spaces */
			if ( (*pCodeOut >= 0x30 && *pCodeOut <= 0x39) ||
				(*pCodeOut >= VK_NUMPAD0 && *pCodeOut <= VK_NUMPAD9 )
				) {
				if (pKeyState) {
					pKeyState->Function = FUNCTION_SELECT_BY_NUMBER;
				}
				return TRUE; // eat but do nothing
			}
			else {
				if (pKeyState) {
					pKeyState->Function = FUNCTION_INPUT;
				}
			}
			return TRUE; // eat but do nothing
		}
		return TRUE;
    }
	else {
		return FALSE;
	}
}

//+---------------------------------------------------------------------------
//
// ConvertVKey
/* converts a virtual key to some unicode character key */
//
//----------------------------------------------------------------------------

WCHAR CSampleIME::ConvertVKey(UINT code)
{
    //
    // Map virtual key to scan code
    //
    UINT scanCode = 0;
    scanCode = MapVirtualKey(code, 0);

    //
    // Keyboard state
    //
    BYTE abKbdState[256] = {'\0'};
    if (!GetKeyboardState(abKbdState))
    {
        return 0;
    }

    //
    // Map virtual key to character code
    //
    WCHAR wch = '\0';
    if (ToUnicode(code, scanCode, abKbdState, &wch, 1, 0) == 1)
    {
        return wch;
    }

    return 0;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnSetFocus(BOOL fForeground)
{
	fForeground;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';
    UINT code = 0;
    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);

    /*if (KeystrokeState.Category == CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION)
    {
        //
        // Invoke key handler edit session
        //
        KeystrokeState.Category = CATEGORY_COMPOSING;

        _InvokeKeyHandler(pContext, code, wch, (DWORD)lParam, KeystrokeState);
    }*/

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';
    UINT code = 0;

    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, &KeystrokeState);
	/* KeystrokeState is now updated */

    if (*pIsEaten)
    {
		// The original _InvokeKeyHandler
		CKeyHandlerEditSession* pEditSession = nullptr;
		HRESULT hr = E_FAIL;

		// do something instead of this keystroke
		pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, code, wch, KeystrokeState);
		if (pEditSession == nullptr)
		{
			goto Exit;
		}

		//
		// Call CKeyHandlerEditSession::DoEditSession().
		//
		// Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
		//
		hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

		pEditSession->Release();

		Exit:
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    if (pIsEaten == nullptr)
    {
        return E_INVALIDARG;
    }

    Global::UpdateModifiers(wParam, lParam);

    WCHAR wch = '\0';
    UINT code = 0;

    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, NULL);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    Global::UpdateModifiers(wParam, lParam);

    WCHAR wch = '\0';
    UINT code = 0;

    *pIsEaten = _IsKeyEaten(pContext, (UINT)wParam, &code, &wch, NULL);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten)
{
	if (IsEqualGUID(rguid, Global::SampleIMEGuidComposePreservedKey))
    {
		if (_inputState == STATE_NORMAL) {
			*pIsEaten = TRUE;
		
			// The original _InvokeKeyHandler
			CKeyHandlerEditSession* pEditSession = nullptr;
			_KEYSTROKE_STATE KeystrokeState;
			HRESULT hr = E_FAIL;

			KeystrokeState.State = _inputState;
			KeystrokeState.Function = FUNCTION_HEX_MODE;

			// do something instead of this keystroke
			pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, 0, 0, KeystrokeState);
			if (pEditSession == nullptr)
			{
				*pIsEaten = FALSE;
				goto Exit;
			}

			hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

			pEditSession->Release();
Exit:
		}
		else {
			*pIsEaten = FALSE;
		}
    }
    else
    {
        *pIsEaten = FALSE;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitKeyEventSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
    HRESULT hr = S_OK;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return FALSE;
    }

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitKeyEventSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CSampleIME::_UninitKeyEventSink()
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr)))
    {
        return;
    }

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}

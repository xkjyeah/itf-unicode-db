// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"

#include <stdint.h>

//////////////////////////////////////////////////////////////////////
//
// CSampleIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) 
        || (lResult > 0))
    {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) 
        || (lResult < 0))
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _DeleteCandidateList
//
//----------------------------------------------------------------------------

VOID CSampleIME::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{
    isForce;pContext;

    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->_EndCandidateList();

        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }
}

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _DeleteCandidateList(FALSE, pContext);
	
	_ResetNormalState();

    // just terminate the composition
    _TerminateComposition(ec, pContext);

    return S_OK;
}

HRESULT CSampleIME::_HandleInputCancel(TfEditCookie ec, _In_ ITfContext *pContext) {
	HRESULT hr = _HandleCancel(ec, pContext);

	return hr;
}


HRESULT CSampleIME::_HandleRefresh(TfEditCookie ec, _In_ ITfContext *pContext) {
	HRESULT hr = S_OK;

	if (this->_ResetDecor(ec, pContext)) {
		hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
	}

	return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _RemoveDummyCompositionForComposing(ec, _pComposition);

    _DeleteCandidateList(FALSE, pContext);

	this->_clipCandidateList.clear();
	
	_ResetNormalState();

    _TerminateComposition(ec, pContext);

    return S_OK;
}

bool CSampleIME::_ResetDecor(TfEditCookie ec, _In_ ITfContext *pContext) {
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;
	bool ready = true;
	
    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing())
    {
        _StartComposition(pContext);
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);
        pRangeComposition->Release();
        if (!isCovered)
        {
			ready = false;
        }
    }

	tfSelection.range->Release();
	return ready;
}


HRESULT CSampleIME::_HandleEnterSearchMode(TfEditCookie ec, _In_ ITfContext *pContext) {
	HRESULT hr = S_FALSE;

	assert( this->_keystrokeBuffer.length() <= CSampleIME::LENGTH_SEARCH_PREFIX );
	assert( this->_keystrokeBuffer.length() != CSampleIME::LENGTH_SEARCH_PREFIX ||
		(this->_keystrokeBuffer[0] == L'u' && this->_keystrokeBuffer[1] == L'\'') );

	this->_keystrokeBuffer.assign(L"u'");

	if (this->_ResetDecor(ec, pContext)) {
		hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		this->_inputState = STATE_SEARCH;
	}

    return hr;
}


HRESULT CSampleIME::_HandleEnterHexMode(TfEditCookie ec, _In_ ITfContext *pContext) {
	HRESULT hr = S_OK;

	assert( this->_keystrokeBuffer.length() <= CSampleIME::LENGTH_HEX_PREFIX );
	assert( this->_keystrokeBuffer.length() == 0 || this->_keystrokeBuffer[0] == L'u' );

	this->_keystrokeBuffer.assign(L"u");
	
	if (this->_ResetDecor(ec, pContext)) {
		hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		this->_inputState = STATE_AMBIGUOUS; // because we have no chars yet
	}
    return hr;
}

HRESULT CSampleIME::_HandleHexInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
	HRESULT hr = S_OK;

	if (this->_ResetDecor(ec, pContext)) {

		// Add virtual key to composition processor engine
		// Disallow overlong inputs and illegal characters
		if ( this->_keystrokeBuffer.length() < 8 &&
			((wch >= 'A' && wch <= 'F') ||
				(wch >= 'a' && wch <= 'f') ||
				(wch >= '0' && wch <= '9')) ) {
			this->_keystrokeBuffer.append(&wch, 1);

			this->_inputState = STATE_HEX;

			// Update the UI?
			hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		}
	}

    return hr;
}

HRESULT CSampleIME::_HandleHexBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
	HRESULT hr = S_OK;
	
	if (this->_ResetDecor(ec, pContext)) {

		// Delete the last character
		// Check the length and cancel if necessary.
		if (this->_keystrokeBuffer.length() > CSampleIME::LENGTH_HEX_PREFIX)
		{
			this->_keystrokeBuffer.erase( this->_keystrokeBuffer.length() - 1, 1 );

			hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);

			if (this->_keystrokeBuffer.length() == CSampleIME::LENGTH_HEX_PREFIX) {
				this->_inputState = STATE_AMBIGUOUS;
			}
			else if (this->_keystrokeBuffer.length() > CSampleIME::LENGTH_HEX_PREFIX) {
				this->_inputState = STATE_HEX;
			}
		}
		else {
			hr = _HandleCancel(ec, pContext);
			this->_inputState = STATE_NORMAL;
			this->_keystrokeBuffer.clear();
		}
	}

    return hr;
}


HRESULT CSampleIME::_HandleSearchBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
	HRESULT hr = S_OK;
	
	if (this->_ResetDecor(ec, pContext)) {

		// Delete the last character
		// Check the length and cancel if necessary.
		if (this->_keystrokeBuffer.length() > CSampleIME::LENGTH_SEARCH_PREFIX)
		{
			this->_keystrokeBuffer.erase( this->_keystrokeBuffer.length() - 1, 1 );

			hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);

			/* Handle the candidate list */
			std::vector<CCandidateListItem> candidateList;

			this->GetCandidateList(this->_keystrokeBuffer.c_str() + CSampleIME::LENGTH_SEARCH_PREFIX, candidateList);

			/* (Re)initialize the candidate list */
			hr = _CreateAndStartCandidate(ec, pContext);
			if (SUCCEEDED(hr))
			{
				_pCandidateListUIPresenter->_ClearList();
				/* Add the list to the UI */
				_pCandidateListUIPresenter->_SetHelpText(IME_HELP_TEXT_SEARCH);
				_pCandidateListUIPresenter->_SetText(candidateList, TRUE);
			}

			this->_inputState = STATE_SEARCH;
		}
		else {
			this->_DeleteCandidateList(FALSE, pContext);
			this->_inputState = STATE_AMBIGUOUS;
			this->_keystrokeBuffer.erase( this->_keystrokeBuffer.length() - 1, 1 );
			hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		}
	}

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleSearchInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
	HRESULT hr = S_OK;

	if (this->_ResetDecor(ec, pContext)) {
		// Add virtual key to composition processor engine
		this->_keystrokeBuffer.append(&wch, 1);
		
		/* Handle text application side: add the new keystroke to the candidate string */
		hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);

		/* Handle the candidate list */
		std::vector<CCandidateListItem> candidateList;

		this->GetCandidateList(this->_keystrokeBuffer.c_str() + CSampleIME::LENGTH_SEARCH_PREFIX, candidateList);

		/* (Re)initialize the candidate list */
		hr = _CreateAndStartCandidate(ec, pContext);
		if (SUCCEEDED(hr))
		{
			_pCandidateListUIPresenter->_ClearList();
			/* Add the list to the UI */
			_pCandidateListUIPresenter->_SetHelpText(IME_HELP_TEXT_SEARCH);
			_pCandidateListUIPresenter->_SetText(candidateList, TRUE);
		}
	}
	return hr;
}

//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
/*
This is called every time the candidate list is changed.

TODO: maybe we don't have to be so drastic and delete the list every time.
*/
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_CreateAndStartCandidate(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if (_pCandidateListUIPresenter)
    {
        // Recreate candidate list
        _pCandidateListUIPresenter->_EndCandidateList();
        delete _pCandidateListUIPresenter;
        _pCandidateListUIPresenter = nullptr;

        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }

    if (_pCandidateListUIPresenter == nullptr)
    {
        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
			this->_inputState,
            FALSE);
        
		if (!_pCandidateListUIPresenter)
        {
            return E_OUTOFMEMORY;
        }

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, this->GetCandidateWindowWidth());
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionFinalize
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCandidateList)
{
    HRESULT hr = S_OK;

    if (isCandidateList && _pCandidateListUIPresenter)
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        DWORD_PTR candidateLen = 0;
        const WCHAR *pCandidateString = nullptr;

        candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

        std::wstring candidateString = pCandidateString;

        if (candidateLen)
        {
            // Finalize character
            hr = _FinalizeText(ec, pContext, candidateString);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    else
    {
        // Finalize current text store strings
        if (_IsComposing())
        {
            ULONG fetched = 0;
            TF_SELECTION tfSelection;

            if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
            {
                if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
                {
                    _EndComposition(pContext);
                }

                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

HRESULT CSampleIME::_HandleSearchSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode)
{
	HRESULT hr;

    if (_pCandidateListUIPresenter)
    {
		UINT array_index = uCode ? CCandidateListUIPresenter::VKeyToArrayIndex(uCode) : 0;

		// uCode == 0 ==> we pick the selection (e.g. chosen by mouse click)
        if (!uCode || _pCandidateListUIPresenter->_SetSelectionInPage(array_index))
        {	
			const wchar_t *pCandidateString;
			DWORD_PTR candidateLen;
			std::wstring candidateString;

			candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

			if (pCandidateString == 0) return E_FAIL;

			candidateString = pCandidateString;

			// Add composing character
			this->_FinalizeText(ec, pContext, candidateString);
        }
    }

    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleHexConvert(TfEditCookie ec, _In_ ITfContext *pContext)
{
	std::wstring finalString;
	uint32_t unicode_char = 0;
	
	assert(this->_keystrokeBuffer.length() >= CSampleIME::LENGTH_HEX_PREFIX);

	// get string from _keystroke

	// convert it to a 32-bit character -- generate the surrogate pair if necessary
	for (int i=CSampleIME::LENGTH_HEX_PREFIX; i < this->_keystrokeBuffer.length(); i++) {
		int digitValue = -1;

		if ( this->_keystrokeBuffer[i] >= 'A' && this->_keystrokeBuffer[i] <= 'F') {
			digitValue = this->_keystrokeBuffer[i] - 'A' + 10;
		}
		else if ( this->_keystrokeBuffer[i] >= 'a' && this->_keystrokeBuffer[i] <= 'f') {
			digitValue = this->_keystrokeBuffer[i] - 'a' + 10;
		}
		else if ( this->_keystrokeBuffer[i] >= '0' && this->_keystrokeBuffer[i] <= '9') {
			digitValue = this->_keystrokeBuffer[i] - '0';
		}
		else {
			assert(0);
		}

		if (digitValue == -1)
			continue;

		assert(digitValue >= 0 && digitValue <= 15);
		
		unicode_char = (unicode_char << 4) + digitValue;
	}

	if (unicode_char <= 0xD7FF || (unicode_char >= 0xE000 && unicode_char <= 0xFFFF) ) {
		// No surrogate pair needed
		finalString.append( (wchar_t*) &unicode_char, 1 );
	}
	else if (unicode_char >= 0x010000 && unicode_char <= 0x10FFFF) {
		// Surrogate pair needed
		// High surrogates: D800 - DBFF (3FF chars = 10bits)
		// Low surrogates:  DC00 - DFFF (3FF chars = 10bits)
		// Highest UTF-16:  10FFFF = 21bits
		// calculations from http://www.unicode.org/faq/utf_bom.html

		const uint16_t LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
		const uint16_t TRAIL_OFFSET = 0xDC00;

		// computations
		uint16_t lead = (uint16_t) (LEAD_OFFSET + (unicode_char >> 10));
		uint16_t trail = (uint16_t) (TRAIL_OFFSET + (unicode_char & 0x3FF));

		finalString.append( (wchar_t*) &lead, 1 );
		finalString.append( (wchar_t*) &trail, 1 );
	}
	else {
		// Erroneous code. Pass an empty string
	}

	// then send it to the application
	// WARNING: we obviously don't verify the validity of the string.
	// might be a dangerous feature.
	this->_FinalizeText(ec, pContext, finalString);

	return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleSearchArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    // get the selection
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
        || fetched != 1)
    {
        // no selection, eat the keystroke
        return S_OK;
    }

    // get the composition range
    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        goto Exit;
    }

    // For incremental candidate list
    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;
}

HRESULT CSampleIME::_HandleClipSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode)
{
	HRESULT hr;

    if (_pCandidateListUIPresenter)
    {
		UINT array_index = uCode ? CCandidateListUIPresenter::VKeyToArrayIndex(uCode) : 0;

		// uCode == 0 ==> we pick the selection (e.g. chosen by mouse click)
        if (!uCode || _pCandidateListUIPresenter->_SetSelectionInPage(array_index))
        {	
			const wchar_t *pCandidateString;
			DWORD_PTR candidateLen;
			std::wstring candidateString;

			candidateLen = _pCandidateListUIPresenter->_GetSelectedCandidateString(&pCandidateString);

			if (pCandidateString == 0) return E_FAIL;

			candidateString = pCandidateString;

			this->_keystrokeBuffer.append(pCandidateString);
			this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
        }
    }

    return S_FALSE;
}
HRESULT CSampleIME::_HandleClipArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction)
{
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    // get the selection
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
        || fetched != 1)
    {
        // no selection, eat the keystroke
        return S_OK;
    }

    // For incremental candidate list
    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

Exit:
    tfSelection.range->Release();
    return S_OK;
}
HRESULT CSampleIME::_HandleClipBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{	
	HRESULT hr = S_OK;
	
	if (this->_ResetDecor(ec, pContext)) {

		// Delete the last character
		// Check the length and cancel if necessary.
		if (this->_keystrokeBuffer.length() > 0)
		{
			this->_keystrokeBuffer.erase( this->_keystrokeBuffer.length() - 1, 1 );
			
			/* check high/low surrogate */
			while (this->_keystrokeBuffer.length() &&
				IS_HIGH_SURROGATE(this->_keystrokeBuffer.at( this->_keystrokeBuffer.length() - 1 ))
				) {
				this->_keystrokeBuffer.erase( this->_keystrokeBuffer.length() - 1, 1 );
			}

			hr = this->_UpdateCandidateString(
				ec,
				pContext,
				(this->_keystrokeBuffer.length() ? this->_keystrokeBuffer : L".") );
			// this->_inputState = STATE_CLIPBOARD;
		}
		else {
			this->_DeleteCandidateList(FALSE, pContext);
			this->_inputState = STATE_AMBIGUOUS;
			this->_keystrokeBuffer = L"u";
			hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		}
	}

    return hr;
}
HRESULT CSampleIME::_HandleClipInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
	HRESULT hr = S_FALSE;
	if (this->_ResetDecor(ec, pContext)) {
		this->_keystrokeBuffer.append(&wch, 1);
		hr = this->_UpdateCandidateString(ec, pContext, this->_keystrokeBuffer);
		
		/* (Re)initialize the candidate list */
		hr = _CreateAndStartCandidate(ec, pContext);
		if (SUCCEEDED(hr))
		{
			_pCandidateListUIPresenter->_ClearList();
			_pCandidateListUIPresenter->_SetHelpText(IME_HELP_TEXT_CLIPBOARD);
			_pCandidateListUIPresenter->_SetText(_clipCandidateList, TRUE);
		}
	}
	return hr;
}

HRESULT CSampleIME::_HandleClipFinalize(TfEditCookie ec, _In_ ITfContext *pContext)
{
	HRESULT hr = S_FALSE;
	if (this->_ResetDecor(ec, pContext)) {
		std::wstring finalString = this->_keystrokeBuffer.c_str();

		hr = this->_FinalizeText(ec, pContext, finalString);
	}

	return hr;
}

HRESULT CSampleIME::_HandleEnterClipMode(TfEditCookie ec, _In_ ITfContext *pContext)
{
	HRESULT hr = S_FALSE;

	if (this->_ResetDecor(ec, pContext)) {
		vector<CCandidateListItem> &candidateList = this->_clipCandidateList;

		candidateList.clear();

		// Clear the keystroke buffer, so search engines can read while we type
		this->_keystrokeBuffer = L"";
		this->_UpdateCandidateString(ec, pContext, L"."); /* Show the dot because if candidate string is empty candidate UI is not displayed */
		this->_inputState = STATE_CLIPBOARD;

		// Read the clipboard
		if (OpenClipboard(NULL)) {
			WCHAR data[101];
			WCHAR *pdata = data;
			HANDLE clipboardData;

			clipboardData = GetClipboardData(CF_UNICODETEXT);
			if (clipboardData) {
				WCHAR *clipboardText = (WCHAR*)GlobalLock(clipboardData);

				StringCchCopy(data, 100, clipboardText);  /* only up to 100 characters */

				GlobalUnlock(clipboardData);
			}

			CloseClipboard();

			while (*pdata) {
				WCHAR &ch = *pdata;
				uint32_t codepoint = ch;
				CCandidateListItem candidate;

				++pdata;

				if (IS_HIGH_SURROGATE(ch)) {
					WCHAR &ch2 = *pdata;

					if (! IS_LOW_SURROGATE(ch2) ) {
						candidate.SetCodePoint(ch);
						candidate._CharDescription = L"Unmatched high surrogate ";
						candidate._CharDescription.append(candidate._CharUnicodeHex);
						
						candidate.SetCodePoint(L'?');
						continue;
					}
					else {
						++pdata;

						const uint16_t LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
						const uint16_t TRAIL_OFFSET = 0xDC00;

						codepoint = (ch - LEAD_OFFSET) << 10;
						codepoint += (ch2 & 0x3FF);
					}
				}
				if (IS_LOW_SURROGATE(ch)) {
					candidate.SetCodePoint(ch);
					candidate._CharDescription = L"Unmatched low surrogate ";
					candidate._CharDescription.append(candidate._CharUnicodeHex);
						
					candidate.SetCodePoint(L'?');
					continue;
				}

				candidate.SetCodePoint(codepoint);
				WCHAR *desc = this->_unicodeDB->findDescription(codepoint);
				if (desc)
					candidate._CharDescription = desc;
				else {
					candidate._CharDescription = L"Unicode character ";
					candidate._CharDescription.append(candidate._CharUnicodeHex);
				}

				candidateList.push_back(candidate);
			} /* while *pdata */
			
		} /* If (OpenClipboard) */

		/* (Re)initialize the candidate list */
		hr = _CreateAndStartCandidate(ec, pContext);
		if (SUCCEEDED(hr))
		{
			_pCandidateListUIPresenter->_ClearList();
			_pCandidateListUIPresenter->_SetHelpText(IME_HELP_TEXT_CLIPBOARD);
			/* Add the list to the UI */
			_pCandidateListUIPresenter->_SetText(candidateList, TRUE);
		}
	}
	return hr;
}
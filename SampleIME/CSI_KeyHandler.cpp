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
				_pCandidateListUIPresenter->_SetText(candidateList, TRUE);
			}

			this->_inputState = STATE_SEARCH;
		}
		else {
			if (_pCandidateListUIPresenter)
				_pCandidateListUIPresenter->_EndCandidateList();
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

    if (((_candidateMode == CANDIDATE_PHRASE) && (_pCandidateListUIPresenter))
        || ((_candidateMode == CANDIDATE_NONE) && (_pCandidateListUIPresenter)))
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

// TODO:
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

		// computations
		uint16_t lead = LEAD_OFFSET + (unicode_char >> 10);
		uint16_t trail = 0xDC00 + (unicode_char & 0x3FF);

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


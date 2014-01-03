// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus)
{
    pDocMgrPrevFocus;

    _InitTextEditSink(pDocMgrFocus);

    _UpdateLanguageBarOnSetFocus(pDocMgrFocus);

    //
    // We have to hide/unhide candidate list depending on whether they are 
    // associated with pDocMgrFocus.
    //
    ITfDocumentMgr* pCandidateListDocumentMgr = nullptr;
    ITfContext* pTfContext = _pCandidateListUIPresenter->_GetContextDocument();
    if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
    {
        if (pCandidateListDocumentMgr != pDocMgrFocus)
        {
				
			if (_pCandidateListUIPresenter)
			{
				_pCandidateListUIPresenter->OnKillThreadFocus();
			}
        }
        else
        {
			// if there were a composition string... I can try to restore it...
			if (this->_inputState != STATE_NORMAL) {
				CKeyHandlerEditSession* pEditSession = nullptr;
				HRESULT hr = E_FAIL;		
				_KEYSTROKE_STATE KeystrokeState;

				KeystrokeState.State = this->_inputState;
				KeystrokeState.Function = FUNCTION_SHOW_STRING;

				pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pTfContext, 0, 0, KeystrokeState);
				if (pEditSession == nullptr) {
					goto Exit;
				}

				//
				// Call CKeyHandlerEditSession::DoEditSession().
				//
				// Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
				//
				hr = pTfContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

Exit:
				;
			}

			if (_pCandidateListUIPresenter)
			{
				_pCandidateListUIPresenter->OnSetThreadFocus();
			}
        }

        pCandidateListDocumentMgr->Release();
    }

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->Release();
		_pDocMgrLastFocused = nullptr;
    }

    _pDocMgrLastFocused = pDocMgrFocus;

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->AddRef();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPushContext(_In_ ITfContext *pContext)
{
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPopContext(_In_ ITfContext *pContext)
{
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrEventSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;
    BOOL ret = FALSE;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        return ret;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_threadMgrEventSinkCookie)))
    {
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrEventSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CSampleIME::_UninitThreadMgrEventSink()
{
    ITfSource* pSource = nullptr;

    if (_threadMgrEventSinkCookie == TF_INVALID_COOKIE)
    {
        return; 
    }

    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource)))
    {
        pSource->UnadviseSink(_threadMgrEventSinkCookie);
        pSource->Release();
    }

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
}

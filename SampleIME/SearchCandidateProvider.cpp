// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "private.h"
#include "SearchCandidateProvider.h"
#include <new>
#include "SampleIME.h"
#include "TipCandidateList.h"
#include "TipCandidateString.h"

/*------------------------------------------------------------------------------

create instance of CSearchCandidateProvider : also implements ITfFunction

Enables an integrated search experience in an Input Method Editor (IME).

- GetSearchCandidates
Gets a list of conversion candidates for a given string without generating any IME-related messages or events.

- SetResult
Provides a text Service or IME with history data when a candidate is chosen by the user.

------------------------------------------------------------------------------*/
HRESULT CSearchCandidateProvider::CreateInstance(_Outptr_ ITfFnSearchCandidateProvider **ppobj, _In_ ITfTextInputProcessorEx *ptip)
{  
    if (ppobj == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppobj = nullptr;

    *ppobj = new (std::nothrow) CSearchCandidateProvider(ptip);
    if (nullptr == *ppobj)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

/*------------------------------------------------------------------------------

create instance of CSearchCandidateProvider

------------------------------------------------------------------------------*/
HRESULT CSearchCandidateProvider::CreateInstance(REFIID riid, _Outptr_ void **ppvObj, _In_ ITfTextInputProcessorEx *ptip)
{ 
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppvObj = nullptr;

    *ppvObj = new (std::nothrow) CSearchCandidateProvider(ptip);
    if (nullptr == *ppvObj)
    {
        return E_OUTOFMEMORY;
    }

    return ((CSearchCandidateProvider*)(*ppvObj))->QueryInterface(riid, ppvObj);
}

/*------------------------------------------------------------------------------

constructor of CSearchCandidateProvider

------------------------------------------------------------------------------*/
CSearchCandidateProvider::CSearchCandidateProvider(_In_ ITfTextInputProcessorEx *ptip)
{
    assert(ptip != nullptr);

    _pTip = ptip;
    _refCount = 0;
}

/*------------------------------------------------------------------------------

destructor of CSearchCandidateProvider

------------------------------------------------------------------------------*/
CSearchCandidateProvider::~CSearchCandidateProvider(void)
{  
}

/*------------------------------------------------------------------------------

query interface
(IUnknown method)

------------------------------------------------------------------------------*/
STDMETHODIMP CSearchCandidateProvider::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_POINTER;
    }
    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ITfFnSearchCandidateProvider)))
    {
        *ppvObj = (ITfFnSearchCandidateProvider*)this;
    }
    else if (IsEqualIID(riid, IID_ITfFunction))
    {
        *ppvObj = (ITfFunction*)this;
    }

    if (*ppvObj == nullptr)
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

/*------------------------------------------------------------------------------

increment reference count
(IUnknown method)

------------------------------------------------------------------------------*/
STDMETHODIMP_(ULONG) CSearchCandidateProvider::AddRef()
{
    return (ULONG)InterlockedIncrement(&_refCount);
}

/*------------------------------------------------------------------------------

decrement reference count and release object
(IUnknown method)

------------------------------------------------------------------------------*/
STDMETHODIMP_(ULONG) CSearchCandidateProvider::Release()
{
    ULONG ref = (ULONG)InterlockedDecrement(&_refCount);
    if (0 < ref)
    {
        return ref;
    }

    delete this;
    return 0;
}

STDMETHODIMP CSearchCandidateProvider::GetDisplayName(_Out_ BSTR *pbstrName)
{
    if (pbstrName == nullptr)
    {
        return E_INVALIDARG;
    }

    *pbstrName = SysAllocString(L"SearchCandidateProvider");
    return  S_OK;
}
/*
Gets a list of conversion candidates for a given string without generating any IME-related messages or events.
Syntax

C++

HRESULT GetSearchCandidates(
  [in]   BSTR bstrQuery,
  [in]   BSTR bstrApplicationId,
  [out]  ITfCandidateList **pplist
);

Parameters

bstrQuery [in]
A string that specifies the reading string that the text service attempts to convert.
bstrApplicationId [in]
App-specified string that enables a text service to optionally provide different candidates to different apps or contexts based on input history. You can pass an empty BSTR, L””, for a generic context.
pplist [out]
An ITfCandidateList that receives the requested candidate data.
*/
STDMETHODIMP CSearchCandidateProvider::GetSearchCandidates(BSTR bstrQuery, BSTR bstrApplicationID, _Outptr_result_maybenull_ ITfCandidateList **pplist)
{
	bstrApplicationID;bstrQuery;
    HRESULT hr = E_FAIL;
    *pplist = nullptr;

    if (nullptr == _pTip)
    {
        return hr;
    }

	CSampleIME *cime = dynamic_cast<CSampleIME*>(_pTip);
	std::vector<CCandidateListItem> candidateList;
    cime->GetCandidateList(bstrQuery, candidateList);

    int cCand = min(candidateList.size(), FAKECANDIDATENUMBER);
    if (0 < cCand)
    {
        hr = CTipCandidateList::CreateInstance(pplist, cCand);
		if (FAILED(hr))
		{
			return hr;
		}
        for (int iCand = 0; iCand < cCand; iCand++)
        {
            ITfCandidateString* pCandStr = nullptr;
            CTipCandidateString::CreateInstance(IID_ITfCandidateString, (void**)&pCandStr);

            ((CTipCandidateString*)pCandStr)->SetIndex(iCand);
            ((CTipCandidateString*)pCandStr)->SetString(
				candidateList[iCand].GetChar(),
				wcslen(candidateList[iCand].GetChar()));

            ((CTipCandidateList*)(*pplist))->SetCandidate(&pCandStr);
        }
    }
    hr = S_OK;

    return hr;
}

/*------------------------------------------------------------------------------

set result
(ITfFnSearchCandidateProvider method)

Provides a text Service or IME with history data when a candidate is chosen by the user.
Syntax

C++

HRESULT SetResult(
  [in]  BSTR bstrQuery,
  [in]  BSTR bstrApplicationID,
  [in]  BSTR bstrResult
);

Parameters

bstrQuery [in]
The reading string for the text service or IME to convert.
bstrApplicationID [in]
App-specified string that enables a text service or IME to optionally provide different candidates to different apps or contexts based on input history. You can pass an empty BSTR, L””, for a generic context.
bstrResult [in]
A string that represents the candidate string chosen by the user. It should be one of the candidate string values returned by the GetSearchCandidates method.
(not implemented...)

------------------------------------------------------------------------------*/
STDMETHODIMP CSearchCandidateProvider::SetResult(BSTR bstrQuery, BSTR bstrApplicationID, BSTR bstrResult)
{
    bstrQuery;bstrApplicationID;bstrResult;

    return E_NOTIMPL;
}


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SampleIME.h"
#include "TfInputProcessorProfile.h"
#include "Globals.h"
#include "Compartment.h"
#include "LanguageBar.h"
#include "RegKey.h"
#include "UnicodeDB.h"

//////////////////////////////////////////////////////////////////////
//
// CSampleIME implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _AddTextProcessorEngine
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_AddTextProcessorEngine()
{
    LANGID langid = 0;
    CLSID clsid = GUID_NULL;
    GUID guidProfile = GUID_NULL;

    // Get default profile.
    CTfInputProcessorProfile profile;

    if (FAILED(profile.CreateInstance()))
    {
        return FALSE;
    }

    if (FAILED(profile.GetCurrentLanguage(&langid)))
    {
        return FALSE;
    }

    if (FAILED(profile.GetDefaultLanguageProfile(langid, GUID_TFCAT_TIP_KEYBOARD, &clsid, &guidProfile)))
    {
        return FALSE;
    }

	// TODO: figure out what this actually does.
    LANGID langidProfile = 0;
    GUID guidLanguageProfile = GUID_NULL;
	guidLanguageProfile = this->GetLanguageProfile(&langidProfile);
    if ((langid == langidProfile) && IsEqualGUID(guidProfile, guidLanguageProfile))
    {
        return TRUE;
    }

    if (FALSE == this->SetupLanguageProfile(langid, guidProfile, _GetThreadMgr(), _GetClientId(), _IsSecureMode(), _IsComLess()))
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CSampleIME::~CSampleIME()
{
	if (_unicodeDB) {
		delete _unicodeDB;
		_unicodeDB = NULL;
	}
}

//+---------------------------------------------------------------------------
//
// SetupLanguageProfile
//
// Setup language profile for Composition Processor Engine.
// param
//     [in] LANGID langid = Specify language ID
//     [in] GUID guidLanguageProfile - Specify GUID language profile which GUID is as same as Text Service Framework language profile.
//     [in] ITfThreadMgr - pointer ITfThreadMgr.
//     [in] tfClientId - TfClientId value.
//     [in] isSecureMode - secure mode
// returns
//     If setup succeeded, returns true. Otherwise returns false.
// N.B. For reverse conversion, ITfThreadMgr is NULL, TfClientId is 0 and isSecureMode is ignored.
//+---------------------------------------------------------------------------

BOOL CSampleIME::SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode)
{
    BOOL ret = TRUE;
    if ((tfClientId == 0) && (pThreadMgr == nullptr))
    {
        ret = FALSE;
        goto Exit;
    }

    _langid = langid;
    _guidProfile = guidLanguageProfile;
    _tfClientId = tfClientId;

    SetupPreserved(pThreadMgr, tfClientId);	
	InitializeSampleIMECompartment(pThreadMgr, tfClientId);
    SetupLanguageBar(pThreadMgr, tfClientId, isSecureMode);
    SetupConfiguration();
    SetupDictionaryFile();

Exit:
    return ret;
}

/**
Convert the UNICODE_T into a CCandidateListItem.

Do the necessary MBS-WCS conversions here.
*/
HRESULT CSampleIME::_ConvertToCandidateListItem(CCandidateListItem &clitem, const UNICODE_T &unicode_item) {
	const int BUF_SIZE = 12;
	wchar_t buffer[BUF_SIZE]; // if this isn't enough, tell it to go to hell
	wchar_t *description;
	size_t rv;

	rv = MultiByteToWideChar(CP_UTF8, 0, unicode_item.data, -1, buffer, BUF_SIZE);

	if (rv == (size_t)-1) {
		return S_FALSE;
	}

	clitem._CharUnicodeHex = buffer;
	clitem._CharDescription = description = _unicodeDB->findDescription(unicode_item.data, 0, -1);

	delete [] description;
	
	return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetCandidateList
/* returns in (pCandidateList) the list of candidates based on the keystroke buffer

*/
//
//----------------------------------------------------------------------------

void CSampleIME::GetCandidateList(const std::wstring &keystrokeBuffer, _Inout_ vector<CCandidateListItem> &pCandidateList)
{
	char *buffer = NULL;
	std::set<UNICODE_T> list, fuzzy_list;
	CCandidateListItem clitem;
	
    if (! _unicodeDB) return;
	
	{
		// Convert keystroke buffer to char* type
		int required_size = WideCharToMultiByte(CP_UTF8, 0, keystrokeBuffer.c_str(), keystrokeBuffer.length(), buffer, 0, NULL, NULL);
		if (!required_size) return;

		buffer = new char[required_size];
		if (!buffer) return;

		WideCharToMultiByte(CP_UTF8, 0, keystrokeBuffer.c_str(), keystrokeBuffer.length(), buffer, required_size, NULL, NULL);
	}

	// tokenize the keystroke buffer into a list of words
	{
		vector <char *> wordlist;
		char *ctok = strtok(buffer, " ");

		while (ctok) {
			wordlist.push_back(ctok);
			ctok = strtok(NULL, " ");
		}
		_unicodeDB->findCandidates(wordlist, list, fuzzy_list);
	}
	
	// Add the items to the list.
	// Need to convert to WCHAR_T
	for (auto i = list.begin();
		i != list.end();
		i++) {
		if (_ConvertToCandidateListItem(clitem, *i) == S_OK)
			pCandidateList.push_back(clitem);
	}
	for (auto i = fuzzy_list.begin();
		i != fuzzy_list.end();
		i++) {
		if (_ConvertToCandidateListItem(clitem, *i) == S_OK)
			pCandidateList.push_back(clitem);
	}
	
	delete buffer;
}

//+---------------------------------------------------------------------------
//
// SetupPreserved
//
//----------------------------------------------------------------------------

void CSampleIME::SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{	
	size_t lenOfDesc = 0;

	ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
    if (pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK) {
        return;
    }

    TF_PRESERVEDKEY preservedKeyCompose;
    preservedKeyCompose.uVKey = 0x55;
    preservedKeyCompose.uModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;

	if (StringCchLength(Global::ImeComposeDescription, STRSAFE_MAX_CCH, &lenOfDesc) != S_OK) {
        return;
    }

	pKeystrokeMgr->PreserveKey(tfClientId, Global::SampleIMEGuidComposePreservedKey, &preservedKeyCompose, Global::ImeComposeDescription, lenOfDesc);
	pKeystrokeMgr->Release();
	this->_preservedKeys.push_back(preservedKeyCompose);

    return;
}

void CSampleIME::TeardownPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    ITfKeystrokeMgr* pKeystrokeMgr = nullptr;
	CLSID Guid = Global::SampleIMEGuidComposePreservedKey;

    if (IsEqualGUID(Guid, GUID_NULL)) {
        return;
    }

    if (FAILED(pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr))) {
        return;
    }

    for (UINT i = 0; i < _preservedKeys.size(); i++)
    {
        TF_PRESERVEDKEY pPreservedKey = _preservedKeys[i];
        pPreservedKey.uModifiers &= 0xffff;

        pKeystrokeMgr->UnpreserveKey(Guid, &pPreservedKey);
	}
	pKeystrokeMgr->Release();
}
//+---------------------------------------------------------------------------
//
// SetupConfiguration
//
//----------------------------------------------------------------------------

void CSampleIME::SetupConfiguration()
{
    _candidateWndWidth = CAND_WIDTH;

    SetDefaultCandidateTextFont();

    return;
}

//+---------------------------------------------------------------------------
//
// SetupLanguageBar
//
//----------------------------------------------------------------------------

void CSampleIME::SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode)
{
	/*
    DWORD dwEnable = 1;
    CreateLanguageBarButton(dwEnable, GUID_LBI_INPUTMODE, Global::LangbarImeModeDescription, Global::ImeModeDescription, Global::ImeModeOnIcoIndex, Global::ImeModeOffIcoIndex, &_pLanguageBar_IMEMode, isSecureMode);
    CreateLanguageBarButton(dwEnable, Global::SampleIMEGuidLangBarDoubleSingleByte, Global::LangbarDoubleSingleByteDescription, Global::DoubleSingleByteDescription, Global::DoubleSingleByteOnIcoIndex, Global::DoubleSingleByteOffIcoIndex, &_pLanguageBar_DoubleSingleByte, isSecureMode);
    CreateLanguageBarButton(dwEnable, Global::SampleIMEGuidLangBarPunctuation, Global::LangbarPunctuationDescription, Global::PunctuationDescription, Global::PunctuationOnIcoIndex, Global::PunctuationOffIcoIndex, &_pLanguageBar_Punctuation, isSecureMode);

    InitLanguageBar(_pLanguageBar_IMEMode, pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    InitLanguageBar(_pLanguageBar_DoubleSingleByte, pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
    InitLanguageBar(_pLanguageBar_Punctuation, pThreadMgr, tfClientId, Global::SampleIMEGuidCompartmentPunctuation);

    if (_pCompartmentKeyboardOpenEventSink)
    {
        _pCompartmentKeyboardOpenEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    }
    if (_pCompartmentConversionEventSink)
    {
        _pCompartmentConversionEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    }
    if (_pCompartmentDoubleSingleByteEventSink)
    {
        _pCompartmentDoubleSingleByteEventSink->_Advise(pThreadMgr, Global::SampleIMEGuidCompartmentDoubleSingleByte);
    }
    if (_pCompartmentPunctuationEventSink)
    {
        _pCompartmentPunctuationEventSink->_Advise(pThreadMgr, Global::SampleIMEGuidCompartmentPunctuation);
    }
	*/
    return;
}

//+---------------------------------------------------------------------------
//
// CreateLanguageBarButton
//
//----------------------------------------------------------------------------

void CSampleIME::CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode)
{
	dwEnable;

    if (ppLangBarItemButton)
    {
        *ppLangBarItemButton = new (std::nothrow) CLangBarItemButton(guidLangBar, pwszDescriptionValue, pwszTooltipValue, dwOnIconIndex, dwOffIconIndex, isSecureMode);
    }

    return;
}

//+---------------------------------------------------------------------------
//
// InitLanguageBar
//
//----------------------------------------------------------------------------

BOOL CSampleIME::InitLanguageBar(_In_ CLangBarItemButton *pLangBarItemButton, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    if (pLangBarItemButton)
    {
        if (pLangBarItemButton->_AddItem(pThreadMgr) == S_OK)
        {
            if (pLangBarItemButton->_RegisterCompartment(pThreadMgr, tfClientId, guidCompartment))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// SetupDictionaryFile
//
//----------------------------------------------------------------------------

BOOL CSampleIME::SetupDictionaryFile()
{	
    // Not yet registered
    // Register CFileMapping
    WCHAR wszFileName[MAX_PATH] = {'\0'};
    DWORD cchA = GetModuleFileName(Global::dllInstanceHandle, wszFileName, ARRAYSIZE(wszFileName));
	std::wstring indexPath, dataPath;

    // find the last '/'
    while (cchA--)
    {
        WCHAR wszChar = wszFileName[cchA];
        if (wszChar == '\\' || wszChar == '/')
        {
			wszFileName[cchA + 1] = 0;
            break;
        }
    }
	
	dataPath = wszFileName;
	indexPath = wszFileName;

	dataPath.append(TEXTSERVICE_DESCRIPTION_DB);
	indexPath.append(TEXTSERVICE_INDEX_FILE);

	_unicodeDB = new UnicodeDB(indexPath.c_str(), dataPath.c_str());

	return TRUE;
}

void CSampleIME::InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    PrivateCompartmentsUpdated(pThreadMgr);
}

void CSampleIME::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
/*    if (!_pCompartmentConversion)
    {
        return;
    }

    DWORD conversionMode = 0;
    if (FAILED(_pCompartmentConversion->_GetCompartmentDWORD(conversionMode)))
    {
        return;
    }

    BOOL isDouble = FALSE;
    CCompartment CompartmentDoubleSingleByte(pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentDoubleSingleByte);
    if (SUCCEEDED(CompartmentDoubleSingleByte._GetCompartmentBOOL(isDouble)))
    {
        if (!isDouble && (conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            CompartmentDoubleSingleByte._SetCompartmentBOOL(TRUE);
        }
        else if (isDouble && !(conversionMode & TF_CONVERSIONMODE_FULLSHAPE))
        {
            CompartmentDoubleSingleByte._SetCompartmentBOOL(FALSE);
        }
    }*/
}
//+---------------------------------------------------------------------------
//
// CompartmentCallback
//
//----------------------------------------------------------------------------

// static
HRESULT CSampleIME::CompartmentCallback(_In_ void *pv, REFGUID guidCompartment)
{
    CSampleIME* fakeThis = (CSampleIME*)pv;
    if (nullptr == fakeThis)
    {
        return E_INVALIDARG;
    }

    ITfThreadMgr* pThreadMgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
    if (FAILED(hr))
    {
        return E_FAIL;
    }

    if (IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentDoubleSingleByte) ||
        IsEqualGUID(guidCompartment, Global::SampleIMEGuidCompartmentPunctuation))
    {
        fakeThis->PrivateCompartmentsUpdated(pThreadMgr);
    }
    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION) ||
        IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
    {
        fakeThis->ConversionModeCompartmentUpdated(pThreadMgr);
    }
    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
    {
        fakeThis->KeyboardOpenCompartmentUpdated(pThreadMgr);
    }

    pThreadMgr->Release();
    pThreadMgr = nullptr;

    return S_OK;
}

void CSampleIME::ShowAllLanguageBarIcons()
{
    SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, FALSE);
}

void CSampleIME::HideAllLanguageBarIcons()
{
    SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, TRUE);
}

void CSampleIME::SetDefaultCandidateTextFont()
{
    // Candidate Text Font
    if (Global::defaultlFontHandle == nullptr)
    {
		WCHAR fontName[50] = {'\0'}; 
		LoadString(Global::dllInstanceHandle, IDS_DEFAULT_FONT, fontName, 50);
        Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
        if (!Global::defaultlFontHandle)
        {
			LOGFONT lf;
			SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
            // Fall back to the default GUI font on failure.
            Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, lf.lfFaceName);
        }
    }
}


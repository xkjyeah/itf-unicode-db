// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "KeyHandlerEditSession.h"
#include "SampleIMEBaseStructure.h"
#include "UnicodeDB.h"

class CLangBarItemButton;
class CCandidateListUIPresenter;
class CCompositionProcessorEngine;

const DWORD WM_CheckGlobalCompartment = WM_USER;
LRESULT CALLBACK CSampleIME_WindowProc(HWND wndHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CSampleIME : public ITfTextInputProcessorEx,
    public ITfThreadMgrEventSink,
    public ITfTextEditSink,
    public ITfKeyEventSink,
    public ITfCompositionSink,
    public ITfDisplayAttributeProvider,
    public ITfActiveLanguageProfileNotifySink,
    public ITfThreadFocusSink,
    public ITfFunctionProvider,
    public ITfFnGetPreferredTouchKeyboardLayout,
	public ITfFnConfigure
{
public:
    CSampleIME();
    ~CSampleIME();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) {
        return ActivateEx(pThreadMgr, tfClientId, 0);
    }
    // ITfTextInputProcessorEx
    STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags); // OK
    STDMETHODIMP Deactivate(); // OK

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr); // unimplemented
    STDMETHODIMP OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr); // unimplemented
	/**
		OnSetFocus -- called when the document view receives or loses the focus

		Tasks:
		- Init text edit sink: Clear out any sink, and then advise our sink
		- Update language bar
		- Hide or unhide the candidate list depending on whether they are associated with the doc
	*/
    STDMETHODIMP OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(_In_ ITfContext *pContext); // unimplemented
    STDMETHODIMP OnPopContext(_In_ ITfContext *pContext); // unimplemented

    // ITfTextEditSink
	/**
		OnEndEdit -- Receives a notification upon completion of an ITfEditSession::DoEditSession
		method that has read/write access to the context.

		*/
    STDMETHODIMP OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord);

    // ITfKeyEventSink
	/**
		OnSetFocus -- Called when a TSF text service receives or loses the keyboard focus
		
		(Does nothing)
	**/
    STDMETHODIMP OnSetFocus(BOOL fForeground);
	/**
		OnTestKeyDown -- Called to determine if a text service will handle a key down event.
	*/
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
	/**
		OnTestKeyUp -- Called when a key down event occurs.
	*/
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
	/**
		OnTestKeyUp -- Called to determine if a text service will handle a key up event.
	*/
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
	/**
		OnTestKeyUp -- Called when a key up event occurs.
	*/
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
	/**
		OnPreservedKey -- Called when a preserved key event occurs.
		*/
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten);

    // ITfCompositionSink
	/**
		OnCompositionTerminated -- 

		The ITfCompositionSink interface is implemented by a text service to receive a notification
		when a composition is terminated. This advise sink is installed by passing a pointer to this '
		interface when the composition is started with the ITfContextComposition::StartComposition method.

		- Clears the 'dummy composition'
		- Calls _EndComposition()
		- Delete the candidate list
	*/
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, _In_ ITfComposition *pComposition);

    // ITfDisplayAttributeProvider
	/**
		EnumDisplayAttributeInfo -- Obtains an enumerator that contains all display attribute info objects
		supported by the display attribute provider.

		- Returns a new CEnumDisplayAttributeInfo
	*/
    STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo **ppEnum);
	/** GetDisplayAttributeInfo -- 
		Obtains a display attribute provider object for a particular display attribute.

		- Checks if guidInfo is either of {
			SampleIMEGuidDisplayAttributeInput, 
			SampleIMEGuidDisplayAttributeConverted
			};
		- Returns a new CDisplayAttributeInfoInput
		*/
    STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guidInfo, __RPC__deref_out_opt ITfDisplayAttributeInfo **ppInfo);

    // ITfActiveLanguageProfileNotifySink
	/**
	The ITfActiveLanguageProfileNotifySink interface is implemented by an application to
	receive a notification when the active language or text service changes.

	- If activated, update compartments, update language bar
	- If not activated, delete candidate list, hide language bar icons
	*/
    STDMETHODIMP OnActivated(_In_ REFCLSID clsid, _In_ REFGUID guidProfile, _In_ BOOL isActivated);

    // ITfThreadFocusSink
	/** The ITfThreadFocusSink interface is implemented by an application or TSF text
	service to receive notifications when the thread receives or loses the UI focus. 
	
	OnSetThreadFocus -- Called when the thread receives the UI focus.
	
	- Forwards to the CandidateList, which then shows itself

	OnKillThreadFocus -- Called when the thread loses the UI focus.

	- Release the reference to last focused Document Mgr
	- Add a reference to Doc Mgr?
	- Forwards to the CandidateList, which then hides itself.
	*/
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // ITfFunctionProvider
	/**
		The ITfFunctionProvider interface is implemented by an application or text service
		to provide various function objects.

	*/
	/** GetType	-- Obtains the type identifier for the function provider.

		Returns SampleIME's CLSID */
    STDMETHODIMP GetType(__RPC__out GUID *pguid);
	/** GetDescription	-- Obtains the description of the function provider.
		
		Returns a null 
		*/
    STDMETHODIMP GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc);
	/** GetFunction	-- Obtains the specified function object.

		- Requires a function group ID = GUID_NULL. (Otherwise does nothing)
		- If riid is the ID of the Search Candidate Provider (a field of CSampleIME) give the interface of FnSearchCandidateProvider
		- Otherwise give the interface of CSampleIME.
		*/
    STDMETHODIMP GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk);

    // ITfFunction
	/** - Returns a null */
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrDisplayName);

    // ITfFnGetPreferredTouchKeyboardLayout, it is the Optimized layout feature.
	/**

	The ITfFnGetPreferredTouchKeyboardLayout interface is implemented by a text service to specify the
	use of a particular keyboard layout supported by the inbox Windows 8 touch keyboard.

	Applies only to the following languages:
		Japanese
		Korean
		Simplified Chinese
		Traditional Chinese
		*/
    STDMETHODIMP GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId);

	// ITfFnConfigure -- opens up the properties page
	STDMETHODIMP Show(_In_ HWND hwndParent, _In_ LANGID langid, _In_ REFGUID rguidProfile);

    // CClassFactory factory callback
    static HRESULT CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);

    // utility function for thread manager.
    ITfThreadMgr* _GetThreadMgr() { return _pThreadMgr; }
    TfClientId _GetClientId() { return _tfClientId; }

    // functions for the composition object.
    void _SetComposition(_In_ ITfComposition *pComposition);
    void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate = FALSE);
    void _SaveCompositionContext(_In_ ITfContext *pContext);

    // key event handlers for composition/candidate/phrase common objects.
    HRESULT _HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleInputCancel(TfEditCookie ec, _In_ ITfContext *pContext);
    HRESULT _HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext);

    // key event handlers for composition object.
    HRESULT _HandleSearchInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch);
    HRESULT _HandleSearchBackspace(TfEditCookie ec, _In_ ITfContext *pContext);
    HRESULT _HandleSearchSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode);
    HRESULT _HandleSearchArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction);

	// key event handlers for the clipboard inspector
	HRESULT _HandleClipSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ UINT uCode);
	HRESULT _HandleClipArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KEYSTROKE_FUNCTION keyFunction);
	HRESULT _HandleClipBackspace(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleClipInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch);
	HRESULT _HandleClipFinalize(TfEditCookie ec, _In_ ITfContext *pContext);
	
	HRESULT _HandleEnterClipMode(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleEnterSearchMode(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleEnterHexMode(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleHexInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch);
	HRESULT _HandleHexConvert(TfEditCookie ec, _In_ ITfContext *pContext);
	HRESULT _HandleHexBackspace(TfEditCookie ec, _In_ ITfContext *pContext);

	HRESULT _HandleRefresh(TfEditCookie ec, _In_ ITfContext *pContext);

	/* Used to reset the candidate list, selection range etc. at the start of each _HandleXX */
	bool	_ResetDecor(TfEditCookie ec, _In_ ITfContext *pContext);
	/* Used to 'finalize'? */
    HRESULT _HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL fCandidateList);

    // key event handlers for candidate object.
/*    HRESULT _HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext *pContext);
    HRESULT _HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext *pContext);
    HRESULT _HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KEYSTROKE_FUNCTION keyFunction);*/

    BOOL _IsSecureMode(void) { return (_dwActivateFlags & TF_TMAE_SECUREMODE) ? TRUE : FALSE; }
    BOOL _IsComLess(void) { return (_dwActivateFlags & TF_TMAE_COMLESS) ? TRUE : FALSE; }
    BOOL _IsStoreAppMode(void) { return (_dwActivateFlags & TF_TMF_IMMERSIVEMODE) ? TRUE : FALSE; };

    // comless helpers
    static HRESULT CSampleIME::CreateInstance(REFCLSID rclsid, REFIID riid, _Outptr_result_maybenull_ LPVOID* ppv, _Out_opt_ HINSTANCE* phInst, BOOL isComLessMode);
    static HRESULT CSampleIME::ComLessCreateInstance(REFGUID rclsid, REFIID riid, _Outptr_result_maybenull_ void **ppv, _Out_opt_ HINSTANCE *phInst);
    static HRESULT CSampleIME::GetComModuleName(REFGUID rclsid, _Out_writes_(cchPath)WCHAR* wchPath, DWORD cchPath);

	/**** CPE public functions ****/
    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode);
    // Get language profile.
    GUID GetLanguageProfile(LANGID *plangid)
    {
        *plangid = _langid;
        return _guidProfile;
    }
    // Get locale
    LCID GetLocale()
    {
        return MAKELCID(_langid, SORT_DEFAULT);
    }
	void GetCandidateList(const std::wstring &keystrokeBuffer, _Inout_ vector<CCandidateListItem> &pCandidateList);
    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet);

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

    void ShowAllLanguageBarIcons();
    void HideAllLanguageBarIcons();

    inline UINT GetCandidateWindowWidth() { return _candidateWndWidth; }

	void ActivateProcessorPart();
	void DeactivateProcessorPart();

private:
    // functions for the composition object.
    HRESULT _CreateAndStartCandidate(TfEditCookie ec, _In_ ITfContext *pContext);

    void _StartComposition(_In_ ITfContext *pContext);
    void _EndComposition(_In_opt_ ITfContext *pContext);
    BOOL _IsComposing();
    BOOL _IsKeyboardDisabled();

    HRESULT _UpdateCandidateString(TfEditCookie ec, _In_ ITfContext *pContext, _In_ const std::wstring &pstrAddString);
    HRESULT _FinalizeText(TfEditCookie ec, _In_ ITfContext *pContext, _In_ std::wstring &pstrAddString);

    BOOL _FindComposingRange(TfEditCookie ec, _In_ ITfContext *pContext, _In_ ITfRange *pSelection, _Outptr_result_maybenull_ ITfRange **ppRange);
    HRESULT _SetInputString(TfEditCookie ec, _In_ ITfContext *pContext, _Out_opt_ ITfRange *pRange, _In_ const std::wstring &pstrAddString, BOOL exist_composing);
    HRESULT _InsertAtSelection(TfEditCookie ec, _In_ ITfContext *pContext, _In_ const std::wstring &pstrAddString, _Outptr_ ITfRange **ppCompRange);

    HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition);

    // Invoke key handler edit session
    HRESULT _InvokeKeyHandler(_In_ ITfContext *pContext, UINT code, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState);

    // function for the language property
    BOOL _SetCompositionLanguage(TfEditCookie ec, _In_ ITfContext *pContext);

    // function for the display attribute
    void _ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute);
    BOOL _InitDisplayAttributeGuidAtom();

    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    BOOL _InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr);

    void _UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus);

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    BOOL _InitActiveLanguageProfileNotifySink();
    void _UninitActiveLanguageProfileNotifySink();

    BOOL _IsKeyEaten(_In_ ITfContext *pContext, UINT codeIn, _Out_ UINT *pCodeOut, _Out_writes_(1) WCHAR *pwch, _Out_opt_ _KEYSTROKE_STATE *pKeyState);

    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover);
    VOID _DeleteCandidateList(BOOL fForce, _In_opt_ ITfContext *pContext);
	VOID _ResetNormalState() { this->_keystrokeBuffer.clear(); this->_inputState = STATE_NORMAL; }

    WCHAR ConvertVKey(UINT code);

    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    BOOL _InitFunctionProviderSink();
    void _UninitFunctionProviderSink();

    BOOL _AddTextProcessorEngine();

    BOOL VerifySampleIMECLSID(_In_ REFCLSID clsid);

    friend LRESULT CALLBACK CSampleIME_WindowProc(HWND wndHandle, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**** From CompositionProcessorEngine ****/
    BOOL InitLanguageBar(_In_ CLangBarItemButton *pLanguageBar, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);
	
	void CSampleIME::_ReadSettings();

	// SetupLanguageProfile
    void SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
	void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
		void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);
    void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode);
		static HRESULT CompartmentCallback(_In_ void *pv, REFGUID guidCompartment);
			void KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);
		void CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode);
    void SetupConfiguration();
		void SetDefaultCandidateTextFont();
    BOOL SetupDictionaryFile();
		HRESULT _ConvertToCandidateListItem(CCandidateListItem &clitem, const UNICODE_T &unicode_item);

	// Teardown
	void TeardownPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

private:
	/**** From CompositionProcessorEngine ****/
    LANGID _langid;
    GUID _guidProfile;
	std::vector< std::pair<CLSID, TF_PRESERVEDKEY> > _preservedKeys;

    UINT _candidateWndWidth;

	UnicodeDB *_unicodeDB;
    static const int OUT_OF_FILE_INDEX = -1;

	UINT	_activationKeyModifiers;
	UINT	_activationKeyVKey;
	WCHAR   _searchKey;
	WCHAR   _clipboardKey;

	/**** Original CSampleIME ****/
    ITfThreadMgr* _pThreadMgr;
    TfClientId _tfClientId;
    DWORD _dwActivateFlags;

    // The cookie of ThreadMgrEventSink
    DWORD _threadMgrEventSinkCookie;

    ITfContext* _pTextEditSinkContext;
    DWORD _textEditSinkCookie;

    // The cookie of ActiveLanguageProfileNotifySink
    DWORD _activeLanguageProfileNotifySinkCookie;

    // The cookie of ThreadFocusSink
    DWORD _dwThreadFocusSinkCookie;

    // Language bar item object.
    CLangBarItemButton* _pLangBarItem;

    // the current composition object.
    ITfComposition* _pComposition;

    // guidatom for the display attibute.
    TfGuidAtom _gaDisplayAttributeInput;
    TfGuidAtom _gaDisplayAttributeConverted;

    CANDIDATE_MODE _candidateMode;
    CCandidateListUIPresenter *_pCandidateListUIPresenter;
    BOOL _isCandidateWithWildcard : 1;

    ITfDocumentMgr* _pDocMgrLastFocused;

    ITfContext* _pContext;

    ITfCompartment* _pSIPIMEOnOffCompartment;
    DWORD _dwSIPIMEOnOffCompartmentSinkCookie;

    HWND _msgWndHandle; 

    LONG _refCount;

    // Support the search integration
    ITfFnSearchCandidateProvider* _pITfFnSearchCandidateProvider;

	// State
	InputStates		_inputState;

	std::wstring	_keystrokeBuffer;
	std::vector<CCandidateListItem> _clipCandidateList;
	static const size_t LENGTH_HEX_PREFIX = 1;		// length of "u"
	static const size_t LENGTH_SEARCH_PREFIX = 2;	// length of "u'"
};

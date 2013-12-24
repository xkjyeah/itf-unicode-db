// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "private.h"
#include "BaseWindow.h"
#include "ShadowWindow.h"
#include "ScrollBarWindow.h"
#include "SampleIMEBaseStructure.h"

enum CANDWND_ACTION
{
    CAND_ITEM_SELECT
};

typedef HRESULT (*CANDWNDCALLBACK)(void *pv, enum CANDWND_ACTION action);

class CCandidateWindow : public CBaseWindow
{
public:
    CCandidateWindow(_In_ CANDWNDCALLBACK pfnCallback, _In_ void *pv, _In_ int numCandidatesPerPage, _In_ BOOL isStoreAppMode);
    virtual ~CCandidateWindow();

    BOOL _Create(ATOM atom, _In_ UINT wndWidth, _In_opt_ HWND parentWndHandle);

    void _Move(int x, int y);
    void _Show(BOOL isShowWnd);

    VOID _SetTextColor(_In_ COLORREF crColor, _In_ COLORREF crBkColor);
    VOID _SetFillColor(_In_ HBRUSH hBrush);

    LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
    void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps);
    void _OnLButtonDown(POINT pt);
    void _OnLButtonUp(POINT pt);
    void _OnMouseMove(POINT pt);
    void _OnVScroll(DWORD dwSB, _In_ DWORD nPos);

    void _AddString(_Inout_ const CCandidateListItem &pCandidateItem);
    void _ClearList();
    UINT _GetCount()
    {
        return _candidateList.size();
    }
    UINT _GetSelection()
    {
        return _currentSelection;
    }
    void _SetScrollInfo(_In_ int nMax, _In_ int nPage);

    DWORD _GetCandidateString(_In_ int iIndex, _Outptr_result_maybenull_z_ const WCHAR **ppwchCandidateString);
    DWORD _GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString);

    BOOL _MoveSelection(_In_ int offSet, _In_ BOOL isNotify);
    BOOL _SetSelection(_In_ int iPage, _In_ BOOL isNotify);
    void _SetSelection(_In_ int nIndex);
    BOOL _MovePage(_In_ int offSet, _In_ BOOL isNotify);
    BOOL _SetSelectionInPage(int nPos);

private:
    void _HandleMouseMsg(_In_ UINT mouseMsg, _In_ POINT point);
    void _DrawList(_In_ HDC dcHandle, _In_ UINT iIndex, _In_ RECT *prc);
    void _DrawBorder(_In_ HWND wndHandle, _In_ int cx);
    BOOL _SetSelectionOffset(_In_ int offSet);
    HRESULT _CurrentPageHasEmptyItems(_Inout_ BOOL *pfHasEmptyItems);

	// LightDismiss feature support, it will fire messages lightdismiss-related to the light dismiss layout.
    void _FireMessageToLightDismiss(_In_ HWND wndHandle, _In_ WINDOWPOS *pWndPos);

    BOOL _CreateMainWindow(ATOM atom, _In_opt_ HWND parentWndHandle);
    BOOL _CreateBackGroundShadowWindow();
    BOOL _CreateVScrollWindow();

    HRESULT _AdjustPageIndex(_Inout_ UINT & currentPage, _Inout_ UINT & currentPageIndex);

    void _ResizeWindow();
    void _DeleteShadowWnd();

    friend COLORREF _AdjustTextColor(_In_ COLORREF crColor, _In_ COLORREF crBkColor);

	inline int _PageNumberToCandidateIndex( int iPage ) {
		return this->_numCandidatesPerPage * iPage;
	}
	inline int _CandidateIndexToPageNumber( int iIndex ) {
		return iIndex % this->_numCandidatesPerPage;
	}
	inline int _PageCount() {
		return
			(this->_candidateList.size() / this->_numCandidatesPerPage) + // number of full pages
			((this->_candidateList.size() % this->_numCandidatesPerPage)?1:0); // the last page
	}

private:
    UINT _currentSelection;
    std::vector<CCandidateListItem> _candidateList;
	int _numCandidatesPerPage;

    COLORREF _crTextColor;
    COLORREF _crBkColor;
    HBRUSH _brshBkColor;

    TEXTMETRIC _TextMetric;
    int _cyRow;
    int _cxTitle;
    UINT _wndWidth;

    CANDWNDCALLBACK _pfnCallback;
    void* _pObj;

    CShadowWindow* _pShadowWnd;

    BOOL _dontAdjustOnEmptyItemPage;
    BOOL _isStoreAppMode;
};

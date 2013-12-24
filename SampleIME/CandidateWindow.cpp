// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "BaseWindow.h"
#include "CandidateWindow.h"
#include "CandidateListUIPresenter.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCandidateWindow::CCandidateWindow(_In_ CANDWNDCALLBACK pfnCallback, _In_ void *pv, _In_ int numCandidatesPerPage, _In_ BOOL isStoreAppMode)
{
    _currentSelection = 0;
	this->_numCandidatesPerPage = numCandidatesPerPage;

    _SetTextColor(CANDWND_ITEM_COLOR, GetSysColor(COLOR_WINDOW));    // text color is black
    _SetFillColor((HBRUSH)(COLOR_WINDOW+1));

    _pfnCallback = pfnCallback;
    _pObj = pv;

    _pShadowWnd = nullptr;

    _cyRow = CANDWND_ROW_WIDTH;
    _cxTitle = 0;

    _wndWidth = 0;

    _dontAdjustOnEmptyItemPage = FALSE;

    _isStoreAppMode = isStoreAppMode;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCandidateWindow::~CCandidateWindow()
{
    _ClearList();
    _DeleteShadowWnd();
}

//+---------------------------------------------------------------------------
//
// _Create
//
// CandidateWinow is the top window
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_Create(ATOM atom, _In_ UINT wndWidth, _In_opt_ HWND parentWndHandle)
{
    BOOL ret = FALSE;
    _wndWidth = wndWidth;

    ret = _CreateMainWindow(atom, parentWndHandle);
    if (FALSE == ret)
    {
        goto Exit;
    }

    ret = _CreateBackGroundShadowWindow();
    if (FALSE == ret)
    {
        goto Exit;
    }

    _ResizeWindow();

Exit:
    return TRUE;
}

BOOL CCandidateWindow::_CreateMainWindow(ATOM atom, _In_opt_ HWND parentWndHandle)
{
    _SetUIWnd(this);

    if (!CBaseWindow::_Create(atom,
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        WS_BORDER | WS_POPUP,
        NULL, 0, 0, parentWndHandle))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CCandidateWindow::_CreateBackGroundShadowWindow()
{
    _pShadowWnd = new (std::nothrow) CShadowWindow(this);
    if (_pShadowWnd == nullptr)
    {
        return FALSE;
    }

    if (!_pShadowWnd->_Create(Global::AtomShadowWindow,
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        WS_DISABLED | WS_POPUP, this))
    {
        _DeleteShadowWnd();
        return FALSE;
    }

    return TRUE;
}

void CCandidateWindow::_ResizeWindow()
{
    SIZE size = {0, 0};

    _cxTitle = max(_cxTitle, size.cx + 2 * GetSystemMetrics(SM_CXFRAME));

    CBaseWindow::_Resize(0, 0, _cxTitle, _cyRow * this->_numCandidatesPerPage);

    RECT rcCandRect = {0, 0, 0, 0};
    _GetClientRect(&rcCandRect);

    int letf = rcCandRect.right - GetSystemMetrics(SM_CXVSCROLL) * 2 - CANDWND_BORDER_WIDTH;
    int top = rcCandRect.top + CANDWND_BORDER_WIDTH;
    int width = GetSystemMetrics(SM_CXVSCROLL) * 2;
    int height = rcCandRect.bottom - rcCandRect.top - CANDWND_BORDER_WIDTH * 2;
}

//+---------------------------------------------------------------------------
//
// _Move
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Move(int x, int y)
{
    CBaseWindow::_Move(x, y);
}

//+---------------------------------------------------------------------------
//
// _Show
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Show(BOOL isShowWnd)
{
    if (_pShadowWnd)
    {
        _pShadowWnd->_Show(isShowWnd);
    }
    CBaseWindow::_Show(isShowWnd);
}

//+---------------------------------------------------------------------------
//
// _SetTextColor
// _SetFillColor
//
//----------------------------------------------------------------------------

VOID CCandidateWindow::_SetTextColor(_In_ COLORREF crColor, _In_ COLORREF crBkColor)
{
    _crTextColor = _AdjustTextColor(crColor, crBkColor);
    _crBkColor = crBkColor;
}

VOID CCandidateWindow::_SetFillColor(_In_ HBRUSH hBrush)
{
    _brshBkColor = hBrush;
}

//+---------------------------------------------------------------------------
//
// _WindowProcCallback
//
// Cand window proc.
//----------------------------------------------------------------------------

const int PageCountPosition = 1;
const int StringPosition = 4;

LRESULT CALLBACK CCandidateWindow::_WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            HDC dcHandle = nullptr;

            dcHandle = GetDC(wndHandle);
            if (dcHandle)
            {
                HFONT hFontOld = (HFONT)SelectObject(dcHandle, Global::defaultlFontHandle);
                GetTextMetrics(dcHandle, &_TextMetric);

                _cxTitle = _TextMetric.tmMaxCharWidth * _wndWidth;
                SelectObject(dcHandle, hFontOld);
                ReleaseDC(wndHandle, dcHandle);
            }
        }
        return 0;

    case WM_DESTROY:
        _DeleteShadowWnd();
        return 0;

    case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS* pWndPos = (WINDOWPOS*)lParam;

            // move shadow
            if (_pShadowWnd)
            {
                _pShadowWnd->_OnOwnerWndMoved((pWndPos->flags & SWP_NOSIZE) == 0);
            }

            _FireMessageToLightDismiss(wndHandle, pWndPos);
        }
        break;

    case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS* pWndPos = (WINDOWPOS*)lParam;

            // show/hide shadow
            if (_pShadowWnd)
            {
                if ((pWndPos->flags & SWP_HIDEWINDOW) != 0)
                {
                    _pShadowWnd->_Show(FALSE);
                }

                // don't go behaind of shadow
                if (((pWndPos->flags & SWP_NOZORDER) == 0) && (pWndPos->hwndInsertAfter == _pShadowWnd->_GetWnd()))
                {
                    pWndPos->flags |= SWP_NOZORDER;
                }

                _pShadowWnd->_OnOwnerWndMoved((pWndPos->flags & SWP_NOSIZE) == 0);
            }

        }
        break;

    case WM_SHOWWINDOW:
        // show/hide shadow
        if (_pShadowWnd)
        {
            _pShadowWnd->_Show((BOOL)wParam);
        }

        break;

    case WM_PAINT:
        {
            HDC dcHandle = nullptr;
            PAINTSTRUCT ps;

            dcHandle = BeginPaint(wndHandle, &ps);
            _OnPaint(dcHandle, &ps);
            _DrawBorder(wndHandle, CANDWND_BORDER_WIDTH*2);
            EndPaint(wndHandle, &ps);
        }
        return 0;

    case WM_SETCURSOR:
        {
            POINT cursorPoint;

            GetCursorPos(&cursorPoint);
            MapWindowPoints(NULL, wndHandle, &cursorPoint, 1);

            // handle mouse message
            _HandleMouseMsg(HIWORD(lParam), cursorPoint);
        }
        return 1;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        {
            POINT point;

            POINTSTOPOINT(point, MAKEPOINTS(lParam));

            // handle mouse message
            _HandleMouseMsg(uMsg, point);
        }
		// we processes this message, it should return zero. 
        return 0;

    case WM_MOUSEACTIVATE:
        {
            WORD mouseEvent = HIWORD(lParam);
            if (mouseEvent == WM_LBUTTONDOWN || 
                mouseEvent == WM_RBUTTONDOWN || 
                mouseEvent == WM_MBUTTONDOWN) 
            {
                return MA_NOACTIVATE;
            }
        }
        break;

    case WM_POINTERACTIVATE:
        return PA_NOACTIVATE;

    case WM_VSCROLL:
        _OnVScroll(LOWORD(wParam), HIWORD(wParam));
        return 0;
    }

    return DefWindowProc(wndHandle, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _HandleMouseMsg
//
//----------------------------------------------------------------------------

void CCandidateWindow::_HandleMouseMsg(_In_ UINT mouseMsg, _In_ POINT point)
{
    switch (mouseMsg)
    {
    case WM_MOUSEMOVE:
        _OnMouseMove(point);
        break;
    case WM_LBUTTONDOWN:
        _OnLButtonDown(point);
        break;
    case WM_LBUTTONUP:
        _OnLButtonUp(point);
        break;
    }
}

//+---------------------------------------------------------------------------
//
// _OnPaint
//
//----------------------------------------------------------------------------

void CCandidateWindow::_OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pPaintStruct)
{
    SetBkMode(dcHandle, TRANSPARENT);

    HFONT hFontOld = (HFONT)SelectObject(dcHandle, Global::defaultlFontHandle);

    FillRect(dcHandle, &pPaintStruct->rcPaint, _brshBkColor);

    UINT currentPageIndex = 0;
    UINT currentPage = _CandidateIndexToPageNumber(_currentSelection);

    _DrawList(dcHandle, currentPageIndex, &pPaintStruct->rcPaint);

    SelectObject(dcHandle, hFontOld);
}

//+---------------------------------------------------------------------------
//
// _OnLButtonDown
//
//----------------------------------------------------------------------------

void CCandidateWindow::_OnLButtonDown(POINT pt)
{
    RECT rcWindow = {0, 0, 0, 0};;
    _GetClientRect(&rcWindow);

    int cyLine = _cyRow;
    
    UINT index = 0;
    int currentPage = _CandidateIndexToPageNumber(_currentSelection);

    // Hit test on list items
    index =  _PageNumberToCandidateIndex(currentPage);

    for (UINT pageCount = 0; (index < _candidateList.size()) && (pageCount < _numCandidatesPerPage); index++, pageCount++)
    {
        RECT rc = {0, 0, 0, 0};

        rc.left = rcWindow.left;
        rc.right = rcWindow.right - GetSystemMetrics(SM_CXVSCROLL) * 2;
        rc.top = rcWindow.top + (pageCount * cyLine);
        rc.bottom = rcWindow.top + ((pageCount + 1) * cyLine);

        if (PtInRect(&rc, pt) && _pfnCallback)
        {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            _currentSelection = index;
            _pfnCallback(_pObj, CAND_ITEM_SELECT);
            return;
        }
    }
	
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

//+---------------------------------------------------------------------------
//
// _OnLButtonUp
//
//----------------------------------------------------------------------------

void CCandidateWindow::_OnLButtonUp(POINT pt)
{
}

//+---------------------------------------------------------------------------
//
// _OnMouseMove
//
//----------------------------------------------------------------------------

void CCandidateWindow::_OnMouseMove(POINT pt)
{
    RECT rcWindow = {0, 0, 0, 0};

    _GetClientRect(&rcWindow);

    RECT rc = {0, 0, 0, 0};

    rc.left   = rcWindow.left;
    rc.right  = rcWindow.right - GetSystemMetrics(SM_CXVSCROLL) * 2;

    rc.top    = rcWindow.top;
    rc.bottom = rcWindow.bottom;

    if (PtInRect(&rc, pt))
    {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return;
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

//+---------------------------------------------------------------------------
//
// _OnVScroll
//
//----------------------------------------------------------------------------

void CCandidateWindow::_OnVScroll(DWORD dwSB, _In_ DWORD nPos)
{
    switch (dwSB)
    {
    case SB_LINEDOWN:
        _SetSelectionOffset(+1);
        _InvalidateRect();
        break;
    case SB_LINEUP:
        _SetSelectionOffset(-1);
        _InvalidateRect();
        break;
    case SB_PAGEDOWN:
        _MovePage(+1, FALSE);
        _InvalidateRect();
        break;
    case SB_PAGEUP:
        _MovePage(-1, FALSE);
        _InvalidateRect();
        break;
    case SB_THUMBPOSITION:
        _SetSelection(nPos, FALSE);
        _InvalidateRect();
        break;
    }
}

//+---------------------------------------------------------------------------
//
// _DrawList

// TODO: Need to measure and possibly resize the window
//
//----------------------------------------------------------------------------

void CCandidateWindow::_DrawList(_In_ HDC dcHandle, _In_ UINT iIndex, _In_ RECT *prc)
{
    int pageOffset = 0;

    int cxLine = _TextMetric.tmAveCharWidth;
    int cyLine = max(_cyRow, _TextMetric.tmHeight);
    int cyOffset = (cyLine == _cyRow ? (cyLine-_TextMetric.tmHeight)/2 : 0);

    RECT rc;

	const size_t lenOfPageCount = 16;
    for (;
        (iIndex < _candidateList.size()) && (pageOffset < _numCandidatesPerPage);
        iIndex++, pageOffset++)
    {
		/* Paint four things:
		1. The selection number
		2. The character itself
		3. Character's unicode code in hex
		4. Description of the character 
		*/
        WCHAR pageCountString[lenOfPageCount] = {'\0'};
        CCandidateListItem* pItemList = nullptr;
		std::wstring sDescription;

        rc.top = prc->top + pageOffset * cyLine;
        rc.bottom = rc.top + cyLine;

        rc.left = prc->left + PageCountPosition * cxLine;
        rc.right = prc->left + StringPosition * cxLine;

        // Number Font Color And BK
        SetTextColor(dcHandle, CANDWND_NUM_COLOR);
        SetBkColor(dcHandle, GetSysColor(COLOR_3DHIGHLIGHT));

        StringCchPrintf(pageCountString, ARRAYSIZE(pageCountString), L"%d", CCandidateListUIPresenter::ArrayIndexToKey(pageOffset) );
        ExtTextOut(dcHandle, PageCountPosition * cxLine, pageOffset * cyLine + cyOffset, ETO_OPAQUE, &rc, pageCountString, lenOfPageCount, NULL);

        rc.left = prc->left + StringPosition * cxLine;
        rc.right = prc->right;

        // Candidate Font Color And BK
        if (_currentSelection != iIndex)
        {
            SetTextColor(dcHandle, _crTextColor);
            SetBkColor(dcHandle, GetSysColor(COLOR_3DHIGHLIGHT));
        }
        else
        {
            SetTextColor(dcHandle, CANDWND_SELECTED_ITEM_COLOR);
            SetBkColor(dcHandle, CANDWND_SELECTED_BK_COLOR);
        }

		sDescription.append( _candidateList[iIndex].GetChar() );
		sDescription.append( L" -- " );
		sDescription.append( _candidateList[iIndex]._CharDescription );

        ExtTextOut(
				dcHandle,
				StringPosition * cxLine, 
				pageOffset * cyLine + cyOffset, 
				ETO_OPAQUE, 
				&rc, 
				sDescription.c_str(),
				(DWORD)sDescription.length(), NULL);
    }
    for (; (pageOffset < _numCandidatesPerPage); pageOffset++)
    {
        rc.top    = prc->top + pageOffset * cyLine;
        rc.bottom = rc.top + cyLine;

        rc.left   = prc->left + PageCountPosition * cxLine;
        rc.right  = prc->left + StringPosition * cxLine;

        FillRect(dcHandle, &rc, (HBRUSH)(COLOR_3DHIGHLIGHT+1));
    }
}

//+---------------------------------------------------------------------------
//
// _DrawBorder
//
//----------------------------------------------------------------------------
void CCandidateWindow::_DrawBorder(_In_ HWND wndHandle, _In_ int cx)
{
    RECT rcWnd;

    HDC dcHandle = GetWindowDC(wndHandle);

    GetWindowRect(wndHandle, &rcWnd);
    // zero based
    OffsetRect(&rcWnd, -rcWnd.left, -rcWnd.top); 

    HPEN hPen = CreatePen(PS_DOT, cx, CANDWND_BORDER_COLOR);
    HPEN hPenOld = (HPEN)SelectObject(dcHandle, hPen);
    HBRUSH hBorderBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hBorderBrushOld = (HBRUSH)SelectObject(dcHandle, hBorderBrush);

    Rectangle(dcHandle, rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

    SelectObject(dcHandle, hPenOld);
    SelectObject(dcHandle, hBorderBrushOld);
    DeleteObject(hPen);
    DeleteObject(hBorderBrush);
    ReleaseDC(wndHandle, dcHandle);

}

//+---------------------------------------------------------------------------
//
// _AddString
//
//----------------------------------------------------------------------------

void CCandidateWindow::_AddString(_Inout_ const CCandidateListItem &pCandidateItem)
{
	this->_candidateList.push_back(pCandidateItem); /* use the copy constructor :) */
    
    return;
}

//+---------------------------------------------------------------------------
//
// _ClearList
//
//----------------------------------------------------------------------------

void CCandidateWindow::_ClearList()
{
	this->_candidateList.clear();
	this->_currentSelection = 0;
}

//+---------------------------------------------------------------------------
//
// _SetScrollInfo
//
//----------------------------------------------------------------------------

void CCandidateWindow::_SetScrollInfo(_In_ int nMax, _In_ int nPage)
{
    CScrollInfo si;
    si.nMax = nMax;
    si.nPage = nPage;
    si.nPos = 0;

}

//+---------------------------------------------------------------------------
//
// _GetCandidateString
//
//----------------------------------------------------------------------------

DWORD CCandidateWindow::_GetCandidateString(_In_ int iIndex, _Outptr_result_maybenull_z_ const WCHAR **ppwchCandidateString)
{
    CCandidateListItem* pItemList = nullptr;

    if (iIndex < 0 )
    {
        *ppwchCandidateString = nullptr;
        return 0;
    }

    UINT index = static_cast<UINT>(iIndex);
	
	if (index >= _candidateList.size())
    {
        *ppwchCandidateString = nullptr;
        return 0;
    }

    pItemList = &_candidateList[iIndex];
    if (ppwchCandidateString)
    {
		*ppwchCandidateString = pItemList->GetChar();
    }
    return (DWORD)wcslen(*ppwchCandidateString);
}

//+---------------------------------------------------------------------------
//
// _GetSelectedCandidateString
//
//----------------------------------------------------------------------------

DWORD CCandidateWindow::_GetSelectedCandidateString(_Outptr_result_maybenull_ const WCHAR **ppwchCandidateString)
{
    CCandidateListItem* pItemList = nullptr;

    if (_currentSelection >= _candidateList.size())
    {
        *ppwchCandidateString = nullptr;
        return 0;
    }

    pItemList = &_candidateList[_currentSelection];
    if (ppwchCandidateString)
    {
		*ppwchCandidateString = pItemList->GetChar();
    }
    return (DWORD)wcslen(*ppwchCandidateString);
}

//+---------------------------------------------------------------------------
//
// _SetSelectionInPage
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_SetSelectionInPage(int nPos)
{	
    if (nPos < 0)
    {
        return FALSE;
    }

    UINT pos = static_cast<UINT>(nPos);

    if (pos >= _candidateList.size())
    {
        return FALSE;
    }

    int currentPage = 0;

    _currentSelection = _PageNumberToCandidateIndex(currentPage) + nPos;

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _MoveSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_MoveSelection(_In_ int offSet, _In_ BOOL isNotify)
{
    if (_currentSelection + offSet >= _candidateList.size())
    {
        return FALSE;
    }

    _currentSelection += offSet;

    _dontAdjustOnEmptyItemPage = TRUE;

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _SetSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_SetSelection(_In_ int selectedIndex, _In_ BOOL isNotify)
{
    if (selectedIndex == -1)
    {
        selectedIndex = _candidateList.size() - 1;
    }

    if (selectedIndex < 0)
    {
        return FALSE;
    }

    int candCnt = static_cast<int>(_candidateList.size());
    if (selectedIndex >= candCnt)
    {
        return FALSE;
    }

    _currentSelection = static_cast<UINT>(selectedIndex);

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _SetSelection
//
//----------------------------------------------------------------------------
void CCandidateWindow::_SetSelection(_In_ int nIndex)
{
    _currentSelection = nIndex;
}

//+---------------------------------------------------------------------------
//
// _MovePage
//
// Move to the next/previous page.
// Set selection to the same offset on the next/previous page.
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_MovePage(_In_ int offSet, _In_ BOOL isNotify)
{
    if (offSet == 0)
    {
        return TRUE;
    }

    int currentPage = _CandidateIndexToPageNumber(_currentSelection);
	int pageOffset = _PageNumberToCandidateIndex(currentPage);
    int selectionOffset = 0;
    int newPage = 0;

    newPage = currentPage + offSet;
    if ((newPage < 0) || (newPage >= _PageCount()))
    {
        return FALSE;
    }

    // If current selection is at the top of the page AND 
    // we are on the "default" page border, then we don't
    // want adjustment to eliminate empty entries.
    //
    // We do this for keeping behavior inline with downlevel.
    if (_currentSelection % _numCandidatesPerPage == 0 && 
        _currentSelection == pageOffset) 
    {
        _dontAdjustOnEmptyItemPage = TRUE;
    }

    selectionOffset = _currentSelection - pageOffset;
    _currentSelection = _PageNumberToCandidateIndex(newPage) + selectionOffset;
    _currentSelection = _candidateList.size() > _currentSelection ? _currentSelection : _candidateList.size() - 1;

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _SetSelectionOffset
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_SetSelectionOffset(_In_ int offSet)
{
	int newOffset = _currentSelection + offSet;

	if (newOffset < 0 || newOffset >= _candidateList.size())
    {
        return FALSE;
    }

    _currentSelection = newOffset;

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _AdjustTextColor
//
//----------------------------------------------------------------------------

COLORREF _AdjustTextColor(_In_ COLORREF crColor, _In_ COLORREF crBkColor)
{
    if (!Global::IsTooSimilar(crColor, crBkColor))
    {
        return crColor;
    }
    else
    {
        return crColor ^ RGB(255, 255, 255);
    }
}

//+---------------------------------------------------------------------------
//
// _CurrentPageHasEmptyItems
//
//----------------------------------------------------------------------------

HRESULT CCandidateWindow::_CurrentPageHasEmptyItems(_Inout_ BOOL *hasEmptyItems)
{
    *hasEmptyItems = ( _CandidateIndexToPageNumber(_currentSelection) < _PageCount() - 1 ) ? FALSE :
		( _candidateList.size() % _numCandidatesPerPage != 0 );
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _FireMessageToLightDismiss
//      fire EVENT_OBJECT_IME_xxx to let LightDismiss know about IME window.
//----------------------------------------------------------------------------

void CCandidateWindow::_FireMessageToLightDismiss(_In_ HWND wndHandle, _In_ WINDOWPOS *pWndPos)
{
    if (nullptr == pWndPos)
    {
        return;
    }

    BOOL isShowWnd = ((pWndPos->flags & SWP_SHOWWINDOW) != 0);
    BOOL isHide = ((pWndPos->flags & SWP_HIDEWINDOW) != 0);
    BOOL needResize = ((pWndPos->flags & SWP_NOSIZE) == 0);
    BOOL needMove = ((pWndPos->flags & SWP_NOMOVE) == 0);
    BOOL needRedraw = ((pWndPos->flags & SWP_NOREDRAW) == 0);

    if (isShowWnd)
    {
        NotifyWinEvent(EVENT_OBJECT_IME_SHOW, wndHandle, OBJID_CLIENT, CHILDID_SELF);
    }
    else if (isHide)
    {
        NotifyWinEvent(EVENT_OBJECT_IME_HIDE, wndHandle, OBJID_CLIENT, CHILDID_SELF);
    }
    else if (needResize || needMove || needRedraw)
    {
        if (IsWindowVisible(wndHandle))
        {
            NotifyWinEvent(EVENT_OBJECT_IME_CHANGE, wndHandle, OBJID_CLIENT, CHILDID_SELF);
        }
    }

}

void CCandidateWindow::_DeleteShadowWnd()
{
    if (nullptr != _pShadowWnd)
    {
        delete _pShadowWnd;
        _pShadowWnd = nullptr;
    }
}


HRESULT CCandidateWindow::_GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt) {
	HRESULT hr = S_OK;

	if (uSize > _PageCount()) {
		uSize = _PageCount();
	}
	else {
		hr = S_FALSE;
	}

	if (pIndex) {
		for (UINT i = 0; i < uSize; i++) {
			*pIndex = _PageNumberToCandidateIndex(i);
			pIndex++;
		}
	}

	*puPageCnt = _PageCount();

	return hr;
}
HRESULT CCandidateWindow::_GetCurrentPage(UINT *puPage) {
	*puPage = _CandidateIndexToPageNumber(_currentSelection);
	return S_OK;
}

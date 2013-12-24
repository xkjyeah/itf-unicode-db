// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "KeyHandlerEditSession.h"
#include "EditSession.h"
#include "SampleIME.h"

//////////////////////////////////////////////////////////////////////
//
//    ITfEditSession
//        CEditSessionBase
// CKeyHandlerEditSession class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// CKeyHandlerEditSession::DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CKeyHandlerEditSession::DoEditSession(TfEditCookie ec)
{
    HRESULT hResult = S_OK;

	KeyHandlerEditSessionDTO dto(ec, _pContext, _uCode,_wch, _KeyState.Function);

	switch (this->_KeyState.State) {
	case STATE_NORMAL:
		switch (this->_KeyState.Function) {
		case FUNCTION_HEX_MODE:
			hResult = this->_pTextService->_HandleEnterHexMode(dto.ec, dto.pContext);
			break;
		default:
			break;
		}
        break;

	case STATE_AMBIGUOUS:
		switch (this->_KeyState.Function) {
		case FUNCTION_INPUT:
			/* i.e. we've received some characters, which may or may not be hexadecimal */
			hResult = this->_pTextService->_HandleHexInput(dto.ec, dto.pContext, dto.wch);
			break;
		case FUNCTION_SEARCH_MODE:
			hResult = this->_pTextService->_HandleEnterSearchMode(dto.ec, dto.pContext);
			break;
		case FUNCTION_BACKSPACE:
			hResult = this->_pTextService->_HandleHexBackspace(dto.ec, dto.pContext);
			break;
		case FUNCTION_CANCEL:
			hResult = this->_pTextService->_HandleInputCancel(dto.ec, dto.pContext);
			break;
		default:
			break;
		}
		break;

    case STATE_HEX:
		switch (this->_KeyState.Function) {
		case FUNCTION_INPUT:
			/* i.e. we've received some characters, which may or may not be hexadecimal */
			hResult = this->_pTextService->_HandleHexInput(dto.ec, dto.pContext, dto.wch);
			break;
		case FUNCTION_SEARCH_MODE:
			hResult = this->_pTextService->_HandleEnterSearchMode(dto.ec, dto.pContext);
			break;
		case FUNCTION_BACKSPACE:
			hResult = this->_pTextService->_HandleHexBackspace(dto.ec, dto.pContext);
			break;
		case FUNCTION_CANCEL:
			hResult = this->_pTextService->_HandleInputCancel(dto.ec, dto.pContext);
			break;
		case FUNCTION_CONVERT:
			hResult = this->_pTextService->_HandleHexConvert(dto.ec, dto.pContext);
			break;
		default:
			break;
		}
        break;

	case STATE_SEARCH:
		switch (this->_KeyState.Function) {
		case FUNCTION_INPUT:
			hResult = this->_pTextService->_HandleSearchInput(dto.ec, dto.pContext, dto.wch);
			break;
		case FUNCTION_BACKSPACE:
			hResult = this->_pTextService->_HandleSearchBackspace(dto.ec, dto.pContext);
			break;
		case FUNCTION_HEX_MODE:
			hResult = this->_pTextService->_HandleEnterHexMode(dto.ec, dto.pContext);
			break;
		case FUNCTION_CANCEL:
			hResult = this->_pTextService->_HandleInputCancel(dto.ec, dto.pContext);
			break;
		case FUNCTION_SELECT_BY_NUMBER:
			hResult = _pTextService->_HandleSearchSelectByNumber(dto.ec, dto.pContext, dto.code);
			break;
		case FUNCTION_MOVE_LEFT:
		case FUNCTION_MOVE_RIGHT:
		case FUNCTION_MOVE_UP:
		case FUNCTION_MOVE_DOWN:
		case FUNCTION_MOVE_PAGE_UP:			// Page up
		case FUNCTION_MOVE_PAGE_DOWN:		// Page down
		case FUNCTION_MOVE_PAGE_TOP:		// Home
		case FUNCTION_MOVE_PAGE_BOTTOM:		// End
			hResult = this->_pTextService->_HandleSearchArrowKey(dto.ec, dto.pContext, dto.arrowKey);
			break;
		default:
			break;
		}

		break;
    default:
        break;
	}

    return hResult;
}

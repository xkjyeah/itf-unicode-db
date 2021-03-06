﻿// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "resource.h"

#define TEXTSERVICE_MODEL        L"Apartment"
// Refer to Globals::TextServiceLangIds
//#define TEXTSERVICE_LANGID       MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define TEXTSERVICE_ICON_INDEX   -IDIS_SAMPLEIME
#define TEXTSERVICE_DESCRIPTION_DB	L"UnicodeData.txt"
#define TEXTSERVICE_INDEX_FILE		L"UnicodeDataIndex.txt"
#define TEXTSERVICE_SETTINGS_APP	L"UnicodeDbIMESettings.exe"

// Important: Must correspond to the same in UnicodeDbImeSettings
#define IME_REG_SUBKEY				L"Software\\UnicodeDbIME"
#define IME_REG_ACTIVATION_SEQUENCE_MODIFIERS L"ActivationSequenceModifiers"
#define IME_REG_ACTIVATION_SEQUENCE_VKEY L"ActivationSequenceVKey"
#define IME_REG_SEARCH_KEY			L"SearchKey"
#define IME_REG_CLIPBOARD_KEY		L"ClipboardKey"

// Help texts
#define IME_HELP_TEXT_SEARCH		L"Search: 0-9 select, ⎗⎘←→ switch pages" //, F1 online info
#define IME_HELP_TEXT_CLIPBOARD		L"Clipboard Inspector: 0-9 select, ⏎ confirm, ⎗⎘←→ switch pages" //, F1 online info

#define IME_MODE_ON_ICON_INDEX      IDI_IME_MODE_ON
#define IME_MODE_OFF_ICON_INDEX     IDI_IME_MODE_OFF
#define IME_DOUBLE_ON_INDEX         IDI_DOUBLE_SINGLE_BYTE_ON
#define IME_DOUBLE_OFF_INDEX        IDI_DOUBLE_SINGLE_BYTE_OFF
#define IME_PUNCTUATION_ON_INDEX    IDI_PUNCTUATION_ON
#define IME_PUNCTUATION_OFF_INDEX   IDI_PUNCTUATION_OFF

// Unicode font for best results...
#define SAMPLEIME_FONT_DEFAULT L"Arial Unicode MS"

//---------------------------------------------------------------------
// defined Candidated Window
//---------------------------------------------------------------------
#define CANDWND_ROW_WIDTH				(25)
#define CANDWND_BORDER_COLOR			(RGB(0x00, 0x00, 0x00))
#define CANDWND_BORDER_WIDTH			(2)
#define CANDWND_NUM_COLOR				(RGB(0xB4, 0xB4, 0xB4))
#define CANDWND_SELECTED_ITEM_COLOR		(RGB(0xFF, 0xFF, 0xFF))
#define CANDWND_SELECTED_BK_COLOR		(RGB(0xA6, 0xA6, 0x00))
#define CANDWND_ITEM_COLOR				(RGB(0x00, 0x00, 0x00))

//---------------------------------------------------------------------
// defined modifier
//---------------------------------------------------------------------
#define _TF_MOD_ON_KEYUP_SHIFT_ONLY    (0x00010000 | TF_MOD_ON_KEYUP)
#define _TF_MOD_ON_KEYUP_CONTROL_ONLY  (0x00020000 | TF_MOD_ON_KEYUP)
#define _TF_MOD_ON_KEYUP_ALT_ONLY      (0x00040000 | TF_MOD_ON_KEYUP)

#define CAND_WIDTH     (15)      // * tmMaxCharWidth

//---------------------------------------------------------------------
// string length of CLSID
//---------------------------------------------------------------------
#define CLSID_STRLEN    (38)  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")
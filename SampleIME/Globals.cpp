// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "resource.h"
#include "BaseWindow.h"
#include "define.h"
#include "SampleIMEBaseStructure.h"

namespace Global {
HINSTANCE dllInstanceHandle;

LONG dllRefCount = -1;

CRITICAL_SECTION CS;
HFONT defaultlFontHandle;				// Global font object we use everywhere

// guidgenerator.com
/*
c4ee1d5f-cda3-4ec6-a15b-adbf40b7f7ad
2930f17f-aaaf-4725-8780-0452ff07f4e2
59fe8982-e85e-4481-a525-4806a991e233
a704c2f8-3897-422a-a677-f332ac811875
fe45b969-2fe7-4751-9061-edfb706894bf
b183f62c-8595-4a98-b04e-9edd32d5ac9f
1727cff7-4de5-4858-b67b-bdf89a9e66b9
937b4709-f4d8-4e8c-ad1b-760147de1f84
be8cc414-eaec-4e73-86ea-a257a1f0d7a0
61caf3e3-6c88-47f3-b964-27637301639d

*/


//---------------------------------------------------------------------
// SampleIME CLSID
//---------------------------------------------------------------------
// {c4ee1d5f-cda3-4ec6-a15b-adbf40b7f7ad}
extern const CLSID SampleIMECLSID = { 
    0xc4ee1d5f,
    0xcda3,
    0x4ec6,
    { 0xa1, 0x5b, 0xad, 0xbf, 0x40, 0xb7, 0xf7, 0xad }
};

//---------------------------------------------------------------------
// Profile GUID
//---------------------------------------------------------------------
// {83955C0E-2C09-47a5-BCF3-F2B98E11EE8B}
// 2930f17f-aaaf-4725-8780-0452ff07f4e2
extern const GUID SampleIMEGuidProfile = { 
    0x2930f17f,
    0xaaaf,
    0x4725,
    { 0x87, 0x80, 0x04, 0x52, 0xff, 0x07, 0xf4, 0xe2 }
};

//---------------------------------------------------------------------
// PreserveKey GUID
//---------------------------------------------------------------------
// {4B62B54B-F828-43B5-9095-A96DF9CBDF38}
// 59fe8982-e85e-4481-a525-4806a991e233
extern const GUID SampleIMEGuidComposePreservedKey = {
    0x59fe8982, 
    0xe85e, 
    0x4481, 
    { 0xa5, 0x25, 0x48, 0x06, 0xa9, 0x91, 0xe2, 0x23 } 
};


//---------------------------------------------------------------------
// UI element
//---------------------------------------------------------------------

// {84B0749F-8DE7-4732-907A-3BCB150A01A8}
// a704c2f8-3897-422a-a677-f332ac811875
extern const GUID SampleIMEGuidCandUIElement = {
    0xa704c2f8,
    0x3897,
    0x422a,
    { 0xA6, 0x77, 0xf3, 0x32, 0xac, 0x81, 0x18, 0x75 }
};

//---------------------------------------------------------------------
// Display Attribute
//---------------------------------------------------------------------
// {4C802E2C-8140-4436-A5E5-F7C544EBC9CD}
extern const GUID SampleIMEGuidDisplayAttributeInput = {
    0x4c802e2c,
    0x8140,
    0x4436,
    { 0xa5, 0xe5, 0xf7, 0xc5, 0x44, 0xeb, 0xc9, 0xcd }
};

// {9A1CC683-F2A7-4701-9C6E-2DA69A5CD474}
extern const GUID SampleIMEGuidDisplayAttributeConverted = {
    0x9a1cc683,
    0xf2a7,
    0x4701,
    { 0x9c, 0x6e, 0x2d, 0xa6, 0x9a, 0x5c, 0xd4, 0x74 }
};

// Supported languages
extern const WORD TextServiceLangIds[] = {
	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
//	MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
//	MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_SINGAPORE),
	0,
};

//---------------------------------------------------------------------
// Unicode byte order mark
//---------------------------------------------------------------------
extern const WCHAR UnicodeByteOrderMark = 0xFEFF;

//---------------------------------------------------------------------
// dictionary table delimiter
//---------------------------------------------------------------------
extern const WCHAR KeywordDelimiter = L'=';
extern const WCHAR StringDelimiter  = L'\"';

//---------------------------------------------------------------------
// defined item in setting file table [PreservedKey] section
//---------------------------------------------------------------------
extern const WCHAR ImeComposeDescription[] = L"Activate Unicode Compose";
extern const int ImeComposeOnIcoIndex = IME_MODE_ON_ICON_INDEX;
extern const int ImeComposeOffIcoIndex = IME_MODE_OFF_ICON_INDEX;

//---------------------------------------------------------------------
// windows class / titile / atom
//---------------------------------------------------------------------
extern const WCHAR CandidateClassName[] = L"UnicodeDbIME.CandidateWindow";
ATOM AtomCandidateWindow;

extern const WCHAR ShadowClassName[] = L"UnicodeDbIME.ShadowWindow";
ATOM AtomShadowWindow;

BOOL RegisterWindowClass()
{
    if (!CBaseWindow::_InitWindowClass(CandidateClassName, &AtomCandidateWindow))
    {
        return FALSE;
    }
    if (!CBaseWindow::_InitWindowClass(ShadowClassName, &AtomShadowWindow))
    {
        return FALSE;
    }
    return TRUE;
}

//---------------------------------------------------------------------
// defined full width characters for Double/Single byte conversion
//---------------------------------------------------------------------
extern const WCHAR FullWidthCharTable[] = {
    //         !       "       #       $       %       &       '       (    )       *       +       ,       -       .       /
    0x3000, 0xFF01, 0xFF02, 0xFF03, 0xFF04, 0xFF05, 0xFF06, 0xFF07, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B, 0xFF0C, 0xFF0D, 0xFF0E, 0xFF0F,
    // 0       1       2       3       4       5       6       7       8       9       :       ;       <       =       >       ?
    0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF1A, 0xFF1B, 0xFF1C, 0xFF1D, 0xFF1E, 0xFF1F,
    // @       A       B       C       D       E       F       G       H       I       J       K       L       M       N       0
    0xFF20, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F,
    // P       Q       R       S       T       U       V       W       X       Y       Z       [       \       ]       ^       _
    0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFF3B, 0xFF3C, 0xFF3D, 0xFF3E, 0xFF3F,
    // '       a       b       c       d       e       f       g       h       i       j       k       l       m       n       o       
    0xFF40, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F,
    // p       q       r       s       t       u       v       w       x       y       z       {       |       }       ~
    0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E
};

//---------------------------------------------------------------------
// defined punctuation characters
//---------------------------------------------------------------------
extern const struct _PUNCTUATION PunctuationTable[14] = {
    {L'!',  0xFF01},
    {L'$',  0xFFE5},
    {L'&',  0x2014},
    {L'(',  0xFF08},
    {L')',  0xFF09},
    {L',',  0xFF0C},
    {L'.',  0x3002},
    {L':',  0xFF1A},
    {L';',  0xFF1B},
    {L'?',  0xFF1F},
    {L'@',  0x00B7},
    {L'\\', 0x3001},
    {L'^',  0x2026},
    {L'_',  0x2014}
};

//+---------------------------------------------------------------------------
//
// CheckModifiers
//
//----------------------------------------------------------------------------

#define TF_MOD_ALLALT     (TF_MOD_RALT | TF_MOD_LALT | TF_MOD_ALT)
#define TF_MOD_ALLCONTROL (TF_MOD_RCONTROL | TF_MOD_LCONTROL | TF_MOD_CONTROL)
#define TF_MOD_ALLSHIFT   (TF_MOD_RSHIFT | TF_MOD_LSHIFT | TF_MOD_SHIFT)
#define TF_MOD_RLALT      (TF_MOD_RALT | TF_MOD_LALT)
#define TF_MOD_RLCONTROL  (TF_MOD_RCONTROL | TF_MOD_LCONTROL)
#define TF_MOD_RLSHIFT    (TF_MOD_RSHIFT | TF_MOD_LSHIFT)

#define CheckMod(m0, m1, mod)        \
    if (m1 & TF_MOD_ ## mod ##)      \
{ \
    if (!(m0 & TF_MOD_ ## mod ##)) \
{      \
    return FALSE;   \
}      \
} \
    else       \
{ \
    if ((m1 ^ m0) & TF_MOD_RL ## mod ##)    \
{      \
    return FALSE;   \
}      \
} \



BOOL CheckModifiers(UINT modCurrent, UINT mod)
{
    mod &= ~TF_MOD_ON_KEYUP;

    if (mod & TF_MOD_IGNORE_ALL_MODIFIER)
    {
        return TRUE;
    }

    if (modCurrent == mod)
    {
        return TRUE;
    }

    if (modCurrent && !mod)
    {
        return FALSE;
    }

    CheckMod(modCurrent, mod, ALT);
    CheckMod(modCurrent, mod, SHIFT);
    CheckMod(modCurrent, mod, CONTROL);

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// UpdateModifiers
//
//    wParam - virtual-key code
//    lParam - [0-15]  Repeat count
//  [16-23] Scan code
//  [24]    Extended key
//  [25-28] Reserved
//  [29]    Context code
//  [30]    Previous key state
//  [31]    Transition state
//----------------------------------------------------------------------------

USHORT ModifiersValue = 0;
BOOL   IsShiftKeyDownOnly = FALSE;
BOOL   IsControlKeyDownOnly = FALSE;
BOOL   IsAltKeyDownOnly = FALSE;

BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam)
{
    // high-order bit : key down
    // low-order bit  : toggled
    SHORT sksMenu = GetKeyState(VK_MENU);
    SHORT sksCtrl = GetKeyState(VK_CONTROL);
    SHORT sksShft = GetKeyState(VK_SHIFT);

    switch (wParam & 0xff)
    {
    case VK_MENU:
        // is VK_MENU down?
        if (sksMenu & 0x8000)
        {
            // is extended key?
            if (lParam & 0x01000000)
            {
                ModifiersValue |= (TF_MOD_RALT | TF_MOD_ALT);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LALT | TF_MOD_ALT);
            }

            // is previous key state up?
            if (!(lParam & 0x40000000))
            {
                // is VK_CONTROL and VK_SHIFT up?
                if (!(sksCtrl & 0x8000) && !(sksShft & 0x8000))
                {
                    IsAltKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    case VK_CONTROL:
        // is VK_CONTROL down?
        if (sksCtrl & 0x8000)    
        {
            // is extended key?
            if (lParam & 0x01000000)
            {
                ModifiersValue |= (TF_MOD_RCONTROL | TF_MOD_CONTROL);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LCONTROL | TF_MOD_CONTROL);
            }

            // is previous key state up?
            if (!(lParam & 0x40000000))
            {
                // is VK_SHIFT and VK_MENU up?
                if (!(sksShft & 0x8000) && !(sksMenu & 0x8000))
                {
                    IsControlKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    case VK_SHIFT:
        // is VK_SHIFT down?
        if (sksShft & 0x8000)    
        {
            // is scan code 0x36(right shift)?
            if (((lParam >> 16) & 0x00ff) == 0x36)
            {
                ModifiersValue |= (TF_MOD_RSHIFT | TF_MOD_SHIFT);
            }
            else
            {
                ModifiersValue |= (TF_MOD_LSHIFT | TF_MOD_SHIFT);
            }

            // is previous key state up?
            if (!(lParam & 0x40000000))
            {
                // is VK_MENU and VK_CONTROL up?
                if (!(sksMenu & 0x8000) && !(sksCtrl & 0x8000))
                {
                    IsShiftKeyDownOnly = TRUE;
                }
                else
                {
                    IsShiftKeyDownOnly = FALSE;
                    IsControlKeyDownOnly = FALSE;
                    IsAltKeyDownOnly = FALSE;
                }
            }
        }
        break;

    default:
        IsShiftKeyDownOnly = FALSE;
        IsControlKeyDownOnly = FALSE;
        IsAltKeyDownOnly = FALSE;
        break;
    }

    if (!(sksMenu & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLALT;
    }
    if (!(sksCtrl & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLCONTROL;
    }
    if (!(sksShft & 0x8000))
    {
        ModifiersValue &= ~TF_MOD_ALLSHIFT;
    }

    return TRUE;
}

//---------------------------------------------------------------------
// override CompareElements
//---------------------------------------------------------------------
BOOL CompareElements(LCID locale, const std::wstring* pElement1, const std::wstring* pElement2)
{
	return (*pElement1).compare(*pElement2) == 0;
}
}
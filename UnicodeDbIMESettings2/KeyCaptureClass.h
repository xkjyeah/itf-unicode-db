#pragma once

#include "stdafx.h"

#include <msctf.h>
#include <cstdint>

namespace UnicodeDbIMESettings2 {
	using namespace System::Windows::Forms;
	
	typedef uint16_t TfModModifiers;

    public ref class KeyCapture
    {
        /* From Microsoft documentation:
            * (Apparently the Windows Key is not supported by PreserveKey)
TF_MOD_ALT
0x0001
Either of the ALT keys is pressed
TF_MOD_CONTROL
0x0002
Either of the CTRL keys is pressed
TF_MOD_SHIFT
0x0004
Either of the SHIFT keys is pressed
TF_MOD_RALT
0x0008
The right ALT key is pressed
TF_MOD_RCONTROL
0x0010
The right CTRL key is pressed
TF_MOD_RSHIFT
0x0020
The right SHIFT key is pressed
TF_MOD_LALT
0x0040
The left ALT key is pressed
TF_MOD_LCONTROL
0x0080
The left CTRL key is pressed
TF_MOD_LSHIFT
0x0100
The left SHIFT key is pressed
TF_MOD_ON_KEYUP
0x0200
The event will be fired when the key is released. Without this flag, the event is fired when the key is pressed.
TF_MOD_IGNORE_ALL_MODIFIER
0x0400 */

	public:
		KeyCapture() : currentModifiers(0), currentKey((Keys)0)
		{
		}

        KeyCapture(TfModModifiers modifiers, Keys key)
        {
            this->currentModifiers = modifiers;
            this->currentKey = key;
        }

		TfModModifiers currentModifiers;
        Keys currentKey;

        TfModModifiers capturingModifiers;
        Keys capturingKey;

        virtual System::String ^ToString() override
        {
            return ToString(false);
        }

        virtual System::String ^ToString(bool capturing)
        {
            TfModModifiers mods = capturing ? capturingModifiers : currentModifiers;
            Keys key = capturing ? capturingKey : currentKey;
            System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder();

            if (mods & TF_MOD_CONTROL)
                sb->Append("Control+");
            if (mods & TF_MOD_ALT)
                sb->Append("Alt+");
            if (mods & TF_MOD_SHIFT)
                sb->Append("Shift+");

            sb->Append(key.ToString());

            return sb->ToString();
        }
    };
}
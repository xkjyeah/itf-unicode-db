// UnicodeDbIMESettings2.cpp : main project file.

#include "stdafx.h"
#include "IMESettings.h"
#include <ObjBase.h>

using namespace UnicodeDbIMESettings2;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Initialize COM
	HRESULT hr;
	hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if (!SUCCEEDED(hr)) {
		/* TODO: show a message or something */
		return 1;
	}

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew SettingsForm());
	
	CoUninitialize();
	return 0;
}

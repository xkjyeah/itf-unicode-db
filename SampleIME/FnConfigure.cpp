#include "Private.h"
#include "globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "Compartment.h"

STDMETHODIMP CSampleIME::Show(_In_ HWND hwndParent, _In_ LANGID langid, _In_ REFGUID rguidProfile)
{
	// execute the settings file
	std::wstring exePath;
	{
		// first find the settings app. It is ../UnicodeDbIMESettings.exe
		// Register CFileMapping
		WCHAR wszFileName[MAX_PATH] = {'\0'};
		DWORD cchA = GetModuleFileName(Global::dllInstanceHandle, wszFileName, ARRAYSIZE(wszFileName));

		for (int i=0; i<2; i++) {
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
		}
	
		exePath = wszFileName;
		exePath.append(TEXTSERVICE_SETTINGS_APP);
	}

	{
		PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
		STARTUPINFO StartupInfo; //This is an [in] parameter
		ZeroMemory(&StartupInfo, sizeof(StartupInfo));
		StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field

		if (CreateProcess(exePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) { 
			CloseHandle(ProcessInfo.hProcess);
			CloseHandle(ProcessInfo.hThread);
			return S_OK;
		}
		else {
			return S_FALSE;
		}
	}
}
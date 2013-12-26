#include "Private.h"
#include "globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "Compartment.h"

STDMETHODIMP CSampleIME::Show(_In_ HWND hwndParent, _In_ LANGID langid, _In_ REFGUID rguidProfile)
{
	return S_OK;
}
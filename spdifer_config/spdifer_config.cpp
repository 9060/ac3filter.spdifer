#include <windows.h>
#include <ocidl.h>
#include <olectl.h>
#include "..\guids.h"

INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
  ISpecifyPropertyPages *spp;
  CAUUID cauuid;
  HRESULT hr;

  CoInitialize(0);
  if FAILED(CoCreateInstance(CLSID_Spdifer, NULL, CLSCTX_INPROC_SERVER, IID_ISpecifyPropertyPages, (LPVOID *)&spp))
  {
    MessageBox(0, "Cannot create Spdifer instance (Spdifer is not installed?)", 
      "ERROR!", MB_OK | MB_ICONSTOP);
    return 1;
  }

  spp->GetPages(&cauuid);
  hr = OleCreatePropertyFrame(0, 30, 30, NULL, 1, (IUnknown **)&spp, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);
  CoTaskMemFree(cauuid.pElems);
  spp->Release();
  CoUninitialize();
  return 0;
};
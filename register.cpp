#include "guids.h"
#include "spdifer_ds.h"
#include "spdifer_dlg.h"

// setup data

const AMOVIESETUP_MEDIATYPE sudInPinTypes[] =
{
// MPEG2 formats
{&MEDIATYPE_MPEG2_PES,          &MEDIASUBTYPE_DOLBY_AC3         },
{&MEDIATYPE_MPEG2_PES,          &MEDIASUBTYPE_MPEG1AudioPayload },
{&MEDIATYPE_MPEG2_PES,          &MEDIASUBTYPE_MPEG2_AUDIO       },
{&MEDIATYPE_MPEG2_PES,          &MEDIASUBTYPE_DTS               },
// DVD formats
{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_AC3         },
{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG1AudioPayload },
{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_AUDIO       },
{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS               },
// Compressed formats
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_AVI_AC3           },
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_AVI_DTS           },
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_DOLBY_AC3         },
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_DTS               },
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_MPEG1AudioPayload },
{&MEDIATYPE_Audio,              &MEDIASUBTYPE_MPEG2_AUDIO       },
};

const AMOVIESETUP_MEDIATYPE sudOutPinTypes[] =
{
{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM             },
{&MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_AC3_SPDIF },
};

const AMOVIESETUP_PIN psudPins[] =                 
{
  {
    L"Input",           // String pin name
    FALSE,              // Is it rendered
    FALSE,              // Is it an output
    FALSE,              // Allowed none
    FALSE,              // Allowed many
    &CLSID_NULL,        // Connects to filter
    L"Output",          // Connects to pin
    array_size(sudInPinTypes), // Number of types
    sudInPinTypes       // The pin details
  },     
  { 
    L"Output",          // String pin name
    FALSE,              // Is it rendered
    TRUE,               // Is it an output
    FALSE,              // Allowed none
    FALSE,              // Allowed many
    &CLSID_NULL,        // Connects to filter
    L"Input",           // Connects to pin
    array_size(sudOutPinTypes), // Number of types
    sudOutPinTypes      // The pin details
  }
};

const AMOVIESETUP_FILTER sudSpdifer =
{
    &CLSID_Spdifer,         // Filter CLSID
    L"Spdifer",             // Filter name
    0x40000000,             // Its merit      MERIT_PREFERRED
    2,                      // Number of pins
    psudPins                // Pin details
};

CFactoryTemplate g_Templates[] = {
    { L"Spdifer Main page"
    , &CLSID_SpdiferMain
    , SpdiferDlg::CreateMain },
    { L"Spdifer About page"
    , &CLSID_SpdiferAbout
    , SpdiferDlg::CreateAbout },
    { L"Spdifer"
    , &CLSID_Spdifer
    , SpdiferDS::CreateInstance
    , NULL
    , &sudSpdifer }
};
int g_cTemplates = array_size(g_Templates);


//
// Handle registration of this filter
//

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  return DllEntryPoint(hinst, reason, reserved);
}

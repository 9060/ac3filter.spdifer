/*
  Class GUIDs
  Interface GUIDs
  Interface definitions
*/

#ifndef SPDIFER_GUIDS_H
#define SPDIFER_GUIDS_H

#include <objbase.h>
#include "spk.h"
#include "registry.h"

// registry key
#define REG_KEY        "Software\\AC3Filter\\Spdifer"

// Constants from dvd_graph.h
#define SPDIF_MODE_NONE                0
#define SPDIF_MODE_DISABLED            1
#define SPDIF_MODE_PASSTHROUGH         2
#define SPDIF_MODE_ENCODE              3

// Constants from spdif_wrapper.h
#define DTS_MODE_AUTO    0
#define DTS_MODE_WRAPPED 1
#define DTS_MODE_PADDED  2

#define DTS_CONV_NONE    0
#define DTS_CONV_16BIT   1
#define DTS_CONV_14BIT   2

///////////////////////////////////////////////////////////////////////////////
// Media types
///////////////////////////////////////////////////////////////////////////////

DEFINE_GUID(MEDIASUBTYPE_AVI_AC3, 
0x00002000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(MEDIASUBTYPE_AVI_DTS, 
0x00002001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

///////////////////////////////////////////////////////////////////////////////
// GUIDs
///////////////////////////////////////////////////////////////////////////////

// {67E84447-3BDD-41c7-B224-6BD3058DC6CC}
DEFINE_GUID(IID_ISpdifer, 
0x67e84447, 0x3bdd, 0x41c7, 0xb2, 0x24, 0x6b, 0xd3, 0x5, 0x8d, 0xc6, 0xcc);

// {38CD9158-EC2E-4292-8FFE-7A86EF4F1EDF}
DEFINE_GUID(CLSID_Spdifer, 
0x38cd9158, 0xec2e, 0x4292, 0x8f, 0xfe, 0x7a, 0x86, 0xef, 0x4f, 0x1e, 0xdf);

// {5E118CAF-D53C-4de8-9A48-1AB207A5C4DD}
DEFINE_GUID(CLSID_SpdiferMain, 
0x5e118caf, 0xd53c, 0x4de8, 0x9a, 0x48, 0x1a, 0xb2, 0x7, 0xa5, 0xc4, 0xdd);

// {BB519E0D-F082-4d85-A696-45D5E9CA234D}
DEFINE_GUID(CLSID_SpdiferAbout, 
0xbb519e0d, 0xf082, 0x4d85, 0xa6, 0x96, 0x45, 0xd5, 0xe9, 0xca, 0x23, 0x4d);

///////////////////////////////////////////////////////////////////////////////
// Interfaces
///////////////////////////////////////////////////////////////////////////////

struct SpdiferParams
{
  int  formats;
  int  dts_mode;
  int  dts_conv;
  bool spdif_as_pcm;
  bool spdif_check_sr;
  bool spdif_allow_48;
  bool spdif_allow_44;
  bool spdif_allow_32;
  int  reinit;
};

DECLARE_INTERFACE_(ISpdifer, IUnknown)
{
  // Current stream info
  STDMETHOD (get_info) (char *buf, size_t size, int *frames, int *errors) = 0;

  // Input formats to accept (formats bitmask)
  STDMETHOD (get_formats) (int *formats) = 0;
  STDMETHOD (set_formats) (int  formats) = 0;

  // SPDIF/DTS output mode
  STDMETHOD (get_dts_mode) (int *dts_mode) = 0;
  STDMETHOD (set_dts_mode) (int  dts_mode) = 0;

  // SPDIF/DTS conversion
  STDMETHOD (get_dts_conv) (int *dts_conv) = 0;
  STDMETHOD (set_dts_conv) (int  dts_conv) = 0;

  // SPDIF as PCM output
  STDMETHOD (get_spdif_as_pcm)(bool *spdif_as_pcm) = 0;
  STDMETHOD (set_spdif_as_pcm)(bool  spdif_as_pcm) = 0;

  // SPDIF check sample rate
  STDMETHOD (get_spdif_check_sr)(bool *spdif_check_sr) = 0;
  STDMETHOD (set_spdif_check_sr)(bool  spdif_check_sr) = 0;
  STDMETHOD (get_spdif_allow_48)(bool *spdif_allow_48) = 0;
  STDMETHOD (set_spdif_allow_48)(bool  spdif_allow_48) = 0;
  STDMETHOD (get_spdif_allow_44)(bool *spdif_allow_44) = 0;
  STDMETHOD (set_spdif_allow_44)(bool  spdif_allow_44) = 0;
  STDMETHOD (get_spdif_allow_32)(bool *spdif_allow_32) = 0;
  STDMETHOD (set_spdif_allow_32)(bool  spdif_allow_32) = 0;

  // Reinit after seek/pause option
  STDMETHOD (get_reinit)(int *reinit) = 0;
  STDMETHOD (set_reinit)(int  reinit) = 0;

  // Load/save settings
  STDMETHOD (get_params) (SpdiferParams *params) = 0;
  STDMETHOD (set_params) (SpdiferParams *params) = 0;
  STDMETHOD (load_params) (Config *config) = 0;
  STDMETHOD (save_params) (Config *config) = 0;
};

#endif

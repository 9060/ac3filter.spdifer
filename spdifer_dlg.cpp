#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

#include "guids.h"
#include "registry.h"
#include "resource.h"
#include "spdifer_dlg.h"
#include "spdifer_ver.h"


#define SAFE_RELEASE(p) { if (p) p->Release(); p = 0; }

#define dlg_printf(dlg, ctrl, format, params)                     \
{                                                                 \
  char buf[255];                                                  \
  sprintf(buf, format, ##params);                                 \
  SendDlgItemMessage(dlg, ctrl, WM_SETTEXT, 0, (LONG)(LPSTR)buf); \
}

///////////////////////////////////////////////////////////////////////////////
// Registry functions
///////////////////////////////////////////////////////////////////////////////

bool delete_reg_key(const char *name, HKEY root)
{
  HKEY  key;
  char  buf[256];
  DWORD len;

  if (RegOpenKeyEx(root, name, 0, KEY_READ | KEY_WRITE, &key) != ERROR_SUCCESS)
    return false;

  len = 256;
  while (RegEnumKeyEx(key, 0, (LPTSTR)buf, &len, 0, 0, 0, 0) == ERROR_SUCCESS)
  {
    len = 256;
    if (!delete_reg_key(buf, key))
    {
      RegCloseKey(key);
      return false;
    }
  }

  len = 256;
  while (RegEnumValue(key, 0, (LPTSTR)buf, &len, 0, 0, 0, 0) == ERROR_SUCCESS)
  {
    len = 256;
    if (RegDeleteValue(key, buf) != ERROR_SUCCESS)
    {
      RegCloseKey(key);
      return false;
    }
  }

  RegCloseKey(key);
  RegDeleteKey(root, name);
  return true;
}

int get_merit(HKEY hive, LPCSTR key)
{
  HKEY  reg;
  if (RegOpenKeyEx(hive, key, 0, KEY_READ, &reg) != ERROR_SUCCESS)
    return 0;
  
  DWORD buf[256];
  DWORD buf_len = 1024;
  DWORD type;

  if (RegQueryValueEx(reg, "FilterData", NULL, &type, (LPBYTE)&buf, &buf_len) != ERROR_SUCCESS)
  {
    RegCloseKey(reg);
    return 0;
  }

  RegCloseKey(reg);

  if (type != REG_BINARY || buf_len < sizeof(REGFILTER2) || buf[0] != 2)
    return 0;

  return buf[1];
}


bool set_merit(HKEY hive, LPCSTR key, int merit)
{
  HKEY  reg;
  if (RegOpenKeyEx(hive, key, 0, KEY_READ | KEY_WRITE, &reg) != ERROR_SUCCESS)
    return false;
  
  DWORD buf[256];
  DWORD buf_len = 1024;
  DWORD type;

  if (RegQueryValueEx(reg, "FilterData", NULL, &type, (LPBYTE)&buf, &buf_len) != ERROR_SUCCESS)
  {
    RegCloseKey(reg);
    return false;
  }

  if (type != REG_BINARY || buf_len < sizeof(REGFILTER2) || buf[0] != 2)
  {
    RegCloseKey(reg);
    return false;
  }

  buf[1] = merit;

  if (RegSetValueEx(reg, "FilterData", NULL, REG_BINARY, (LPBYTE)&buf, buf_len) != ERROR_SUCCESS)
    return false;

  RegCloseKey(reg);
  return true;
}



///////////////////////////////////////////////////////////////////////////////
// Initialization / Deinitialization
///////////////////////////////////////////////////////////////////////////////

CUnknown * WINAPI SpdiferDlg::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
  CUnknown *punk = new SpdiferDlg("Spdifer property page", lpunk, phr, IDD_SPDIF, IDS_SPDIF);
  if (punk == NULL) *phr = E_OUTOFMEMORY;
  return punk;
}

SpdiferDlg::SpdiferDlg(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, int DialogId, int TitleId)
:CBasePropertyPage(pName, pUnk, DialogId, TitleId)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::SpdiferDlg(%s)", pName));
  filter = 0;
  InitCommonControls();
}

HRESULT 
SpdiferDlg::OnConnect(IUnknown *pUnknown)
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::OnConnect()"));

  pUnknown->QueryInterface(IID_ISpdifer, (void **)&filter);
  if (!filter)
  {
    DbgLog((LOG_TRACE, 3, "SpdiferDlg::OnConnect() Failed!"));
    SAFE_RELEASE(filter);
    return E_NOINTERFACE; 
  }

  refresh_time = 100;
  RegistryKey reg(REG_KEY);
  reg.get_int32("refresh", refresh_time);
  return NOERROR;
}

HRESULT 
SpdiferDlg::OnDisconnect()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::OnDisconnect()"));

  if (filter) filter->save_params(0);
  SAFE_RELEASE(filter);
  return NOERROR;
}

HRESULT 
SpdiferDlg::OnActivate()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::OnActivate()"));

  visible = true;
  refresh = true;

  // Init and update controls
  init();
  update();

  SetTimer(m_hwnd, 1, refresh_time, 0);
  return NOERROR;
}

HRESULT 
SpdiferDlg::OnDeactivate()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::OnDeactivate()"));
  KillTimer(m_hwnd, 1);
  return NOERROR;
}

///////////////////////////////////////////////////////////////////////////////
// Handle messages
///////////////////////////////////////////////////////////////////////////////

BOOL 
SpdiferDlg::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
      command(LOWORD(wParam), HIWORD(wParam));
      return 1;

    case WM_HSCROLL:
    case WM_VSCROLL:
      command(GetDlgCtrlID((HWND)lParam), LOWORD(wParam));
      return 1;

    case WM_TIMER:
      /////////////////////////////////////////////////////
      // For some reasons OlePropertyFrame does not send
      // WM_SHOW message to property pages. But we need it
      // to update our ststic controls that may be changed
      // at other pages. Therefore we have to detect show
      // event based on window visibility. Disadvantage of
      // this method is that user can see control updates.
      //
      // Note: WM_SHOW message is not sent when window has
      // WS_MAXIMIZEBOX style. For some reasons dialog page
      // is wrapped in one more dialog page as following:
      //
      //   Dialog tab set
      //     Wrapper page (with WS_MAXIMIZEBOX style)
      //       Page 1
      //     Wrapper page (with WS_MAXIMIZEBOX style)
      //       Page 2
      //     ...

      if (IsWindowVisible(hwnd))
        if (visible)
        {
          // normal update
          switch (wParam)
          {
            case 1:
              reload_state();
              update_dynamic_controls();
              break;
          }
        }
        else
        {
          // show window
          refresh = true;
          visible = true;
          
          // update all controls
          // (static controls may be changed at other windows)
          update();
        }
      else
        // hide
        visible = false;

      return 1;

  }

  return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// Controls initalization/update
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Top-level

void 
SpdiferDlg::init()
{
  init_controls();
}

void 
SpdiferDlg::update()
{
  reload_state();
  update_dynamic_controls();
  update_static_controls();
}

///////////////////////////////////////////////////////////////////////////////
// Underground

void 
SpdiferDlg::reload_state()
{
  filter->get_params(this);
}

void 
SpdiferDlg::init_controls()
{
  DbgLog((LOG_TRACE, 3, "SpdiferDlg::init_controls()"));

  /////////////////////////////////////
  // Links

  lnk_home.link(m_Dlg, IDC_LNK_HOME);
  lnk_forum.link(m_Dlg, IDC_LNK_FORUM);
  lnk_email.link(m_Dlg, IDC_LNK_EMAIL);

  /////////////////////////////////////
  // Version

  char ver1[255];
  char ver2[255];
  GetDlgItemText(m_Dlg, IDC_VER, ver1, array_size(ver1));
  sprintf(ver2, ver1, SPDIFER_VER);
  SetDlgItemText(m_Dlg, IDC_VER, ver2);
}

void 
SpdiferDlg::update_dynamic_controls()
{
  /////////////////////////////////////
  // Stream info

  int frames;
  int errors;
  char info[sizeof(old_info)];
  memset(info, 0, sizeof(old_info));
  filter->get_info(info, sizeof(old_info), &frames, &errors);
  if (memcmp(info, old_info, sizeof(old_info)) || refresh)
  {
    memcpy(old_info, info, sizeof(old_info));
    SendDlgItemMessage(m_Dlg, IDC_EDT_INFO, WM_SETTEXT, 0, (LONG)(LPSTR)info);
  }

  dlg_printf(m_Dlg, IDC_EDT_FRAMES, "%i", frames);
  dlg_printf(m_Dlg, IDC_EDT_ERRORS, "%i", errors);
  refresh = false;
}

void 
SpdiferDlg::update_static_controls()
{
  m_bDirty = true;
  if(m_pPageSite)
    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

  /////////////////////////////////////
  // SPDIF passthrough

  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_MPA, (formats & FORMAT_MASK_MPA) != 0? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_AC3, (formats & FORMAT_MASK_AC3) != 0? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_DTS, (formats & FORMAT_MASK_DTS) != 0? BST_CHECKED: BST_UNCHECKED);

  /////////////////////////////////////
  // SPDIF/DTS output mode

  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_MODE_AUTO,    BM_SETCHECK, dts_mode == DTS_MODE_AUTO? BST_CHECKED: BST_UNCHECKED, 1);
  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_MODE_WRAPPED, BM_SETCHECK, dts_mode == DTS_MODE_WRAPPED? BST_CHECKED: BST_UNCHECKED, 1);
  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_MODE_PADDED,  BM_SETCHECK, dts_mode == DTS_MODE_PADDED? BST_CHECKED: BST_UNCHECKED, 1);

  /////////////////////////////////////
  // SPDIF/DTS conversion

  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_CONV_NONE,    BM_SETCHECK, dts_conv == DTS_CONV_NONE? BST_CHECKED: BST_UNCHECKED, 1);
  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_CONV_14BIT,   BM_SETCHECK, dts_conv == DTS_CONV_14BIT? BST_CHECKED: BST_UNCHECKED, 1);
  SendDlgItemMessage(m_Dlg, IDC_RBT_DTS_CONV_16BIT,   BM_SETCHECK, dts_conv == DTS_CONV_16BIT? BST_CHECKED: BST_UNCHECKED, 1);

  /////////////////////////////////////
  // SPDIF options

  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_AS_PCM, spdif_as_pcm? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_CHECK_SR, spdif_check_sr? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_ALLOW_48, spdif_allow_48? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_ALLOW_44, spdif_allow_44? BST_CHECKED: BST_UNCHECKED);
  CheckDlgButton(m_Dlg, IDC_CHK_SPDIF_ALLOW_32, spdif_allow_32? BST_CHECKED: BST_UNCHECKED);
  EnableWindow(GetDlgItem(m_Dlg, IDC_CHK_SPDIF_ALLOW_48), spdif_check_sr);
  EnableWindow(GetDlgItem(m_Dlg, IDC_CHK_SPDIF_ALLOW_44), spdif_check_sr);
  EnableWindow(GetDlgItem(m_Dlg, IDC_CHK_SPDIF_ALLOW_32), spdif_check_sr);
  CheckDlgButton(m_Dlg, IDC_CHK_REINIT, reinit > 0? BST_CHECKED: BST_UNCHECKED);
}

///////////////////////////////////////////////////////////////////////////////
// Commands
///////////////////////////////////////////////////////////////////////////////

void 
SpdiferDlg::command(int control, int message)
{
  switch (control)
  {
    /////////////////////////////////////
    // SPDIF passthrough

    case IDC_CHK_SPDIF_MPA:
    case IDC_CHK_SPDIF_AC3:
    case IDC_CHK_SPDIF_DTS:
    {
      formats = 0;
      formats |= IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_MPA) == BST_CHECKED? FORMAT_MASK_MPA: 0;
      formats |= IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_AC3) == BST_CHECKED? FORMAT_MASK_AC3: 0;
      formats |= IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_DTS) == BST_CHECKED? FORMAT_MASK_DTS: 0;
      filter->set_formats(formats);
      update();
      break;
    }

    /////////////////////////////////////
    // SPDIF options

    case IDC_CHK_SPDIF_AS_PCM:
    {
      spdif_as_pcm = IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_AS_PCM) == BST_CHECKED;
      if (spdif_as_pcm)
        spdif_as_pcm = MessageBox(m_Dlg, "This option is DANGEROUS! Filter may make very loud noise with this option enabled. Press 'No' to enable this option.", "Dangerous option!", MB_YESNO | MB_ICONWARNING) == IDNO;
      filter->set_spdif_as_pcm(spdif_as_pcm);
      update();
      break;
    }

    case IDC_CHK_SPDIF_CHECK_SR:
    {
      spdif_check_sr = IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_CHECK_SR) == BST_CHECKED;
      filter->set_spdif_check_sr(spdif_check_sr);
      update();
      break;
    }

    case IDC_CHK_SPDIF_ALLOW_48:
    {
      spdif_allow_48 = IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_ALLOW_48) == BST_CHECKED;
      filter->set_spdif_allow_48(spdif_allow_48);
      update();
      break;
    }

    case IDC_CHK_SPDIF_ALLOW_44:
    {
      spdif_allow_44 = IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_ALLOW_44) == BST_CHECKED;
      filter->set_spdif_allow_44(spdif_allow_44);
      update();
      break;
    }

    case IDC_CHK_SPDIF_ALLOW_32:
    {
      spdif_allow_32 = IsDlgButtonChecked(m_Dlg, IDC_CHK_SPDIF_ALLOW_32) == BST_CHECKED;
      filter->set_spdif_allow_32(spdif_allow_32);
      update();
      break;
    }

    /////////////////////////////////////
    // Force reinit

    case IDC_CHK_REINIT:
    {
      reinit = IsDlgButtonChecked(m_Dlg, IDC_CHK_REINIT) == BST_CHECKED? 128: 0;
      if (reinit)
        reinit = MessageBox(m_Dlg, "This option is DANGEROUS! Video may go out of sync with this option enabled. Press 'No' to enable this option.", "Dangerous option!", MB_YESNO | MB_ICONWARNING) == IDNO;
      filter->set_reinit(reinit);
      update();
      break;
    }

    /////////////////////////////////////
    // SPDIF/DTS output mode

    case IDC_RBT_DTS_MODE_AUTO:
    {
      dts_mode = DTS_MODE_AUTO;
      filter->set_dts_mode(dts_mode);
      update();
      break;
    }

    case IDC_RBT_DTS_MODE_WRAPPED:
    {
      dts_mode = DTS_MODE_WRAPPED;
      filter->set_dts_mode(dts_mode);
      update();
      break;
    }

    case IDC_RBT_DTS_MODE_PADDED:
    {
      dts_mode = DTS_MODE_PADDED;
      filter->set_dts_mode(dts_mode);
      update();
      break;
    }

    /////////////////////////////////////
    // SPDIF/DTS conversion

    case IDC_RBT_DTS_CONV_NONE:
    {
      dts_conv = DTS_CONV_NONE;
      filter->set_dts_conv(dts_conv);
      update();
      break;
    }

    case IDC_RBT_DTS_CONV_14BIT:
    {
      dts_conv = DTS_CONV_14BIT;
      filter->set_dts_conv(dts_conv);
      update();
      break;
    }

    case IDC_RBT_DTS_CONV_16BIT:
    {
      dts_conv = DTS_CONV_16BIT;
      filter->set_dts_conv(dts_conv);
      update();
      break;
    }

    /////////////////////////////////////
    // Donate

    case IDC_BTN_DONATE:
      if (message == BN_CLICKED)
        ShellExecute(0, 0, "http://order.kagi.com/?6CZJZ", 0, 0, SW_SHOWMAXIMIZED);
      break;
  }
}

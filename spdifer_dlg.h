#ifndef SPDIFER_DLG_H
#define SPDIFER_DLG_H

#include <streams.h>
#include "controls.h"
#include "guids.h"
          
class SpdiferDlg : public CBasePropertyPage, public SpdiferParams
{
public:
  static CUnknown * WINAPI CreateMain(LPUNKNOWN lpunk, HRESULT *phr);
  static CUnknown * WINAPI CreateAbout(LPUNKNOWN lpunk, HRESULT *phr);

  void reload_state();

private:
  ISpdifer *filter;
  bool visible;
  bool refresh;
  int  refresh_time;

  // Options
  char old_info[1024];

  // Links
  LinkButton  lnk_home;
  LinkButton  lnk_forum;
  LinkButton  lnk_email;
  LinkButton  lnk_donate;

  SpdiferDlg(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr, int DialogId, int TitleId);

  INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HRESULT OnConnect(IUnknown *pUnknown);
  HRESULT OnDisconnect();
  HRESULT OnActivate();
  HRESULT OnDeactivate();

  /////////////////////////////////////////////////////////////////////////////
  // Interface update functions

  void init();
  void update();
  void init_controls();
  void update_dynamic_controls();
  void update_static_controls();

  /////////////////////////////////////////////////////////////////////////////
  // Handle control notifications

  void command(int control, int message);
};

#endif

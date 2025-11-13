#include <vector>
#include "framed_browser_view.h"
#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_types.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "muon_handler.h"
#include "project_panel.h"

class MuonWindowDelegate : public CefWindowDelegate
{
public:
  MuonWindowDelegate(CefRefPtr<MuonHandler> handler, cef_show_state_t initial_show_state);
  void OnWindowCreated(CefRefPtr<CefWindow> window) override;

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override;

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override;


  bool CanClose(CefRefPtr<CefWindow> window) override;

  // bool OnKeyEvent(CefRefPtr<CefWindow> window,
  //                 const CefKeyEvent& event) override;


  cef_runtime_style_t GetWindowRuntimeStyle() override;

  CefSize GetPreferredSize(CefRefPtr<CefView> /*view*/) override ;

private:
  CefRefPtr<MuonHandler> handler;
  //Crashes on close when project.size() > 1
  std::vector<CefRefPtr<ProjectPanel>> projects;
  int active_project_idx;
  cef_show_state_t initial_show_state;
  IMPLEMENT_REFCOUNTING(MuonWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(MuonWindowDelegate);
};

#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_types.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "muon_handler.h"
class MuonWindowDelegate : public CefWindowDelegate
{
public:
  MuonWindowDelegate(CefRefPtr<MuonHandler> handler, cef_show_state_t initial_show_state);
  void OnWindowCreated(CefRefPtr<CefWindow> window) override;
  void OnWindowClosing(CefRefPtr<CefWindow> window) override;
  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override;
  void OnWindowActivationChanged(CefRefPtr<CefWindow> window,
                                 bool active) override;
  void OnWindowBoundsChanged(CefRefPtr<CefWindow> window,
                             const CefRect& new_bounds) override;
  void OnWindowFullscreenTransition(CefRefPtr<CefWindow> window,
                                    bool is_completed) override;


  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override;


  bool CanClose(CefRefPtr<CefWindow> window) override;

  bool OnKeyEvent(CefRefPtr<CefWindow> window,
                  const CefKeyEvent& event) override;


  cef_runtime_style_t GetWindowRuntimeStyle() override;
private:
  CefRefPtr<MuonHandler> handler;
  CefRefPtr<CefBrowserView> browser_view;
  cef_show_state_t initial_show_state;
  IMPLEMENT_REFCOUNTING(MuonWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(MuonWindowDelegate);
};

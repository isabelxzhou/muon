#include "muon_app.h"


#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include "muon_handler.h"
#include "muon_window_delegate.h"

MuonApp::MuonApp() = default;

void MuonApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();


  // MuonHandler implements browser-level callbacks.
  CefRefPtr<MuonHandler> handler(new MuonHandler());

  cef_show_state_t initial_show_state = CEF_SHOW_STATE_NORMAL;

  CefWindow::CreateTopLevelWindow(
      new MuonWindowDelegate(handler, initial_show_state));
}
CefRefPtr<CefClient> MuonApp::GetDefaultClient() {
  // Called when a new browser window is created via Chrome style UI.
  return MuonHandler::GetInstance();
}

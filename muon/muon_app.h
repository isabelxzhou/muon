#ifndef CEF_TESTS_MUON_SIMPLE_APP_H_
#define CEF_TESTS_MUON_SIMPLE_APP_H_

#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class MuonApp : public CefApp, public CefBrowserProcessHandler {
 public:
  MuonApp();

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(MuonApp);
};

#endif  // CEF_TESTS_MUON_SIMPLE_APP_H_

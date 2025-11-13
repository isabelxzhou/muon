// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_MUON_MUON_HANDLER_H_
#define CEF_TESTS_MUON_MUON_HANDLER_H_

#include <functional>
#include <list>

#include "include/cef_client.h"

class MuonHandler;

class MuonWindowDelegate;

class MuonHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefKeyboardHandler {
 public:
  explicit MuonHandler();
  ~MuonHandler() override;

  // Provide access to the single global instance of this object.
  static MuonHandler* GetInstance();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override { return this; }
  
  void SetWindowDelegate(MuonWindowDelegate* delegate) { window_delegate_ = delegate; }

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  void OnAddressChange(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                       const CefString& url) override;
  
  // CefKeyboardHandler methods:
  bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                     const CefKeyEvent& event,
                     CefEventHandle os_event,
                     bool* is_keyboard_shortcut) override;
  
  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   ErrorCode errorCode,
                   const CefString& errorText,
                   const CefString& failedUrl) override;

  void ShowMainWindow();

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

  // Set callback for title updates
  void SetTitleUpdateCallback(std::function<void(CefRefPtr<CefBrowser>, const CefString&)> callback) {
    title_update_callback_ = callback;
  }

 private:
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);
  void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_ = false;
  std::function<void(CefRefPtr<CefBrowser>, const CefString&)> title_update_callback_;
  MuonWindowDelegate* window_delegate_ = nullptr;
  bool waiting_for_project_number_ = false;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(MuonHandler);
};

#endif  // CEF_TESTS_MUON_MUON_HANDLER_H_

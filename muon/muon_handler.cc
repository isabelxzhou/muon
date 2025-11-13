// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "muon_handler.h"

#include <sstream>
#include <string>

#include "framed_browser_view.h"
#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_keyboard_handler.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "muon_window_delegate.h"

namespace {

MuonHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

MuonHandler::MuonHandler() {
  DCHECK(!g_instance);
  g_instance = this;
}

MuonHandler::~MuonHandler() {
  g_instance = nullptr;
}

// static
MuonHandler* MuonHandler::GetInstance() {
  return g_instance;
}

bool MuonHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                 const CefKeyEvent& event,
                                 CefEventHandle os_event,
                                 bool* is_keyboard_shortcut) {
  CEF_REQUIRE_UI_THREAD();
  
  if (!window_delegate_) {
    return false;
  }

  if (event.type != KEYEVENT_KEYDOWN &&
      event.type != KEYEVENT_RAWKEYDOWN) {
    return false;
  }

  const int key_code = event.windows_key_code;
  const int native_key_code = event.native_key_code;
  const bool has_command = (event.modifiers & EVENTFLAG_COMMAND_DOWN) != 0;
  const char character = event.character;

  auto extract_project_index = [](int win_code, int native_code) -> int {
    if (win_code >= 48 && win_code <= 57) {
      return win_code - 48;
    }
    if (native_code >= 29 && native_code <= 38) {
      return native_code - 29;
    }
    if (win_code >= 112 && win_code <= 121) {
      return win_code - 112;
    }
    return -1;
  };

  if (waiting_for_project_number_) {
    int project_num = extract_project_index(key_code, native_key_code);
    waiting_for_project_number_ = false;
    if (project_num >= 0 && project_num < 10) {
      window_delegate_->SwitchProject(project_num);
      return true;
    }
  }

  const bool is_p_key = (key_code == 80) || (key_code == 112) ||
                        (character == 'p') || (character == 'P');

  if (has_command && is_p_key) {
    waiting_for_project_number_ = true;
    return true;
  }
  
  return false;
}

void MuonHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (title_update_callback_) {
    title_update_callback_(browser, title);
  }

  if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window) {
      window->SetTitle(title);
    }
  } else {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}
void MuonHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& url) {
  CEF_REQUIRE_UI_THREAD();

  // Only reflect top-level navigations.
  if (!frame || !frame->IsMain())
    return;

  if (auto framed = FramedBrowserView::GetForBrowser(browser)) {
    framed->SetURL(url);
  }
}

void MuonHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Sanity-check the configured runtime style.
  CHECK_EQ(CEF_RUNTIME_STYLE_ALLOY, browser->GetHost()->GetRuntimeStyle());

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool MuonHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void MuonHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void MuonHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              ErrorCode errorCode,
                              const CefString& errorText,
                              const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED) {
    return;
  }

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void MuonHandler::ShowMainWindow() {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&MuonHandler::ShowMainWindow, this));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  auto main_browser = browser_list_.front();

  if (auto browser_view = CefBrowserView::GetForBrowser(main_browser)) {
    // Show the window using the Views framework.
    if (auto window = browser_view->GetWindow()) {
      window->Show();
    }
  } else {
    PlatformShowWindow(main_browser);
  }
}

void MuonHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&MuonHandler::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty()) {
    return;
  }

  for (auto& browser : browser_list_) {
    if (browser && browser->GetHost())
      browser->GetHost()->CloseBrowser(force_close);
  }
}

#if !defined(OS_MAC)
void MuonHandler::PlatformShowWindow(CefRefPtr<CefBrowser> browser) {
  NOTIMPLEMENTED();
}
#endif

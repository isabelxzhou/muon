#include <iostream>
#include "include/cef_browser.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_types.h"
#include "include/internal/cef_types_runtime.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_button_delegate.h"
#include "include/views/cef_label_button.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "muon_handler.h"
#include "project_panel.h"
#include "muon_window_delegate.h"

class NullButtonDelegate : public CefButtonDelegate {
 public:
  NullButtonDelegate() = default;

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
  }

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {
  }

  IMPLEMENT_REFCOUNTING(NullButtonDelegate);
};

class MuonBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  explicit MuonBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    return false;
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return CEF_RUNTIME_STYLE_ALLOY;
  }

  IMPLEMENT_REFCOUNTING(MuonBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(MuonBrowserViewDelegate);
};

// todo: global request context object
auto default_browser(CefRefPtr<MuonHandler> handler, std::string url) {
  CefBrowserSettings settings;
  return CefBrowserView::CreateBrowserView(
      handler, url, settings, nullptr, nullptr, new MuonBrowserViewDelegate());
}

MuonWindowDelegate::MuonWindowDelegate(CefRefPtr<MuonHandler> handler,
                                       cef_show_state_t initial_show_state)
    : handler(handler), initial_show_state(initial_show_state) {
  browser_view = default_browser(handler, "https://www.flytre.com");
  browser_view_2 = default_browser(handler, "https://www.isotau.com");
}

void MuonWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
  // 1) Window: horizontal (sidebar + content)
  CefBoxLayoutSettings win_layout;
  win_layout.horizontal = true;
  window->SetToBoxLayout(win_layout);

  auto project_pane = new ProjectPane();                
  auto pane_root = project_pane->root();
  window->AddChildView(pane_root);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(pane_root, 1); 

  CefRefPtr<CefPanel> content = CefPanel::CreatePanel(nullptr);
  window->AddChildView(content);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(content, 20);   

  CefBoxLayoutSettings content_layout;
  content_layout.horizontal = false;
  content->SetToBoxLayout(content_layout);

  auto minibuffer =
      CefLabelButton::CreateLabelButton(new NullButtonDelegate(), "Minibuffer");

  content->AddChildView(browser_view);
  content->AddChildView(browser_view_2);
  content->AddChildView(minibuffer);

  content->GetLayout()->AsBoxLayout()->SetFlexForView(browser_view,   1);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(browser_view_2, 1);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(minibuffer,     0);

  if (initial_show_state != CEF_SHOW_STATE_HIDDEN)
    window->Show();
}

void MuonWindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> /*window*/) {
  browser_view = nullptr;
}

bool MuonWindowDelegate::CanClose(CefRefPtr<CefWindow> /*window*/) {
  CefRefPtr<CefBrowser> browser =
      browser_view ? browser_view->GetBrowser() : nullptr;
  if (browser)
    return browser->GetHost()->TryCloseBrowser();
  return true;
}

CefSize MuonWindowDelegate::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  return CefSize(1920, 1080);
}

cef_show_state_t
MuonWindowDelegate::GetInitialShowState(CefRefPtr<CefWindow> /*window*/) {
  return initial_show_state;
}

cef_runtime_style_t MuonWindowDelegate::GetWindowRuntimeStyle() {
  return CEF_RUNTIME_STYLE_ALLOY;
}

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
#include "include/views/cef_textfield.h"
#include "include/views/cef_textfield_delegate.h"
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

class NavigateButtonDelegate : public CefButtonDelegate {
 public:
  NavigateButtonDelegate(CefRefPtr<CefTextfield> url_field,
                         CefRefPtr<CefBrowserView> target_browser)
      : url_field_(url_field), target_browser_(target_browser) {}

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    if (url_field_ && target_browser_) {
      std::string url = url_field_->GetText().ToString();
      if (!url.empty()) {
        if (url.find("://") == std::string::npos) {
          url = "https://" + url;
        }
        CefRefPtr<CefBrowser> browser = target_browser_->GetBrowser();
        if (browser) {
          browser->GetMainFrame()->LoadURL(url);
        }
      }
    }
  }

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {
  }

 private:
  CefRefPtr<CefTextfield> url_field_;
  CefRefPtr<CefBrowserView> target_browser_;
  IMPLEMENT_REFCOUNTING(NavigateButtonDelegate);
};

class URLTextFieldDelegate : public CefTextfieldDelegate {
 public:
  URLTextFieldDelegate(CefRefPtr<CefBrowserView> target_browser)
      : target_browser_(target_browser) {}

  bool OnKeyEvent(CefRefPtr<CefTextfield> textfield,
                   const CefKeyEvent& event) override {
    bool is_enter = (event.type == KEYEVENT_KEYDOWN) && 
                    ((event.windows_key_code == 13) || 
                     (event.native_key_code == 36) || 
                     (event.native_key_code == 76));
    
    if (is_enter) {
      std::string url = textfield->GetText().ToString();
      if (!url.empty()) {
        if (url.find("://") == std::string::npos) {
          url = "https://" + url;
        }
        CefRefPtr<CefBrowser> browser = target_browser_->GetBrowser();
        if (browser) {
          browser->GetMainFrame()->LoadURL(url);
        }
      }
      return true;
    }
    return false;
  }

 private:
  CefRefPtr<CefBrowserView> target_browser_;
  IMPLEMENT_REFCOUNTING(URLTextFieldDelegate);
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

  project_pane_ = new ProjectPane();                
  auto pane_root = project_pane_->root();
  window->AddChildView(pane_root);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(pane_root, 1); 

  CefRefPtr<CefPanel> content = CefPanel::CreatePanel(nullptr);
  window->AddChildView(content);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(content, 20);   

  CefBoxLayoutSettings content_layout;
  content_layout.horizontal = false;
  content->SetToBoxLayout(content_layout);

  CefRefPtr<CefPanel> url_panel = CefPanel::CreatePanel(nullptr);
  CefBoxLayoutSettings url_layout;
  url_layout.horizontal = true;
  url_layout.between_child_spacing = 8;
  url_panel->SetToBoxLayout(url_layout);

  CefRefPtr<CefTextfield> url_field = CefTextfield::CreateTextfield(
      new URLTextFieldDelegate(browser_view));
  url_field->SetPlaceholderText("Enter URL...");
  
  CefRefPtr<CefLabelButton> navigate_button = CefLabelButton::CreateLabelButton(
      new NavigateButtonDelegate(url_field, browser_view), "→");

  url_panel->AddChildView(url_field);
  url_panel->AddChildView(navigate_button);
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(url_field, 1);
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(navigate_button, 0);

  content->AddChildView(browser_view);
  content->AddChildView(browser_view_2);
  content->AddChildView(url_panel);

  content->GetLayout()->AsBoxLayout()->SetFlexForView(browser_view,   1);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(browser_view_2, 1);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(url_panel,     0);

  handler->SetTitleUpdateCallback([this](CefRefPtr<CefBrowser> browser, const CefString& title) {
    if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
      if (project_pane_) {
        project_pane_->UpdateTabTitle(browser_view, title.ToString());
      }
    }
  });

  std::string title1 = "Flytre";
  std::string title2 = "Isotau";
  if (auto browser1 = browser_view->GetBrowser()) {
    if (auto frame = browser1->GetMainFrame()) {
      std::string url = frame->GetURL().ToString();
      if (!url.empty()) {
        title1 = url;
      }
    }
  }
  if (auto browser2 = browser_view_2->GetBrowser()) {
    if (auto frame = browser2->GetMainFrame()) {
      std::string url = frame->GetURL().ToString();
      if (!url.empty()) {
        title2 = url;
      }
    }
  }

  project_pane_->AddTab(browser_view, title1);
  project_pane_->AddTab(browser_view_2, title2);

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

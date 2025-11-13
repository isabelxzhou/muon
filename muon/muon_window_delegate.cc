#include "muon_window_delegate.h"
#include <vector>
#include "framed_browser_view.h"
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
#include "project_list_panel.h"
#include "project_panel.h"

class NullButtonDelegate : public CefButtonDelegate {
 public:
  NullButtonDelegate() = default;

  void OnButtonPressed(CefRefPtr<CefButton> button) override {}

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {}

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

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {}

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
    bool is_enter =
        (event.type == KEYEVENT_KEYDOWN) &&
        ((event.windows_key_code == 13) || (event.native_key_code == 36) ||
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

class ProjectButtonDelegate : public CefButtonDelegate {
 public:
  ProjectButtonDelegate(int project_id) : project_id_(project_id) {}

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    project_id_++;
  }

 private:
  int project_id_;

  IMPLEMENT_REFCOUNTING(ProjectButtonDelegate);
  DISALLOW_COPY_AND_ASSIGN(ProjectButtonDelegate);
};

MuonWindowDelegate::MuonWindowDelegate(CefRefPtr<MuonHandler> handler,
                                       cef_show_state_t initial_show_state)
    : handler(handler), initial_show_state(initial_show_state) {
  projects = std::vector<CefRefPtr<ProjectPanel>>();
  active_project_idx = 0;
  for (int i = 0; i < 10; i++)
    projects.push_back(new ProjectPanel(i, handler));
}
void MuonWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
  // 1) Window: horizontal (sidebar + content)
  CefBoxLayoutSettings win_layout;
  win_layout.horizontal = true;
  window->SetToBoxLayout(win_layout);

  auto project_pane = new ProjectListPanel(
      [](int id) { return new ProjectButtonDelegate(id); });
  auto pane_root = project_pane->root();
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

  CefRefPtr<CefTextfield> url_field =
      CefTextfield::CreateTextfield(new URLTextFieldDelegate(
          projects[active_project_idx]->top()->browser_view()));
  url_field->SetPlaceholderText("Enter URL...");

  CefRefPtr<CefLabelButton> navigate_button = CefLabelButton::CreateLabelButton(
      new NavigateButtonDelegate(
          url_field, projects[active_project_idx]->top()->browser_view()),
      "→");

  url_panel->AddChildView(url_field);
  url_panel->AddChildView(navigate_button);

  // setting the flex to this to 1 hides the side panel idk why
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(url_field, 1);
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(navigate_button, 0);

  auto minibuffer =
      CefLabelButton::CreateLabelButton(new NullButtonDelegate(), "Minibuffer");

  content->AddChildView(projects[active_project_idx]->root());
  content->AddChildView(url_panel);
  content->AddChildView(minibuffer);

  content->GetLayout()->AsBoxLayout()->SetFlexForView(
      projects[active_project_idx]->root(), 1);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(minibuffer, 0);
  content->GetLayout()->AsBoxLayout()->SetFlexForView(url_panel, 0);
  if (initial_show_state != CEF_SHOW_STATE_HIDDEN)
    window->Show();
}

void MuonWindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> /*window*/) {
  projects = {};
}

bool MuonWindowDelegate::CanClose(CefRefPtr<CefWindow> /*window*/) {
  // CefRefPtr<CefBrowser> browser =
  //   framed_browser_view ? framed_browser_view->browser_view()->GetBrowser() :
  //   nullptr;
  // if (browser)
  //   return browser->GetHost()->TryCloseBrowser();
  return true;
}

CefSize MuonWindowDelegate::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  return CefSize(1920, 1080);
}

cef_show_state_t MuonWindowDelegate::GetInitialShowState(
    CefRefPtr<CefWindow> /*window*/) {
  return initial_show_state;
}

cef_runtime_style_t MuonWindowDelegate::GetWindowRuntimeStyle() {
  return CEF_RUNTIME_STYLE_ALLOY;
}

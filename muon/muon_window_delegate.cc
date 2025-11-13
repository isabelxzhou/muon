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

//todo: fix bug where muon must be force quit if we switch projects!
class NullButtonDelegate : public CefButtonDelegate {
 public:
  NullButtonDelegate() = default;

  void OnButtonPressed(CefRefPtr<CefButton> button) override {}

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {}

  IMPLEMENT_REFCOUNTING(NullButtonDelegate);
};

class NavigateButtonDelegate : public CefButtonDelegate {
 public:
  NavigateButtonDelegate(
      CefRefPtr<CefTextfield> url_field,
      std::function<CefRefPtr<CefBrowserView>()> target_browser_producer)
      : url_field_(url_field),
        target_browser_producer_(target_browser_producer) {}

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    if (url_field_) {
      std::string url = url_field_->GetText().ToString();
      if (!url.empty()) {
        if (url.find("://") == std::string::npos) {
          url = "https://" + url;
        }
        CefRefPtr<CefBrowser> browser =
            target_browser_producer_()->GetBrowser();
        if (browser) {
          browser->GetMainFrame()->LoadURL(url);
        }
      }
    }
  }

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {}

 private:
  CefRefPtr<CefTextfield> url_field_;
  std::function<CefRefPtr<CefBrowserView>()> target_browser_producer_;
  IMPLEMENT_REFCOUNTING(NavigateButtonDelegate);
};

class URLTextFieldDelegate : public CefTextfieldDelegate {
 public:
  URLTextFieldDelegate(
      std::function<CefRefPtr<CefBrowserView>()> target_browser_producer)
      : target_browser_producer_(std::move(target_browser_producer)) {}

  bool OnKeyEvent(CefRefPtr<CefTextfield> textfield,
                  const CefKeyEvent& event) override {
    if (event.type != KEYEVENT_KEYDOWN &&
        event.type != KEYEVENT_RAWKEYDOWN) {
      return false;
    }
    
    // Check for Enter key
    bool is_enter = (event.windows_key_code == 13) || 
                    (event.native_key_code == 36) ||
                    (event.native_key_code == 76);
    
    // Check for Spacebar key (32 is spacebar key code)
    bool is_space = (event.windows_key_code == 32) || 
                    (event.native_key_code == 49);

    if (is_enter || is_space) {
      std::string url = textfield->GetText().ToString();
      if (!url.empty()) {
        if (url.find("://") == std::string::npos) {
          url = "https://" + url;
        }
        CefRefPtr<CefBrowser> browser =
            target_browser_producer_()->GetBrowser();
        if (browser) {
          browser->GetMainFrame()->LoadURL(url);
        }
      }
      return true;
    }
    return false;
  }

 private:
  std::function<CefRefPtr<CefBrowserView>()> target_browser_producer_;
  IMPLEMENT_REFCOUNTING(URLTextFieldDelegate);
};

class ProjectButtonDelegate : public CefButtonDelegate {
 public:
  ProjectButtonDelegate(MuonWindowDelegate* owner, int project_id)
      : owner_(owner), project_id_(project_id) {}

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    owner_->SwitchProject(project_id_);
  }

 private:
  MuonWindowDelegate* owner_;
  int project_id_;

  IMPLEMENT_REFCOUNTING(ProjectButtonDelegate);
  DISALLOW_COPY_AND_ASSIGN(ProjectButtonDelegate);
};

MuonWindowDelegate::MuonWindowDelegate(CefRefPtr<MuonHandler> handler,
                                       cef_show_state_t initial_show_state)
    : handler(handler), initial_show_state(initial_show_state), waiting_for_project_number_(false) {
  projects = std::vector<CefRefPtr<ProjectPanel>>();
  active_project_idx = 0;
  for (int i = 0; i < 10; i++)
    projects.push_back(new ProjectPanel(i, handler));
}
void MuonWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
  handler->SetWindowDelegate(this);
  
  // 1) Window: horizontal (sidebar + content)
  CefBoxLayoutSettings win_layout;
  win_layout.horizontal = true;
  window->SetToBoxLayout(win_layout);

  auto project_pane = new ProjectListPanel(
      [this](int id) { return new ProjectButtonDelegate(this, id); });
  auto pane_root = project_pane->root();
  window->AddChildView(pane_root);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(pane_root, 1);

  content_panel = CefPanel::CreatePanel(nullptr);
  content_panel->SetBackgroundColor(CefColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
  window->AddChildView(content_panel);
  window->GetLayout()->AsBoxLayout()->SetFlexForView(content_panel, 20);

  CefBoxLayoutSettings content_layout;
  content_layout.horizontal = false;
  content_layout.between_child_spacing = 0;
  content_panel->SetToBoxLayout(content_layout);

  CefRefPtr<CefPanel> url_panel = CefPanel::CreatePanel(nullptr);
  CefBoxLayoutSettings url_layout;
  url_layout.horizontal = true;
  url_layout.between_child_spacing = 12;
  url_layout.cross_axis_alignment = CEF_AXIS_ALIGNMENT_CENTER;
  url_layout.inside_border_insets = CefInsets(8, 12, 8, 12);
  url_panel->SetToBoxLayout(url_layout);
  url_panel->SetBackgroundColor(CefColorSetARGB(0xFF, 0xF5, 0xF5, 0xF5));

  CefRefPtr<CefTextfield> url_field =
      CefTextfield::CreateTextfield(new URLTextFieldDelegate([this]() {
        return projects[active_project_idx]->top()->browser_view();
      }));
  url_field->SetPlaceholderText("Enter URL...");

  CefRefPtr<CefLabelButton> navigate_button = CefLabelButton::CreateLabelButton(
      new NavigateButtonDelegate(
          url_field,
          [this]() {
            return projects[active_project_idx]->top()->browser_view();
          }),
      "→");
  navigate_button->SetTextColor(CEF_BUTTON_STATE_NORMAL, CefColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF));
  navigate_button->SetMinimumSize(CefSize(40, 32));

  url_panel->AddChildView(url_field);
  url_panel->AddChildView(navigate_button);

  // setting the flex to this to 1 hides the side panel idk why
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(url_field, 1);
  url_panel->GetLayout()->AsBoxLayout()->SetFlexForView(navigate_button, 0);

  content_panel->AddChildView(
      projects[active_project_idx]->ActivateAndGetRoot());
  content_panel->AddChildView(url_panel);

  content_panel->GetLayout()->AsBoxLayout()->SetFlexForView(
      projects[active_project_idx]->ActivateAndGetRoot(), 1);
  content_panel->GetLayout()->AsBoxLayout()->SetFlexForView(url_panel, 0);
  if (initial_show_state != CEF_SHOW_STATE_HIDDEN)
    window->Show();
}

void MuonWindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> /*window*/) {
  projects.clear();
}

bool MuonWindowDelegate::CanClose(CefRefPtr<CefWindow> /*window*/) {
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
bool MuonWindowDelegate::OnKeyEvent(CefRefPtr<CefWindow> window,
                                     const CefKeyEvent& event) {
  if (event.type != KEYEVENT_KEYDOWN) {
    return false;
  }

  int key_code = event.windows_key_code;
  int native_key_code = event.native_key_code;
  char character = event.character;
  
  bool is_p = (key_code == 80) || (native_key_code == 35) || 
              (character == 'p') || (character == 'P');

  if (waiting_for_project_number_) {
    bool is_number = false;
    int project_num = -1;
    
    if (key_code >= 48 && key_code <= 57) {
      project_num = key_code - 48;
      is_number = true;
    } else if (native_key_code >= 29 && native_key_code <= 38) {
      project_num = native_key_code - 29;
      is_number = true;
    } else if (character >= '0' && character <= '9') {
      project_num = character - '0';
      is_number = true;
    }
    
    if (is_number && project_num >= 0 && project_num < 10) {
      SwitchProject(project_num);
      waiting_for_project_number_ = false;
      return true;
    }
    waiting_for_project_number_ = false;
  }

  if (is_p) {
    waiting_for_project_number_ = true;
    return true;
  }

  return false;
}

void MuonWindowDelegate::SwitchProject(int id) {
  CefRefPtr<CefPanel> content = content_panel;

  content->RemoveChildView(projects[active_project_idx]->ActivateAndGetRoot());

  active_project_idx = id;

  auto new_root = projects[active_project_idx]->ActivateAndGetRoot();
  content->AddChildViewAt(new_root, 0);

  content_panel->GetLayout()->AsBoxLayout()->SetFlexForView(
      new_root, 1);
  content->Layout();
}

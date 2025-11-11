#include "project_panel.h"

#include "include/views/cef_box_layout.h"
#include "include/views/cef_label_button.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_button_delegate.h"

class TabButtonDelegate : public CefButtonDelegate {
 public:
  TabButtonDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnButtonPressed(CefRefPtr<CefButton> button) override {
    if (browser_view_) {
      browser_view_->RequestFocus();
    }
  }

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  IMPLEMENT_REFCOUNTING(TabButtonDelegate);
};

ProjectPane::ProjectPane() {
  panel_ = CefPanel::CreatePanel(nullptr);
  
  CefBoxLayoutSettings layout_settings;
  layout_settings.horizontal = false;
  panel_->SetToBoxLayout(layout_settings);
}

void ProjectPane::AddTab(CefRefPtr<CefBrowserView> browser_view, const std::string& initial_title) {
  if (!browser_view) return;
  
  CefRefPtr<CefLabelButton> tab_button = CefLabelButton::CreateLabelButton(
      new TabButtonDelegate(browser_view), initial_title);
  
  tab_buttons_[browser_view] = tab_button;
  panel_->AddChildView(tab_button);
  panel_->GetLayout()->AsBoxLayout()->SetFlexForView(tab_button, 0);
}

void ProjectPane::UpdateTabTitle(CefRefPtr<CefBrowserView> browser_view, const std::string& title) {
  auto it = tab_buttons_.find(browser_view);
  if (it != tab_buttons_.end()) {
    it->second->SetText(title);
  }
}


#pragma once

#include <vector>

#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_button_delegate.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"

class ProjectListPanel : public CefPanelDelegate {
 public:
  using DelegateFactory = std::function<CefRefPtr<CefButtonDelegate>(int)>;

  explicit ProjectListPanel(DelegateFactory delegate_factory);


  CefRefPtr<CefPanel> root() const { return root_; }

  // Fixed 50px width; flexible height.
  CefSize GetPreferredSize(CefRefPtr<CefView>) override;

  // Cannot set bg colors without using this hook
  void OnThemeChanged(CefRefPtr<CefView> view) override;

 private:
  void BuildPanels(CefRefPtr<CefBoxLayout> layout);
  void ApplyColors();

  CefRefPtr<CefPanel> root_;
  std::vector<CefRefPtr<CefPanel>> panels_;
  DelegateFactory delegate_factory_;

  IMPLEMENT_REFCOUNTING(ProjectListPanel);
  DISALLOW_COPY_AND_ASSIGN(ProjectListPanel);
};

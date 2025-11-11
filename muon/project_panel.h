#pragma once

#include <vector>

#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"

class ProjectPanel : public CefPanelDelegate {
 public:
  ProjectPanel();

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

  IMPLEMENT_REFCOUNTING(ProjectPanel);
  DISALLOW_COPY_AND_ASSIGN(ProjectPanel);
};

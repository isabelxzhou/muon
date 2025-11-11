#pragma once

#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/internal/cef_types.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"

class ProjectPanel : public CefPanelDelegate {
 public:
  ProjectPanel();

  CefRefPtr<CefPanel> root() const { return root_; }

  // Fixed 50px width; flexible height.
  CefSize GetPreferredSize(CefRefPtr<CefView>) override;

  // Re-apply colors when the system/app theme changes.
  void OnThemeChanged(CefRefPtr<CefView> view) override;

 private:
  void ApplyColors();

  cef_color_t root_color_, top_color_, bottom_color_;
  CefRefPtr<CefPanel> root_, top_, bottom_;

  IMPLEMENT_REFCOUNTING(ProjectPanel);
  DISALLOW_COPY_AND_ASSIGN(ProjectPanel);
};

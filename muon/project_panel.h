#pragma once

#include "framed_browser_view.h"
#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"

class ProjectPanel : public CefPanelDelegate {
 public:
  ProjectPanel(int num, CefRefPtr<MuonHandler> handler);

  CefRefPtr<CefPanel> ActivateAndGetRoot();

  CefRefPtr<FramedBrowserView> top() const { return top_; }

  CefSize GetPreferredSize(CefRefPtr<CefView>) override;

 private:
  void LazyCreate();
  void BuildLayout();

  int num_;
  CefRefPtr<MuonHandler> handler_;

  bool created_ = false;  // <-- important

  CefRefPtr<CefPanel> root_;
  CefRefPtr<FramedBrowserView> top_;
  CefRefPtr<FramedBrowserView> bottom_;

  IMPLEMENT_REFCOUNTING(ProjectPanel);
  DISALLOW_COPY_AND_ASSIGN(ProjectPanel);
};

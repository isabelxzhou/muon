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

  CefRefPtr<CefPanel> root() const { return root_; }
  CefRefPtr<FramedBrowserView> top() const {return top_;}

  // Fixed 50px width; flexible height.
  CefSize GetPreferredSize(CefRefPtr<CefView>) override;


  void close_browsers();
 private:
  void BuildLayout();

  CefRefPtr<CefPanel> root_;
  CefRefPtr<FramedBrowserView> top_;
  CefRefPtr<FramedBrowserView> bottom_;

  IMPLEMENT_REFCOUNTING(ProjectPanel);
  DISALLOW_COPY_AND_ASSIGN(ProjectPanel);
};

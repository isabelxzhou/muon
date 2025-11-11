#pragma once

#include <string>

#include "include/base/cef_macros.h"
#include "include/cef_base.h"
#include "include/views/cef_box_layout.h"
#include "include/views/cef_label_button.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"
#include "include/views/cef_browser_view.h"
#include "muon_handler.h"

class FramedBrowserView : public CefPanelDelegate {
 public:
  FramedBrowserView(CefRefPtr<MuonHandler> handler, std::string url);

  CefRefPtr<CefPanel> root() const { return root_; }
  CefRefPtr<CefBrowserView> browser_view() const { return browser_view_; }

  void SetWindowNumber(int number);
  void SetURL(const CefString& url_text);

  CefSize GetPreferredSize(CefRefPtr<CefView>) override;

  static CefRefPtr<FramedBrowserView> GetForBrowser(CefRefPtr<CefBrowser> browser);
   private:
    void BuildLayout();
    void ApplyInitialStyles();

    CefRefPtr<CefPanel> root_;
    CefRefPtr<CefBrowserView> browser_view_;
    CefRefPtr<CefPanel> status_bar_;
    CefRefPtr<CefLabelButton> window_num_label_;
    CefRefPtr<CefLabelButton> url_label_;

    IMPLEMENT_REFCOUNTING(FramedBrowserView);
    DISALLOW_COPY_AND_ASSIGN(FramedBrowserView);
  };

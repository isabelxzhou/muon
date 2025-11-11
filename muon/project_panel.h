#ifndef MUON_PROJECT_PANEL_H_
#define MUON_PROJECT_PANEL_H_

#include <map>
#include <string>
#include "include/internal/cef_ptr.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_label_button.h"
#include "include/views/cef_panel.h"
#include "include/views/cef_view.h"

// ProjectPane creates a sidebar panel for the muon application.
class ProjectPane {
 public:
  ProjectPane();
  
  // Returns the root view of the project pane.
  CefRefPtr<CefView> root() { return panel_; }
  
  // Add a tab for a browser view
  void AddTab(CefRefPtr<CefBrowserView> browser_view, const std::string& initial_title);
  
  // Update the title of a tab
  void UpdateTabTitle(CefRefPtr<CefBrowserView> browser_view, const std::string& title);

 private:
  CefRefPtr<CefPanel> panel_;
  std::map<CefRefPtr<CefBrowserView>, CefRefPtr<CefLabelButton>> tab_buttons_;
};

#endif  // MUON_PROJECT_PANEL_H_


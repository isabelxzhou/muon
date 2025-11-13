#include "project_panel.h"

#include "framed_browser_view.h"
#include "include/internal/cef_types_wrappers.h"  // for CefColorSetARGB
#include "muon_handler.h"


const std::vector<std::string> sites = {
    "https://flytre.com",  // 0
    "https://isotau.com",  // 1

    "https://example.com",           // 2
    "https://wikipedia.org",         // 3
    "https://github.com",            // 4
    "https://stackoverflow.com",     // 5
    "https://news.ycombinator.com",  // 6
    "https://reddit.com",            // 7
    "https://openai.com",            // 8
    "https://mozilla.org",           // 9
    "https://kernel.org",            // 10
    "https://gnu.org",               // 11
    "https://archlinux.org",         // 12
    "https://debian.org",            // 13
    "https://ubuntu.com",            // 14
    "https://godotengine.org",       // 15
    "https://llvm.org",              // 16
    "https://cmake.org",             // 17
    "https://nodejs.org",            // 18
    "https://python.org"             // 19
};

ProjectPanel::ProjectPanel(int num, CefRefPtr<MuonHandler> handler) {
  root_ = CefPanel::CreatePanel(this);
  top_ = new FramedBrowserView(handler, sites[(num * 2) % 20]);
  bottom_ = new FramedBrowserView(handler, sites[(num * 2 + 1) % 20]);
  root_->SetSize(CefSize(1000, 1000));
  BuildLayout();
}

CefSize ProjectPanel::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  return CefSize(50, 0);
}


void ProjectPanel::BuildLayout() {
  CefBoxLayoutSettings root_layout;
  root_layout.horizontal = false;
  root_layout.cross_axis_alignment = CEF_AXIS_ALIGNMENT_STRETCH;
  auto root_box = root_->SetToBoxLayout(root_layout);

  auto top_root = top_->root();
  auto bottom_root = bottom_->root();
  top_->SetWindowNumber(1);
  bottom_->SetWindowNumber(2);
  root_->AddChildView(top_root);
  root_->AddChildView(bottom_root);
  root_->GetLayout()->AsBoxLayout()->SetFlexForView(top_root, 1);
  root_->GetLayout()->AsBoxLayout()->SetFlexForView(bottom_root, 1);
}

void ProjectPanel::close_browsers() {
  top_->browser_view()->GetBrowser()->GetHost()->TryCloseBrowser();
  bottom_->browser_view()->GetBrowser()->GetHost()->TryCloseBrowser();
}

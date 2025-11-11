#include "project_panel.h"
#include "include/views/cef_button.h"
#include "include/views/cef_button_delegate.h"
namespace {
class NullButtonDelegate : public CefButtonDelegate {
 public:
  NullButtonDelegate() = default;
  void OnButtonPressed(CefRefPtr<CefButton> /*button*/) override {}
  void OnButtonStateChanged(CefRefPtr<CefButton> /*button*/) override {}
  IMPLEMENT_REFCOUNTING(NullButtonDelegate);
};
}  // namespace

ProjectPanel::ProjectPanel()
    : root_color_(CefColorSetARGB(0xFF, 0x33, 0x33, 0x33)),
      top_color_(CefColorSetARGB(0xFF, 0x42, 0x85, 0xF4)),
      bottom_color_(CefColorSetARGB(0xFF, 0xEA, 0x43, 0x35)) {
  root_ = CefPanel::CreatePanel(this);

  CefBoxLayoutSettings s;
  s.horizontal = false;
  s.cross_axis_alignment = CEF_AXIS_ALIGNMENT_STRETCH;
  auto layout = root_->SetToBoxLayout(s);

  top_ = CefPanel::CreatePanel(nullptr);
  bottom_ = CefPanel::CreatePanel(nullptr);

  root_->AddChildView(top_);
  root_->AddChildView(bottom_);
  layout->SetFlexForView(top_, 1);
  layout->SetFlexForView(bottom_, 1);

  ApplyColors();  // initial paint
}

CefSize ProjectPanel::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  return CefSize(50, 0);
}

void ProjectPanel::OnThemeChanged(CefRefPtr<CefView> /*view*/) {
  ApplyColors();
}

void ProjectPanel::ApplyColors() {
  root_->SetBackgroundColor(root_color_);
  top_->SetBackgroundColor(top_color_);
  bottom_->SetBackgroundColor(bottom_color_);
}

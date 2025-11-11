#include "project_panel.h"

#include "include/internal/cef_types_wrappers.h"  // for CefColorSetARGB

namespace {

static constexpr cef_color_t kRainbow[10] = {
    CefColorSetARGB(0xFF, 0xE5, 0x39, 0x35),  // red
    CefColorSetARGB(0xFF, 0xFB, 0x8C, 0x00),  // orange
    CefColorSetARGB(0xFF, 0xFF, 0xEB, 0x3B),  // yellow
    CefColorSetARGB(0xFF, 0x43, 0xA0, 0x47),  // green
    CefColorSetARGB(0xFF, 0x00, 0x96, 0x88),  // teal
    CefColorSetARGB(0xFF, 0x00, 0xBC, 0xD4),  // cyan
    CefColorSetARGB(0xFF, 0x1E, 0x88, 0xE5),  // blue
    CefColorSetARGB(0xFF, 0x3F, 0x51, 0xB5),  // indigo
    CefColorSetARGB(0xFF, 0x8E, 0x24, 0xAA),  // violet
    CefColorSetARGB(0xFF, 0xD8, 0x1B, 0x60),  // magenta
};

inline cef_color_t RainbowColor(int i) {
  return kRainbow[i % 10];
}

}  // namespace

ProjectPanel::ProjectPanel() {
  // Root container.
  root_ = CefPanel::CreatePanel(this);

  CefBoxLayoutSettings s;
  s.horizontal = false;
  s.cross_axis_alignment = CEF_AXIS_ALIGNMENT_STRETCH;

  auto layout = root_->SetToBoxLayout(s);

  BuildPanels(layout);
  root_->SetSize(CefSize(20, 20));
  ApplyColors();
}

CefSize ProjectPanel::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  return CefSize(50, 0);
}

void ProjectPanel::OnThemeChanged(CefRefPtr<CefView> /*view*/) {
  ApplyColors();
}

void ProjectPanel::BuildPanels(CefRefPtr<CefBoxLayout> layout) {
  panels_.reserve(10);
  for (int i = 0; i < 10; ++i) {
    auto panel = CefPanel::CreatePanel(nullptr);
    panels_.push_back(panel);
    root_->AddChildView(panel);
    layout->SetFlexForView(panel, 1);  
  }
}

void ProjectPanel::ApplyColors() {
  for (int i = 0; i < static_cast<int>(panels_.size()); ++i) {
    panels_[i]->SetBackgroundColor(RainbowColor(i));
  }
}

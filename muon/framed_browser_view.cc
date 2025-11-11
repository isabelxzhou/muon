#include "framed_browser_view.h"

#include "include/internal/cef_types.h"
#include "include/internal/cef_types_wrappers.h"  // CefColorSetARGB
#include "muon_handler.h"


namespace {
std::mutex g_registry_mutex;
std::unordered_map<int, FramedBrowserView*> g_browser_registry;

inline void RegisterFramedView(int id, FramedBrowserView* view) {
  std::lock_guard<std::mutex> lock(g_registry_mutex);
  g_browser_registry[id] = view;
}

inline void UnregisterFramedView(int id) {
  std::lock_guard<std::mutex> lock(g_registry_mutex);
  g_browser_registry.erase(id);
}

inline FramedBrowserView* LookupFramedView(int id) {
  std::lock_guard<std::mutex> lock(g_registry_mutex);
  auto it = g_browser_registry.find(id);
  return (it == g_browser_registry.end()) ? nullptr : it->second;
}
}  // namespace

// ---------- Delegate ----------
class FramedBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  explicit FramedBrowserViewDelegate(FramedBrowserView* owner) : owner_(owner) {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    return false;
  }

  void OnBrowserCreated(CefRefPtr<CefBrowserView> browser_view,
                        CefRefPtr<CefBrowser> browser) override {
    if (browser) {
      RegisterFramedView(browser->GetIdentifier(), owner_);
    }
  }

  void OnBrowserDestroyed(CefRefPtr<CefBrowserView> browser_view,
                          CefRefPtr<CefBrowser> browser) override {
    if (browser) {
      UnregisterFramedView(browser->GetIdentifier());
    }
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return CEF_RUNTIME_STYLE_ALLOY;
  }

  IMPLEMENT_REFCOUNTING(FramedBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(FramedBrowserViewDelegate);

 private:
  FramedBrowserView* owner_;  // raw back-pointer; owner controls lifetime
};


// todo: global request context object
auto CreateBrowserView(CefRefPtr<MuonHandler> handler, std::string url, FramedBrowserView* owner) {
  CefBrowserSettings settings;
  return CefBrowserView::CreateBrowserView(
      handler, url, settings, nullptr, nullptr, new FramedBrowserViewDelegate(owner));
}


class NullButtonDelegate : public CefButtonDelegate {
 public:
  NullButtonDelegate() = default;

  void OnButtonPressed(CefRefPtr<CefButton> button) override {}

  void OnButtonStateChanged(CefRefPtr<CefButton> button) override {}

  IMPLEMENT_REFCOUNTING(NullButtonDelegate);
};

FramedBrowserView::FramedBrowserView(CefRefPtr<MuonHandler> handler, std::string url) {
  browser_view_ = CreateBrowserView(handler, url, this);
  root_ = CefPanel::CreatePanel(this);
  BuildLayout();
  ApplyInitialStyles();
}

void FramedBrowserView::BuildLayout() {
  CefBoxLayoutSettings root_layout;
  root_layout.horizontal = false;
  root_layout.cross_axis_alignment = CEF_AXIS_ALIGNMENT_STRETCH;
  auto root_box = root_->SetToBoxLayout(root_layout);

  root_->AddChildView(browser_view_);
  root_box->SetFlexForView(browser_view_, 1);

  status_bar_ = CefPanel::CreatePanel(nullptr);
  CefBoxLayoutSettings sb_layout;
  sb_layout.horizontal = true;
  sb_layout.between_child_spacing = 8;
  sb_layout.cross_axis_alignment = CEF_AXIS_ALIGNMENT_CENTER;
  sb_layout.inside_border_insets = CefInsets(6, 4, 6, 4);
  auto sb_box = status_bar_->SetToBoxLayout(sb_layout);

  window_num_label_ =
      CefLabelButton::CreateLabelButton(new NullButtonDelegate(), "");
  url_label_ =
      CefLabelButton::CreateLabelButton(new NullButtonDelegate(), "");
  // Let URL stretch; window number stays compact.
  status_bar_->AddChildView(window_num_label_);
  status_bar_->AddChildView(url_label_);
  sb_box->SetFlexForView(window_num_label_, 0);
  sb_box->SetFlexForView(url_label_, 1);

  // Add status bar to root (minimal height).
  root_->AddChildView(status_bar_);
  root_box->SetFlexForView(status_bar_, 0);
}

void FramedBrowserView::ApplyInitialStyles() {
  // Neutral dark root, subtle status bar background; tweak as desired.
  root_->SetBackgroundColor(CefColorSetARGB(0xFF, 0x20, 0x20, 0x20));
  status_bar_->SetBackgroundColor(CefColorSetARGB(0xFF, 0x2A, 0x2A, 0x2A));

  window_num_label_->SetTextColor(CEF_BUTTON_STATE_NORMAL,
                                  CefColorSetARGB(0xFF, 0xDD, 0xDD, 0xDD));
  url_label_->SetTextColor(CEF_BUTTON_STATE_NORMAL,
                           CefColorSetARGB(0xFF, 0xBB, 0xBB, 0xBB));

  window_num_label_->SetFontList("monospace, 12px");
  url_label_->SetFontList("monospace, 12px");

  // Sensible defaults.
  SetWindowNumber(1);
  SetURL("about:blank");
}

void FramedBrowserView::SetWindowNumber(int number) {
  window_num_label_->SetText("[" + std::to_string(number) + "]");
}

void FramedBrowserView::SetURL(const CefString& url_text) {
  url_label_->SetText(url_text);
}

CefSize FramedBrowserView::GetPreferredSize(CefRefPtr<CefView> /*view*/) {
  // Fixed 50px width (to match your project panels) — adjust if you want.
  return CefSize(50, 0);
}

CefRefPtr<FramedBrowserView> FramedBrowserView::GetForBrowser(
    CefRefPtr<CefBrowser> browser) {
  if (!browser)
    return nullptr;
  auto* raw = LookupFramedView(browser->GetIdentifier());
  return raw ? CefRefPtr<FramedBrowserView>(raw) : nullptr;
}

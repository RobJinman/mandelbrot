#include "params_page.hpp"
#include "utils.hpp"
#include "wx_helpers.hpp"
#include "defaults.hpp"

using std::string;

static const long MIN_ITERATIONS = 1;
static const long MAX_ITERATIONS = 10000;
static const double MIN_ZOOM_AMOUNT = 1.0;
static const double MAX_ZOOM_AMOUNT = 1000.0;
static const double MIN_TARGET_FPS = 0.1;
static const double MAX_TARGET_FPS = 60.0;
static const double MIN_ZOOM_PER_FRAME = 0.0001;
static const double MAX_ZOOM_PER_FRAME = 10.0;

wxDEFINE_EVENT(APPLY_PARAMS_EVENT, ApplyParamsEvent);

ApplyParamsEvent::ApplyParamsEvent(int maxI, double zoomAmount,
                                   double targetFps, double zoomPerFrame)
  : wxCommandEvent(APPLY_PARAMS_EVENT),
    maxI(maxI),
    zoomAmount(zoomAmount),
    targetFps(targetFps),
    zoomPerFrame(zoomPerFrame) {}

ApplyParamsEvent::ApplyParamsEvent(const ApplyParamsEvent& cpy)
  : wxCommandEvent(cpy),
    maxI(cpy.maxI),
    zoomAmount(cpy.zoomAmount),
    targetFps(cpy.targetFps),
    zoomPerFrame(cpy.zoomPerFrame) {}

wxEvent* ApplyParamsEvent::Clone() const {
  return new ApplyParamsEvent(*this);
}

ParamsPage::ParamsPage(wxWindow* parent)
  : wxNotebookPage(parent, wxID_ANY) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructRenderParamsPanel(this), 1, wxEXPAND | wxALL, 10);
  vbox->Add(constructFlyThroughParamsPanel(this), 1,
            wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

  m_btnApply = new wxButton(this, wxID_ANY, wxGetTranslation("Apply"));
  m_btnApply->Bind(wxEVT_BUTTON, &ParamsPage::onApplyParamsClick, this);

  vbox->Add(m_btnApply, 0, wxALIGN_RIGHT | wxALL, 10);

  SetSizer(vbox);
}

wxStaticBoxSizer* ParamsPage::constructRenderParamsPanel(wxWindow* parent) {
  auto boxSizer = new wxStaticBoxSizer(wxVERTICAL, parent,
                                       wxGetTranslation("General"));
  auto box = boxSizer->GetStaticBox();

  auto grid = new wxFlexGridSizer(2);
  boxSizer->Add(grid, 1, wxEXPAND);

  auto lblMaxI = constructLabel(box, wxGetTranslation("Max iterations"));
  string strMaxI = numberToString(DEFAULT_MAX_ITERATIONS, false);
  m_txtMaxIterations = constructTextBox(box, strMaxI);
  m_txtMaxIterations->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  auto lblZoomAmount = constructLabel(box,
                                      wxGetTranslation("Zoom amount"));
  m_txtZoomAmount = constructTextBox(box, numberToString(DEFAULT_ZOOM, false));
  m_txtZoomAmount->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblMaxI, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtMaxIterations, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);
  grid->Add(lblZoomAmount, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtZoomAmount, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);

  grid->AddGrowableCol(0);

  return boxSizer;
}

wxStaticBoxSizer* ParamsPage::constructFlyThroughParamsPanel(wxWindow* parent) {
  auto boxSizer = new wxStaticBoxSizer(wxVERTICAL, parent,
                                       wxGetTranslation("Fly-Through Mode"));
  auto box = boxSizer->GetStaticBox();

  auto grid = new wxFlexGridSizer(2);
  boxSizer->Add(grid, 1, wxEXPAND);

  auto strFps = numberToString(DEFAULT_TARGET_FPS, false);
  auto lblFps = constructLabel(box, wxGetTranslation("Target frame rate"));
  m_txtTargetFps = constructTextBox(box, strFps);
  m_txtTargetFps->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  auto strZoom = numberToString(DEFAULT_ZOOM_PER_FRAME, false);
  auto lblZoom = constructLabel(box, wxGetTranslation("Zoom per frame"));
  m_txtZoomPerFrame = constructTextBox(box, strZoom);
  m_txtZoomPerFrame->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblFps, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtTargetFps, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);
  grid->Add(lblZoom, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtZoomPerFrame, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);

  grid->AddGrowableCol(0);

  return boxSizer;
}

void ParamsPage::onApplyParamsClick(wxCommandEvent&) {
  long maxI = getBoundedValue<long>(*m_txtMaxIterations, MIN_ITERATIONS,
                                    MAX_ITERATIONS);

  double zoomAmount = getBoundedValue<double>(*m_txtZoomAmount,
                                              MIN_ZOOM_AMOUNT,
                                              MAX_ZOOM_AMOUNT);

  double targetFps = getBoundedValue<double>(*m_txtTargetFps,
                                             MIN_TARGET_FPS,
                                             MAX_TARGET_FPS);

  double zoomPerFrame = getBoundedValue<double>(*m_txtZoomPerFrame,
                                                MIN_ZOOM_PER_FRAME,
                                                MAX_ZOOM_PER_FRAME);

  ApplyParamsEvent event(maxI, zoomAmount, targetFps, zoomPerFrame);
  wxPostEvent(this, event);
}

void ParamsPage::disable() {
  m_btnApply->Disable();
}

void ParamsPage::enable() {
  m_btnApply->Enable();
}

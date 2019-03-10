#include <sstream>
#include <iomanip>
#include <wx/gbsizer.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/xml/xml.h>
#include "main_window.hpp"
#include "config.hpp"
#include "exception.hpp"
#include "wx_helpers.hpp"
#include "utils.hpp"

using std::string;

static const long DEFAULT_EXPORT_HEIGHT = 1000;
static const long MIN_EXPORT_WIDTH = 10;
static const long MAX_EXPORT_WIDTH = 10000;
static const long MIN_EXPORT_HEIGHT = 10;
static const long MAX_EXPORT_HEIGHT = 10000;
static const long MIN_ITERATIONS = 1;
static const long MAX_ITERATIONS = 10000;
static const double MIN_ZOOM_AMOUNT = 1.0;
static const double MAX_ZOOM_AMOUNT = 1000.0;
static const double MIN_TARGET_FPS = 0.1;
static const double MAX_TARGET_FPS = 60.0;
static const double MIN_ZOOM_PER_FRAME = 0.0001;
static const double MAX_ZOOM_PER_FRAME = 10.0;

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(wxID_EXIT, MainWindow::onExit)
  EVT_MENU(wxID_ABOUT, MainWindow::onAbout)
  EVT_CLOSE(MainWindow::onClose)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {

  m_renderer.reset(new Renderer([this]() {
    makeGlContextCurrent();
  }));

  m_vbox = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_vbox);

  SetAutoLayout(true);

  m_splitter = new wxSplitterWindow(this);
  m_splitter->SetMinimumPaneSize(300);

  m_vbox->Add(m_splitter, 1, wxEXPAND, 0);

  constructMenu();
  constructLeftPanel();
  constructRightPanel();

  m_splitter->SplitVertically(m_leftPanel, m_rightPanel, 0.55 * WINDOW_W);

  CreateStatusBar();
  SetStatusText(wxEmptyString);
}

void MainWindow::onClose(wxCloseEvent& e) {
  m_quitting = true;

  if (m_export.busy) {
    e.Veto();
  }
  else {
    e.Skip();
  }
}

void MainWindow::makeGlContextCurrent() {
  if (m_canvas) {
    m_canvas->makeGlContextCurrent();
  }
}

void MainWindow::constructLeftPanel() {
  m_leftPanel = new wxPanel(m_splitter, wxID_ANY);
  auto vbox = new wxBoxSizer(wxVERTICAL);
  m_leftPanel->SetSizer(vbox);
  m_leftPanel->SetCanFocus(true);

  wxGLAttributes glAttrs;
  glAttrs.PlatformDefaults().Defaults().EndList();

  m_canvas = new Canvas(m_leftPanel, glAttrs, *m_renderer,
                        [this]() { onRender(); });
  m_canvas->Bind(FLY_THROUGH_MODE_TOGGLED, &MainWindow::onFlyThroughModeToggle,
                 this);
  m_canvas->Bind(wxEVT_SIZE, &MainWindow::onCanvasResize, this);
  m_canvas->Bind(wxEVT_SET_FOCUS, &MainWindow::onCanvasGainFocus, this);
  m_canvas->Bind(wxEVT_KILL_FOCUS, &MainWindow::onCanvasLoseFocus, this);

  vbox->Add(m_canvas, 1, wxEXPAND | wxALL, 5);
}

wxStaticBox* MainWindow::constructInfoPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_NORMAL);

  auto txtInfo = new wxRichTextCtrl(box, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxVSCROLL | wxBORDER_NONE | wxWANTS_CHARS);

  txtInfo->SetFont(font);

  auto addHeading = [txtInfo](const wxString& text) {
    txtInfo->BeginBold();
    txtInfo->BeginUnderline();
    txtInfo->WriteText(wxGetTranslation(text));
    txtInfo->EndUnderline();
    txtInfo->EndBold();
    txtInfo->Newline();
  };

  auto addText = [txtInfo](const wxString& text, bool translate = true,
                           bool newline = true, bool bold = false) {
    if (bold) {
      txtInfo->BeginBold();
    }
    txtInfo->WriteText(translate ? wxGetTranslation(text) : text);
    if (newline) {
      txtInfo->Newline();
    }
    if (bold) {
      txtInfo->EndBold();
    }
  };

  txtInfo->SetEditable(false);
  txtInfo->SetMinSize(wxSize(0, 80));

  addHeading(versionString());
  addText("The Mandelbrot fractal rendered on the GPU.");
  txtInfo->Newline();
  addText("Click and drag the canvas to zoom.");
  txtInfo->Newline();
  addHeading(wxGetTranslation("Controls"));
  addText("R   ", false, false, true);
  addText("Reset view");
  addText("Z   ", false, false, true);
  addText("Toggle Fly-Through mode");
  addText("I   ", false, false, true);
  addText("Zoom in");
  addText("O   ", false, false, true);
  addText("Zoom out");

  vbox->AddSpacer(10);
  vbox->Add(txtInfo, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructColourSchemePanel(wxWindow* parent) {
  loadColourSchemes();

  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  wxFont codeFontNormal(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  wxFont codeFontBold(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_BOLD);

  auto txtShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 computeColour(int i, float x, float y) {");
  txtShaderCodePre->SetFont(codeFontBold);

  m_colourScheme.cboSelector = new wxComboBox(box, wxID_ANY);
  std::vector<wxString> names;
  for (auto entry : m_colourScheme.colourSchemes) {
    names.push_back(entry.first);
  }
  m_colourScheme.cboSelector->Insert(names, 0);
  m_colourScheme.cboSelector->SetStringSelection(DEFAULT_COLOUR_SCHEME);
  m_colourScheme.cboSelector->Bind(wxEVT_COMBOBOX,
                                   &MainWindow::onSelectColourScheme, this);
  m_colourScheme.cboSelector->Bind(wxEVT_TEXT,
                                   &MainWindow::onColourSchemeNameChange, this);

  m_colourScheme.txtCode = constructTextBox(box,
                                            PRESETS.at(DEFAULT_COLOUR_SCHEME),
                                            true, false, true);
  m_colourScheme.txtCode->SetFont(codeFontNormal);

  auto txtShaderCodePost = new wxStaticText(box, wxID_ANY, "}");
  txtShaderCodePost->SetFont(codeFontBold);

  m_colourScheme.txtCompileStatus = constructTextBox(box, wxEmptyString, true,
                                                     true, true);
  m_colourScheme.txtCompileStatus->SetFont(codeFontNormal);
  m_colourScheme.txtCompileStatus->SetEditable(false);
  m_colourScheme.txtCompileStatus->SetMinSize(wxSize(0, 80));

  m_colourScheme.btnDelete = new wxButton(box, wxID_ANY,
                                          wxGetTranslation("Delete"));
  m_colourScheme.btnDelete->Bind(wxEVT_BUTTON,
                                 &MainWindow::onDeleteColourSchemeClick,
                                 this);
  m_colourScheme.btnDelete->Hide();

  m_colourScheme.btnRestore = new wxButton(box, wxID_ANY,
                                           wxGetTranslation("Restore"));
  m_colourScheme.btnRestore->Bind(wxEVT_BUTTON,
                                  &MainWindow::onRestoreColourSchemeClick,
                                  this);

  m_colourScheme.btnSave = new wxButton(box, wxID_ANY,
                                        wxGetTranslation("Save"));
  m_colourScheme.btnSave->Bind(wxEVT_BUTTON,
                               &MainWindow::onSaveColourSchemeClick, this);

  wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
  hbox->AddStretchSpacer(1);
  hbox->Add(m_colourScheme.btnDelete, 0, wxRIGHT, 5);
  hbox->Add(m_colourScheme.btnRestore, 0, wxRIGHT, 5);
  hbox->Add(m_colourScheme.btnSave, 0, wxRIGHT, 0);

  vbox->AddSpacer(10);
  vbox->Add(m_colourScheme.cboSelector, 0,
            wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  vbox->Add(txtShaderCodePre, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(m_colourScheme.txtCode, 2, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(txtShaderCodePost, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(m_colourScheme.txtCompileStatus, 1, wxEXPAND | wxLEFT | wxRIGHT,
            10);

  return box;
}

wxStaticBox* MainWindow::constructRenderParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, "");

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto lblMaxI = constructLabel(box, wxGetTranslation("Max iterations"));
  string strMaxI = std::to_string(DEFAULT_MAX_ITERATIONS);
  m_params.txtMaxIterations = constructTextBox(box, strMaxI);
  m_params.txtMaxIterations->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  auto lblZoomAmount = constructLabel(box,
                                      wxGetTranslation("Zoom amount"));
  m_params.txtZoomAmount = constructTextBox(box, std::to_string(DEFAULT_ZOOM));
  m_params.txtZoomAmount->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblMaxI, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_params.txtMaxIterations, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblZoomAmount, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_params.txtZoomAmount, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

  return box;
}

wxStaticBox* MainWindow::constructFlyThroughParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Fly-Through Mode"));

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto strFps = std::to_string(DEFAULT_TARGET_FPS);
  auto lblFps = constructLabel(box, wxGetTranslation("Target frame rate"));
  m_params.txtTargetFps = constructTextBox(box, strFps);
  m_params.txtTargetFps->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  auto strZoom = std::to_string(DEFAULT_ZOOM_PER_FRAME);
  auto lblZoom = constructLabel(box, wxGetTranslation("Zoom per frame"));
  m_params.txtZoomPerFrame = constructTextBox(box, strZoom);
  m_params.txtZoomPerFrame->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblFps, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_params.txtTargetFps, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblZoom, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_params.txtZoomPerFrame, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

  return box;
}

wxStaticBox* MainWindow::constructDataPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Data"));

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto lblMagLevel = constructLabel(box, wxGetTranslation("Magnification"));
  m_dataFields.txtMagLevel = constructLabel(box, "");

  auto lblXMin = constructLabel(box, "x-min");
  m_dataFields.txtXMin = constructLabel(box, "");

  auto lblXMax = constructLabel(box, "x-max");
  m_dataFields.txtXMax = constructLabel(box, "");

  auto lblYMin = constructLabel(box, "y-min");
  m_dataFields.txtYMin = constructLabel(box, "");

  auto lblYMax = constructLabel(box, "y-max");
  m_dataFields.txtYMax = constructLabel(box, "");

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblMagLevel, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_dataFields.txtMagLevel, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblXMin, 0, wxEXPAND | wxLEFT  | wxRIGHT, 10);
  grid->Add(m_dataFields.txtXMin, 1, wxEXPAND, 10);
  grid->Add(lblXMax, 0, wxEXPAND | wxLEFT  | wxRIGHT, 10);
  grid->Add(m_dataFields.txtXMax, 1, wxEXPAND, 10);
  grid->Add(lblYMin, 0, wxEXPAND | wxLEFT  | wxRIGHT, 10);
  grid->Add(m_dataFields.txtYMin, 1, wxEXPAND, 10);
  grid->Add(lblYMax, 0, wxEXPAND | wxLEFT  | wxRIGHT, 10);
  grid->Add(m_dataFields.txtYMax, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(1);

  return box;
}

wxStaticBox* MainWindow::constructExportPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto grid = new wxGridBagSizer;
  box->SetSizer(grid);

  auto lblWidth = constructLabel(box, wxGetTranslation("Width"));
  m_export.txtWidth = constructTextBox(box, wxEmptyString);
  m_export.txtWidth->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_export.txtWidth->Bind(wxEVT_TEXT, &MainWindow::onExportWidthChange, this);

  auto lblHeight = constructLabel(box, wxGetTranslation("Height"));
  m_export.txtHeight = constructTextBox(box,
                                        std::to_string(DEFAULT_EXPORT_HEIGHT));
  m_export.txtHeight->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_export.txtHeight->Bind(wxEVT_TEXT, &MainWindow::onExportHeightChange, this);

  m_export.btnExport = new wxButton(box, wxID_ANY, wxGetTranslation("Export"));
  m_export.btnExport->Bind(wxEVT_BUTTON, &MainWindow::onExportClick, this);

  m_export.progressBar = new wxGauge(box, wxID_ANY, 100);

  grid->Add(lblWidth, wxGBPosition(0, 0), wxGBSpan(1, 1), wxLEFT | wxRIGHT, 10);
  grid->Add(m_export.txtWidth, wxGBPosition(1, 0), wxGBSpan(1, 1),
            wxEXPAND | wxRIGHT, 10);
  grid->Add(lblHeight, wxGBPosition(0, 1), wxGBSpan(1, 1), wxLEFT | wxRIGHT,
            10);
  grid->Add(m_export.txtHeight, wxGBPosition(1, 1), wxGBSpan(1, 1),
            wxEXPAND | wxRIGHT, 10);
  grid->Add(m_export.btnExport, wxGBPosition(2, 1), wxGBSpan(1, 1),
            wxEXPAND | wxRIGHT, 10);
  grid->Add(m_export.progressBar, wxGBPosition(3, 0), wxGBSpan(2, 1),
            wxEXPAND | wxLEFT | wxRIGHT | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);

  grid->AddGrowableCol(0);

  m_export.progressBar->Hide();

  return box;
}

void MainWindow::constructInfoPage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Info"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructInfoPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(constructDataPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  page->SetSizer(vbox);
}

void MainWindow::constructParamsPage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Parameters"));

  m_params.btnApply = new wxButton(page, wxID_ANY, wxGetTranslation("Apply"));
  m_params.btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyParamsClick, this);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructRenderParamsPanel(page), 1,
            wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vbox->Add(constructFlyThroughParamsPanel(page), 1,
            wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vbox->Add(m_params.btnApply, 0, wxALIGN_RIGHT | wxRIGHT, 10);

  page->SetSizer(vbox);
}

void MainWindow::constructColourSchemePage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Colours"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructColourSchemePanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT,
            10);

  page->SetSizer(vbox);
}

void MainWindow::constructExportPage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Export"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructExportPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  page->SetSizer(vbox);
}

void MainWindow::constructRightPanel() {
  m_rightPanel = new wxNotebook(m_splitter, wxID_ANY);
  constructInfoPage();
  constructParamsPage();
  constructColourSchemePage();
  constructExportPage();
}

void MainWindow::constructMenu() {
  wxMenu* mnuFile = new wxMenu;
  mnuFile->Append(wxID_EXIT);

  wxMenu* mnuHelp = new wxMenu;
  mnuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(mnuFile, wxGetTranslation("&File"));
  menuBar->Append(mnuHelp, wxGetTranslation("&Help"));

  SetMenuBar(menuBar);
}

void MainWindow::adjustExportSize(bool adjustWidth) {
  float canvasW = m_canvas->GetSize().x;
  float canvasH = m_canvas->GetSize().y;
  float aspect = canvasW / canvasH;

  if (adjustWidth) {
    long exportH = 0;
    m_export.txtHeight->GetValue().ToLong(&exportH);

    long exportW = exportH * aspect;
    m_export.txtWidth->ChangeValue(std::to_string(exportW));
  }
  else {
    long exportW = 0;
    m_export.txtWidth->GetValue().ToLong(&exportW);

    long exportH = exportW / aspect;
    m_export.txtHeight->ChangeValue(std::to_string(exportH));
  }
}

void MainWindow::onCanvasGainFocus(wxFocusEvent&) {
  m_leftPanel->SetBackgroundColour(*wxBLUE);
  Refresh();
}

void MainWindow::onCanvasLoseFocus(wxFocusEvent&) {
  m_leftPanel->SetBackgroundColour(*wxLIGHT_GREY);
  Refresh();
}

void MainWindow::onCanvasResize(wxSizeEvent& e) {
  adjustExportSize(true);

  e.Skip();
}

void MainWindow::onExportHeightChange(wxCommandEvent&) {
  adjustExportSize(true);
}

void MainWindow::onExportWidthChange(wxCommandEvent&) {
  adjustExportSize(false);
}

void MainWindow::onRender() {
  auto magLevel = formatDouble(m_renderer->computeMagnification());
  m_dataFields.txtMagLevel->SetLabel(magLevel);
  m_dataFields.txtXMin->SetLabel(formatDouble(m_renderer->getXMin()));
  m_dataFields.txtXMax->SetLabel(formatDouble(m_renderer->getXMax()));
  m_dataFields.txtYMin->SetLabel(formatDouble(m_renderer->getYMin()));
  m_dataFields.txtYMax->SetLabel(formatDouble(m_renderer->getYMax()));
}

uint8_t* MainWindow::beginExport(int w, int h) {
  m_export.progressBar->Show();
  m_export.btnExport->Disable();
  m_params.btnApply->Disable();
  m_colourScheme.btnDelete->Disable();
  m_colourScheme.btnRestore->Disable();
  m_colourScheme.btnSave->Disable();
  m_canvas->disable();
  SetStatusText(wxGetTranslation("Exporting to file..."));

  m_export.busy = true;
  m_renderer->renderToMainMemoryBuffer(w, h);

  const OfflineRenderStatus& status = m_renderer->continueOfflineRender();

  while (status.progress != 100) {
    m_export.progressBar->SetValue(status.progress);
    wxYield();

    if (m_quitting) {
      m_export.busy = false;
      Close();
      return nullptr;
    }

    m_renderer->continueOfflineRender();
  }

  return status.data;
}

void MainWindow::endExport(const wxString& exportFilePath, int w, int h,
                           uint8_t* data) {
  m_export.busy = false;

  if (data != nullptr) {
    wxImage image(w, h, data);
    image = image.Mirror(false);

    image.SaveFile(exportFilePath, wxBITMAP_TYPE_BMP);
  }

  m_export.progressBar->Hide();
  m_export.btnExport->Enable();
  m_params.btnApply->Enable();
  m_colourScheme.btnDelete->Enable();
  m_colourScheme.btnRestore->Enable();
  m_colourScheme.btnSave->Enable();
  m_canvas->enable();
  SetStatusText(wxGetTranslation("Export complete"));

  m_canvas->refresh();
}

void MainWindow::onExportClick(wxCommandEvent&) {
  wxFileDialog fileDialog(this, wxGetTranslation("Save as BMP image"), "",
                          "", "BMP files (*.bmp)|*.bmp", wxFD_SAVE);
  if (fileDialog.ShowModal() == wxID_CANCEL) {
    return;
  }

  long w = getBoundedValue<long>(*m_export.txtWidth, MIN_EXPORT_WIDTH,
                                 MAX_EXPORT_WIDTH);

  long h = getBoundedValue<long>(*m_export.txtHeight, MIN_EXPORT_HEIGHT,
                                 MAX_EXPORT_HEIGHT);

  wxString exportFilePath = fileDialog.GetPath();

  uint8_t* data = beginExport(w, h);
  endExport(exportFilePath, w, h, data);
}

void MainWindow::onApplyParamsClick(wxCommandEvent&) {
  long maxI = getBoundedValue<long>(*m_params.txtMaxIterations, MIN_ITERATIONS,
                                    MAX_ITERATIONS);
  m_renderer->setMaxIterations(maxI);

  double zoomAmount = getBoundedValue<double>(*m_params.txtZoomAmount,
                                              MIN_ZOOM_AMOUNT,
                                              MAX_ZOOM_AMOUNT);
  m_canvas->setZoomAmount(zoomAmount);

  double targetFps = getBoundedValue<double>(*m_params.txtTargetFps,
                                             MIN_TARGET_FPS,
                                             MAX_TARGET_FPS);
  m_canvas->setTargetFps(targetFps);

  double zoomPerFrame = getBoundedValue<double>(*m_params.txtZoomPerFrame,
                                                MIN_ZOOM_PER_FRAME,
                                                MAX_ZOOM_PER_FRAME);
  m_canvas->setZoomPerFrame(zoomPerFrame);

  m_canvas->refresh();
}

void MainWindow::applyColourScheme() {
  try {
    string name = m_colourScheme.cboSelector->GetValue().ToStdString();
    string code = m_colourScheme.txtCode->GetValue().ToStdString();
    m_renderer->setColourSchemeImpl(code);
    m_canvas->refresh();
    m_colourScheme.txtCompileStatus->SetValue(wxGetTranslation("Success"));
    m_colourScheme.colourSchemes[name] = code;
  }
  catch (const ShaderException& e) {
    m_colourScheme.txtCompileStatus->SetValue(e.errorOutput());
  }
}

void MainWindow::selectColourScheme(int idx) {
  auto& cbo = *m_colourScheme.cboSelector;
  assert(idx >= 0 && idx < static_cast<int>(cbo.GetCount()));

  wxString name = cbo.GetString(idx);
  cbo.SetSelection(idx);

  wxCommandEvent e;
  e.SetString(name);
  onSelectColourScheme(e);
}

void MainWindow::selectColourScheme(const wxString& name) {
  wxCommandEvent e;
  int idx = m_colourScheme.cboSelector->FindString(name);

  if (idx != wxNOT_FOUND) {
    m_colourScheme.cboSelector->SetSelection(idx);
    e.SetString(name);
  }
  else {
    m_colourScheme.cboSelector->SetSelection(0);
    e.SetString(m_colourScheme.cboSelector->GetString(0));
  }

  onColourSchemeNameChange(e);
  onSelectColourScheme(e);
}

void MainWindow::onColourSchemeNameChange(wxCommandEvent&) {
  wxString scheme = m_colourScheme.cboSelector->GetValue();
  bool isPreset = PRESETS.count(scheme.ToStdString()) == 1;

  m_colourScheme.btnDelete->Show(!isPreset);
  m_colourScheme.btnRestore->Show(isPreset);

  m_colourScheme.btnDelete->GetParent()->Layout();
}

void MainWindow::onSelectColourScheme(wxCommandEvent& e) {
  string scheme = e.GetString().ToStdString();
  m_colourScheme.txtCode->SetValue(m_colourScheme.colourSchemes.at(scheme));
  applyColourScheme();
}

static wxXmlDocument colourSchemesToXmlDoc(const StringMap& schemes) {
  wxXmlDocument doc;
  auto root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "colour_schemes");
  
  for (auto entry : schemes) {
    const wxString& name = entry.first;
    const wxString& code = entry.second;

    auto xmlColourScheme = new wxXmlNode(root, wxXML_ELEMENT_NODE,
                                         "colour_scheme");

    xmlColourScheme->AddAttribute("name", name);
    xmlColourScheme->AddChild(new wxXmlNode(nullptr, wxXML_TEXT_NODE, "code",
                                            code));
  }

  doc.SetRoot(root);
  return doc;
}

void MainWindow::saveColourSchemes() {
  if (!wxFile::Exists(m_colourScheme.filePath)) {
    wxDir::Make(userDataPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  }

  wxXmlDocument doc = colourSchemesToXmlDoc(m_colourScheme.colourSchemes);
  doc.Save(m_colourScheme.filePath);
}

void MainWindow::onSaveColourSchemeClick(wxCommandEvent&) {
  string name = m_colourScheme.cboSelector->GetValue().ToStdString();
  string code = m_colourScheme.txtCode->GetValue().ToStdString();
  m_colourScheme.colourSchemes[name] = code;

  updateColourSchemeSelector();
  saveColourSchemes();
}

void MainWindow::updateColourSchemeSelector() {
  auto& cbo = *m_colourScheme.cboSelector;
  wxString name = cbo.GetValue();

  std::vector<wxString> names;
  for (auto entry : m_colourScheme.colourSchemes) {
    names.push_back(entry.first);
  }
  cbo.Clear();
  cbo.Insert(names, 0);

  selectColourScheme(name);
}

void MainWindow::onDeleteColourSchemeClick(wxCommandEvent&) {
  auto& cbo = *m_colourScheme.cboSelector;
  string name = cbo.GetValue().ToStdString();

  assert(PRESETS.count(name) == 0);

  m_colourScheme.colourSchemes.erase(name);
  updateColourSchemeSelector();
  saveColourSchemes();
}

void MainWindow::loadColourSchemes() {
  m_colourScheme.filePath = userDataPath("colour_schemes.xml");

  for (auto entry : PRESETS) {
    m_colourScheme.colourSchemes.insert(entry);
  }

  if (wxFile::Exists(m_colourScheme.filePath)) {
    wxXmlDocument doc;
    doc.Load(m_colourScheme.filePath);

    auto xmlColourScheme = doc.GetRoot()->GetChildren();
    while (xmlColourScheme) {
      wxString name = xmlColourScheme->GetAttribute("name");

      auto xmlCode = xmlColourScheme->GetChildren();
      wxString code = xmlCode->GetContent();

      m_colourScheme.colourSchemes[name.ToStdString()] = code.ToStdString();

      xmlColourScheme = xmlColourScheme->GetNext();
    }
  }
}

void MainWindow::onRestoreColourSchemeClick(wxCommandEvent&) {
  string name = m_colourScheme.cboSelector->GetValue().ToStdString();
  m_colourScheme.txtCode->ChangeValue(PRESETS.at(name));

  applyColourScheme();
  saveColourSchemes();
}

void MainWindow::onFlyThroughModeToggle(wxCommandEvent& e) {
  if (e.GetInt() == TOGGLED_ON) {
    SetStatusText(wxGetTranslation("Fly-Through mode activated"));
  }
  else if (e.GetInt() == TOGGLED_OFF) {
    SetStatusText(wxGetTranslation("Fly-Through mode deactivated"));
  }
}

void MainWindow::onExit(wxCommandEvent&) {
  Close();
}

void MainWindow::onAbout(wxCommandEvent&) {
  std::stringstream ss;
  ss << "The Mandelbrot fractal rendered on the GPU." << std::endl << std::endl;
  ss << "Author: Rob Jinman <jinmanr@gmail.com>" << std::endl << std::endl;
  ss << "Copyright Rob Jinman 2019. All rights reserved.";

  wxMessageBox(wxGetTranslation(ss.str()), versionString(),
               wxOK | wxICON_INFORMATION);
}

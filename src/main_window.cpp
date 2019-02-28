#include <sstream>
#include <iomanip>
#include <wx/gbsizer.h>
#include <wx/richtext/richtextctrl.h>
#include "main_window.hpp"
#include "config.hpp"
#include "exception.hpp"
#include "wx_helpers.hpp"

static const int WINDOW_W = 1000;
static const int WINDOW_H = 600;
static const int DEFAULT_EXPORT_HEIGHT = 1000;

static std::string formatDouble(double d) {
  std::stringstream ss;
  ss << std::scientific << d;
  return ss.str();
}

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(wxID_EXIT, MainWindow::onExit)
  EVT_MENU(wxID_ABOUT, MainWindow::onAbout)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {

  m_vbox = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_vbox);

  SetAutoLayout(true);

  m_mandelbrot.reset(new Mandelbrot(400, 400));
  m_splitter = new wxSplitterWindow(this);
  m_splitter->SetMinimumPaneSize(300);

  m_vbox->Add(m_splitter, 1, wxEXPAND, 0);

  constructMenu();
  constructLeftPanel();
  constructRightPanel();

  m_splitter->SplitVertically(m_leftPanel, m_rightPanel);

  CreateStatusBar();
  SetStatusText(wxEmptyString);
}

void MainWindow::constructLeftPanel() {
  m_leftPanel = new wxPanel(m_splitter, wxID_ANY);
  auto vbox = new wxBoxSizer(wxVERTICAL);
  m_leftPanel->SetSizer(vbox);
  m_leftPanel->SetCanFocus(true);

  int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
  m_canvas = new Canvas(m_leftPanel, args, *m_mandelbrot,
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

  auto txtInfo = new wxRichTextCtrl(box, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxVSCROLL | wxBORDER_NONE | wxWANTS_CHARS);

  auto addHeading = [txtInfo](const wxString& text) {
    txtInfo->BeginBold();
    txtInfo->BeginUnderline();
    txtInfo->WriteText(wxGetTranslation(text));
    txtInfo->EndUnderline();
    txtInfo->EndBold();
    txtInfo->Newline();
  };

  auto addText = [txtInfo](const wxString& text, bool translate = true,
                           bool newline = true) {
    txtInfo->WriteText(translate ? wxGetTranslation(text) : text);
    if (newline) {
      txtInfo->Newline();
    }
  };

  txtInfo->SetEditable(false);
  txtInfo->SetMinSize(wxSize(0, 80));

  addHeading(wxGetTranslation("Controls"));
  addText("R\t", false, false);
  addText("Reset view");
  addText("Z\t", false, false);
  addText("Toggle Fly-Through mode");

  vbox->AddSpacer(10);
  vbox->Add(txtInfo, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructColourSchemePanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto cboScheme = new wxChoice(box, wxID_ANY);
  std::vector<wxString> names;
  for (auto entry : PRESETS) {
    names.push_back(entry.first);
  }
  cboScheme->Insert(names, 0);
  cboScheme->SetStringSelection(DEFAULT_COLOUR_SCHEME);
  cboScheme->Bind(wxEVT_CHOICE, &MainWindow::onSelectColourScheme, this);

  wxFont codeFontNormal(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  wxFont codeFontBold(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_BOLD);

  auto txtShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 computeColour(int i, float x, float y) {");
  txtShaderCodePre->SetFont(codeFontBold);
  m_txtComputeColourImpl = constructTextBox(box,
                                            PRESETS.at(DEFAULT_COLOUR_SCHEME),
                                            true);
  m_txtComputeColourImpl->SetFont(codeFontNormal);
  auto txtShaderCodePost = new wxStaticText(box, wxID_ANY, "}");
  txtShaderCodePost->SetFont(codeFontBold);
  m_txtCompileStatus = new wxTextCtrl(box, wxID_ANY, wxEmptyString,
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      wxTE_MULTILINE);
  m_txtCompileStatus->SetFont(codeFontNormal);
  m_txtCompileStatus->SetEditable(false);
  m_txtCompileStatus->SetMinSize(wxSize(0, 80));

  auto btnApply = new wxButton(box, wxID_ANY, wxGetTranslation("Apply"));
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyColourSchemeClick, this);

  vbox->AddSpacer(10);
  vbox->Add(cboScheme, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  vbox->Add(txtShaderCodePre, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(m_txtComputeColourImpl, 2, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(txtShaderCodePost, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(btnApply, 0, wxALIGN_RIGHT | wxRIGHT, 10);
  vbox->Add(m_txtCompileStatus, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructRenderParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Render Parameters"));

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto strMaxI = std::to_string(DEFAULT_MAX_ITERATIONS);
  auto lblMaxI = constructLabel(box, wxGetTranslation("Max iterations"));
  m_txtMaxIterations = constructTextBox(box, strMaxI);
  m_txtMaxIterations->SetValidator(wxTextValidator(wxFILTER_DIGITS));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblMaxI, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtMaxIterations, 0, wxEXPAND | wxRIGHT, 10);

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
  m_txtTargetFps = constructTextBox(box, strFps);
  m_txtTargetFps->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  auto strZoom = std::to_string(DEFAULT_ZOOM_PER_FRAME);
  auto lblZoom = constructLabel(box, wxGetTranslation("Zoom per frame"));
  m_txtZoomPerFrame = constructTextBox(box, strZoom);
  m_txtZoomPerFrame->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblFps, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtTargetFps, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblZoom, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtZoomPerFrame, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

  return box;
}

wxStaticBox* MainWindow::constructDataPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Data"));

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  double magnification = m_mandelbrot->computeMagnification();

  auto lblMagLevel = constructLabel(box, wxGetTranslation("Magnification"));
  m_dataFields.txtMagLevel = constructLabel(box, formatDouble(magnification));

  auto lblXMin = constructLabel(box, "x-min");
  m_dataFields.txtXMin = constructLabel(box,
                                        formatDouble(m_mandelbrot->getXMin()));

  auto lblXMax = constructLabel(box, "x-max");
  m_dataFields.txtXMax = constructLabel(box,
                                        formatDouble(m_mandelbrot->getXMax()));

  auto lblYMin = constructLabel(box, "y-min");
  m_dataFields.txtYMin = constructLabel(box,
                                        formatDouble(m_mandelbrot->getYMin()));

  auto lblYMax = constructLabel(box, "y-max");
  m_dataFields.txtYMax = constructLabel(box,
                                        formatDouble(m_mandelbrot->getYMax()));

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

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto lblWidth = constructLabel(box, wxGetTranslation("Width"));
  m_txtExportWidth = constructTextBox(box, wxEmptyString);
  m_txtExportWidth->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_txtExportWidth->Bind(wxEVT_TEXT, &MainWindow::onExportWidthChange, this);

  auto lblHeight = constructLabel(box, wxGetTranslation("Height"));
  m_txtExportHeight = constructTextBox(box,
                                       std::to_string(DEFAULT_EXPORT_HEIGHT));
  m_txtExportHeight->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_txtExportHeight->Bind(wxEVT_TEXT, &MainWindow::onExportHeightChange, this);

  auto btnExport = new wxButton(box, wxID_ANY, wxGetTranslation("Export"));
  btnExport->Bind(wxEVT_BUTTON, &MainWindow::onExportClick, this);

  grid->Add(lblWidth, 0, wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtExportWidth, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblHeight, 0, wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtExportHeight, 0, wxEXPAND | wxRIGHT, 10);
  grid->AddSpacer(1);
  grid->Add(btnExport, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

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

  auto btnApply = new wxButton(page, wxID_ANY, wxGetTranslation("Apply"));
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyParamsClick, this);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructRenderParamsPanel(page), 1,
            wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vbox->Add(constructFlyThroughParamsPanel(page), 1,
            wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vbox->Add(btnApply, 0, wxALIGN_RIGHT | wxRIGHT, 10);

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
    m_txtExportHeight->GetValue().ToLong(&exportH);

    long exportW = exportH * aspect;
    m_txtExportWidth->ChangeValue(std::to_string(exportW));
  }
  else {
    long exportW = 0;
    m_txtExportWidth->GetValue().ToLong(&exportW);

    long exportH = exportW / aspect;
    m_txtExportHeight->ChangeValue(std::to_string(exportH));
  }
}

void MainWindow::onCanvasGainFocus(wxFocusEvent&) {
  m_leftPanel->SetBackgroundColour(*wxBLUE);
}

void MainWindow::onCanvasLoseFocus(wxFocusEvent&) {
  m_leftPanel->SetBackgroundColour(*wxLIGHT_GREY);
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
  auto magLevel = formatDouble(m_mandelbrot->computeMagnification());
  m_dataFields.txtMagLevel->SetLabel(magLevel);
  m_dataFields.txtXMin->SetLabel(formatDouble(m_mandelbrot->getXMin()));
  m_dataFields.txtXMax->SetLabel(formatDouble(m_mandelbrot->getXMax()));
  m_dataFields.txtYMin->SetLabel(formatDouble(m_mandelbrot->getYMin()));
  m_dataFields.txtYMax->SetLabel(formatDouble(m_mandelbrot->getYMax()));
}

void MainWindow::onExportClick(wxCommandEvent&) {
  wxFileDialog fileDialog(this, wxGetTranslation("Save as BMP image"), "",
                          "", "BMP files (*.bmp)|*.bmp", wxFD_SAVE);
  if (fileDialog.ShowModal() == wxID_CANCEL) {
    return;
  }

  long w = 0;
  m_txtExportWidth->GetValue().ToLong(&w);

  long h = 0;
  m_txtExportHeight->GetValue().ToLong(&h);

  size_t nBytes = 0;
  uint8_t* data = m_mandelbrot->renderToMainMemoryBuffer(w, h, nBytes);

  wxImage image(w, h, data);
  image = image.Mirror(false);

  image.SaveFile(fileDialog.GetPath(), wxBITMAP_TYPE_BMP);
}

void MainWindow::onApplyParamsClick(wxCommandEvent&) {
  bool needRefresh = false;

  long maxI = 0;
  m_txtMaxIterations->GetValue().ToLong(&maxI);
  m_mandelbrot->setMaxIterations(maxI);
  needRefresh = true;

  double targetFps = 0;
  m_txtTargetFps->GetValue().ToDouble(&targetFps);
  m_canvas->setTargetFps(targetFps);

  double zoomPerFrame = 0;
  m_txtZoomPerFrame->GetValue().ToDouble(&zoomPerFrame);
  m_canvas->setZoomPerFrame(zoomPerFrame);

  if (needRefresh) {
    m_canvas->refresh();
  }
}

void MainWindow::applyColourScheme() {
  try {
    std::string code = m_txtComputeColourImpl->GetValue().ToStdString();
    m_mandelbrot->setColourSchemeImpl(code);
    m_canvas->refresh();
    m_txtCompileStatus->SetValue(wxGetTranslation("Success"));
  }
  catch (const ShaderException& e) {
    m_txtCompileStatus->SetValue(e.errorOutput());
  }
}

void MainWindow::onSelectColourScheme(wxCommandEvent& e) {
  std::string scheme = e.GetString().ToStdString();
  m_txtComputeColourImpl->SetValue(PRESETS.at(scheme));
  applyColourScheme();
}

void MainWindow::onApplyColourSchemeClick(wxCommandEvent&) {
  applyColourScheme();
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
  Close(true);
}

static std::string versionString() {
  std::stringstream ss;
  ss << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
     << Mandelbrot_VERSION_MINOR;
  return ss.str();
}

void MainWindow::onAbout(wxCommandEvent&) {
  std::stringstream ss;
  ss << "The Mandelbrot fractal rendered on the GPU inside a fragment "
        "shader." << std::endl << std::endl;
  ss << "Author: Rob Jinman <jinmanr@gmail.com>" << std::endl << std::endl;
  ss << "Copyright Rob Jinman 2019. All rights reserved.";

  wxMessageBox(wxGetTranslation(ss.str()), versionString(),
               wxOK | wxICON_INFORMATION);
}

class Application : public wxApp {
public:
  virtual bool OnInit() override {
    MainWindow* frame = new MainWindow(versionString(),
                                       wxSize(WINDOW_W, WINDOW_H));
    frame->Show();
    return true;
  }

  virtual void HandleEvent(wxEvtHandler* handler, wxEventFunction func,
                           wxEvent& event) const override {
    try {
      wxApp::HandleEvent(handler, func, event);
    }
    catch (const ShaderException& e) {
      std::cerr << "A fatal exception occurred: " << std::endl;
      std::cerr << e.what() << ": " << e.errorOutput() << std::endl;
      exit(1);
    }
    catch (const std::runtime_error& e) {
      std::cerr << "A fatal exception occurred: " << std::endl;
      std::cerr << e.what() << std::endl;
      exit(1);
    }
  }
};

wxIMPLEMENT_APP(Application);

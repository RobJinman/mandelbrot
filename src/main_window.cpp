#include <sstream>
#include <iomanip>
#include <wx/gbsizer.h>
#include "main_window.hpp"
#include "config.hpp"
#include "exception.hpp"
#include "wx_helpers.hpp"

static const int WINDOW_W = 1000;
static const int WINDOW_H = 600;

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

  m_vbox->Add(m_splitter, 1, wxEXPAND, 0);

  constructMenu();
  constructLeftPanel();
  constructRightPanel();

  m_splitter->SplitVertically(m_canvas, m_rightPanel);

  auto sz = GetSize();
  float aspect = static_cast<float>(sz.y) / static_cast<float>(sz.x);
  m_splitter->SetSashGravity(aspect);

  CreateStatusBar();
  SetStatusText("");
}

void MainWindow::constructLeftPanel() {
  int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
  m_canvas = new Canvas(m_splitter, args, *m_mandelbrot,
                        [this]() { onRender(); });
  m_canvas->Bind(FLY_THROUGH_MODE_TOGGLED, &MainWindow::onFlyThroughModeToggle,
                 this);
}

wxStaticBox* MainWindow::constructFlyThroughPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Fly-Through Mode"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto strFlyThrough = "With the left panel in focus, press the Z key to toggle"
                       " Fly-Through mode.";

  auto txtFlyThrough = new wxTextCtrl(box, wxID_ANY,
                                      wxGetTranslation(strFlyThrough),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxTE_MULTILINE);
  txtFlyThrough->SetEditable(false);
  txtFlyThrough->SetMinSize(wxSize(0, 80));

  vbox->AddSpacer(10);
  vbox->Add(txtFlyThrough, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructColourSchemePanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, "");

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

  auto txtShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 computeColour(int i, float x, float y) {");
  m_txtComputeColourImpl = constructTextBox(box,
                                            PRESETS.at(DEFAULT_COLOUR_SCHEME),
                                            true);
  auto txtShaderCodePost = new wxStaticText(box, wxID_ANY, "}");
  m_txtCompileStatus = new wxTextCtrl(box, wxID_ANY, "",
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      wxTE_MULTILINE);
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

wxStaticBox* MainWindow::constructParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, "");

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto strMaxI = std::to_string(DEFAULT_MAX_ITERATIONS);
  auto lblMaxI = constructLabel(box, wxGetTranslation("Max iterations"));
  m_txtMaxIterations = constructTextBox(box, strMaxI);

  auto btnApply = new wxButton(box, wxID_ANY, wxGetTranslation("Apply"));
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyParamsClick, this);

  grid->Add(lblMaxI, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_txtMaxIterations, 0, wxEXPAND | wxRIGHT, 10);
  grid->AddSpacer(1);
  grid->Add(btnApply, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(1);

  return box;
}

wxStaticBox* MainWindow::constructInfoPanel(wxWindow* parent) {
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
  grid->Add(lblMagLevel, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_dataFields.txtMagLevel, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblXMin, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_dataFields.txtXMin, 1, wxEXPAND, 10);
  grid->Add(lblXMax, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_dataFields.txtXMax, 1, wxEXPAND, 10);
  grid->Add(lblYMin, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_dataFields.txtYMin, 1, wxEXPAND, 10);
  grid->Add(lblYMax, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(m_dataFields.txtYMax, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(1);

  return box;
}

wxStaticBox* MainWindow::constructExportPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, "");

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto lblWidth = constructLabel(box, wxGetTranslation("Width"));
  m_txtExportWidth = constructTextBox(box, "");

  auto lblHeight = constructLabel(box, wxGetTranslation("Height"));
  m_txtExportHeight = constructTextBox(box, "");

  auto btnExport = new wxButton(box, wxID_ANY, wxGetTranslation("Export"));
  btnExport->Bind(wxEVT_BUTTON, &MainWindow::onExportClick, this);

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblWidth, 0, wxRIGHT, 10);
  grid->Add(m_txtExportWidth, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblHeight, 0, wxRIGHT, 10);
  grid->Add(m_txtExportHeight, 0, wxEXPAND | wxRIGHT, 10);
  grid->AddSpacer(1);
  grid->Add(btnExport, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(1);

  return box;
}

void MainWindow::constructInfoPage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Info"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructFlyThroughPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(constructInfoPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  page->SetSizer(vbox);
}

void MainWindow::constructParamsPage() {
  auto page = new wxNotebookPage(m_rightPanel, wxID_ANY);
  m_rightPanel->AddPage(page, wxGetTranslation("Parameters"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructParamsPanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

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

static bool tryGetIntFromTextCtrl(wxTextCtrl& txt, int& value) {
  try {
    value = std::stoi(txt.GetValue().ToStdString());
  }
  catch (...) {
    txt.SetValue(wxGetTranslation("error"));
    return false;
  }

  return true;
}

void MainWindow::onRender() {
  auto magLevel = formatDouble(m_mandelbrot->computeMagnification());
  m_dataFields.txtMagLevel->SetLabel(magLevel);
  m_dataFields.txtXMin->SetLabel(formatDouble(m_mandelbrot->getXMin()));
  m_dataFields.txtXMax->SetLabel(formatDouble(m_mandelbrot->getXMax()));
  m_dataFields.txtYMin->SetLabel(formatDouble(m_mandelbrot->getYMin()));
  m_dataFields.txtYMax->SetLabel(formatDouble(m_mandelbrot->getYMax()));
}

void MainWindow::onExportClick(wxCommandEvent& e) {
  int w = 0;
  int h = 0;
  if (tryGetIntFromTextCtrl(*m_txtExportWidth, w)
      && tryGetIntFromTextCtrl(*m_txtExportHeight, h)) {

    // TODO
  }
}

void MainWindow::onApplyParamsClick(wxCommandEvent& e) {
  int maxI = 0;
  if (tryGetIntFromTextCtrl(*m_txtMaxIterations, maxI)) {
    m_mandelbrot->setMaxIterations(maxI);
    m_canvas->Refresh();
  }
}

void MainWindow::applyColourScheme() {
  try {
    std::string code = m_txtComputeColourImpl->GetValue().ToStdString();
    m_mandelbrot->setColourSchemeImpl(code);
    m_canvas->Refresh();
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

void MainWindow::onApplyColourSchemeClick(wxCommandEvent& e) {
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

#include <sstream>
#include <iomanip>
#include <wx/gbsizer.h>
#include "main_window.hpp"
#include "config.hpp"
#include "exception.hpp"

struct WidgetAndHBox {
  wxWindow* widget;
  wxBoxSizer* hbox;
};

static WidgetAndHBox constructTextField(wxWindow* parent, const wxString& label,
                                        const wxString& content = "",
                                        bool multiline = false,
                                        bool readonly = false) {

  WidgetAndHBox ctrl;
  if (readonly) {
    ctrl.widget = new wxStaticText(parent, wxID_ANY, content);
  }
  else {
    ctrl.widget = new wxTextCtrl(parent, wxID_ANY, content,
                                wxDefaultPosition, wxDefaultSize,
                                multiline ? wxTE_MULTILINE : 0L);
  }

  ctrl.hbox = new wxBoxSizer(wxHORIZONTAL);

  if (label.length() > 0) {
    auto txtLabel = new wxStaticText(parent, wxID_ANY, label);
    ctrl.hbox->Add(txtLabel, 0, wxRIGHT, 8);
  }

  ctrl.hbox->Add(ctrl.widget, 1);

  return ctrl;
}

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

  auto ctrlFlyThrough = constructTextField(box, "",
                                           wxGetTranslation(strFlyThrough),
                                           true);
  auto txtFlyThrough = dynamic_cast<wxTextCtrl*>(ctrlFlyThrough.widget);
  txtFlyThrough->SetEditable(false);

  vbox->Add(ctrlFlyThrough.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructColourSchemePanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Colour Scheme"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto txtShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 getColour(int i) {");
  auto ctrlShaderCode = constructTextField(box, "  ", "", true);
  m_txtComputeColourImpl = dynamic_cast<wxTextCtrl*>(ctrlShaderCode.widget);
  auto txtShaderCodePost = new wxStaticText(box, wxID_ANY, "}");
  auto ctrlCompileStatus = constructTextField(box, "", "", true);
  m_txtComputeColourImplCompileStatus =
    dynamic_cast<wxTextCtrl*>(ctrlCompileStatus.widget);
  m_txtComputeColourImplCompileStatus->SetEditable(false);

  auto btnApply = new wxButton(box, wxID_ANY, wxGetTranslation("Apply"));
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyColourSchemeClick, this);

  vbox->Add(txtShaderCodePre);
  vbox->Add(ctrlShaderCode.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(txtShaderCodePost);
  vbox->Add(ctrlCompileStatus.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(btnApply, 0, wxALIGN_RIGHT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Configurable Options"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto strMaxI = std::to_string(DEFAULT_MAX_ITERATIONS);
  auto ctrlMaxIterations = constructTextField(box, "Max iterations", strMaxI);
  m_txtMaxIterations = dynamic_cast<wxTextCtrl*>(ctrlMaxIterations.widget);

  auto btnApply = new wxButton(box, wxID_ANY, wxGetTranslation("Apply"));
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onApplyParamsClick, this);

  vbox->Add(ctrlMaxIterations.hbox, 0, wxEXPAND | wxRIGHT, 10);
  vbox->Add(btnApply, 0, wxALIGN_RIGHT | wxRIGHT, 10);

  return box;
}

wxStaticBox* MainWindow::constructInfoPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Data"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  double magnification = m_mandelbrot->computeMagnification();

  auto ctrlMagLevel = constructTextField(box, "Magnification",
                                        formatDouble(magnification),
                                        false, true);

  auto ctrlXMin = constructTextField(box, "x-min",
                                     formatDouble(m_mandelbrot->getXMin()),
                                     false, true);

  auto ctrlXMax = constructTextField(box, "x-max",
                                     formatDouble(m_mandelbrot->getXMax()),
                                     false, true);

  auto ctrlYMin = constructTextField(box, "y-min",
                                     formatDouble(m_mandelbrot->getYMin()),
                                     false, true);

  auto ctrlYMax = constructTextField(box, "y-max",
                                     formatDouble(m_mandelbrot->getYMax()),
                                     false, true);

  m_dataFields.txtMagLevel = dynamic_cast<wxStaticText*>(ctrlMagLevel.widget);
  m_dataFields.txtXMin = dynamic_cast<wxStaticText*>(ctrlXMin.widget);
  m_dataFields.txtXMax = dynamic_cast<wxStaticText*>(ctrlXMax.widget);
  m_dataFields.txtYMin = dynamic_cast<wxStaticText*>(ctrlYMin.widget);
  m_dataFields.txtYMax = dynamic_cast<wxStaticText*>(ctrlYMax.widget);

  vbox->Add(ctrlMagLevel.hbox, 0, wxEXPAND | wxRIGHT, 10);
  vbox->Add(ctrlXMin.hbox, 0, wxEXPAND | wxRIGHT, 10);
  vbox->Add(ctrlXMax.hbox, 0, wxEXPAND | wxRIGHT, 10);
  vbox->Add(ctrlYMin.hbox, 0, wxEXPAND | wxRIGHT, 10);
  vbox->Add(ctrlYMax.hbox, 0, wxEXPAND | wxRIGHT, 10);

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
  vbox->Add(constructColourSchemePanel(page), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  page->SetSizer(vbox);
}

void MainWindow::constructRightPanel() {
  m_rightPanel = new wxNotebook(m_splitter, wxID_ANY);
  constructInfoPage();
  constructParamsPage();
  constructColourSchemePage();
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

void MainWindow::onApplyParamsClick(wxCommandEvent& e) {
  int maxI = 0;
  if (tryGetIntFromTextCtrl(*m_txtMaxIterations, maxI)) {
    m_mandelbrot->setMaxIterations(maxI);
    m_canvas->Refresh();
  }
}

void MainWindow::onApplyColourSchemeClick(wxCommandEvent& e) {
  try {
    std::string code = m_txtComputeColourImpl->GetValue().ToStdString();
    m_mandelbrot->setColourScheme(code);
    m_canvas->Refresh();
    m_txtComputeColourImplCompileStatus->SetValue(wxGetTranslation("Success"));
  }
  catch (const ShaderException& e) {
    m_txtComputeColourImplCompileStatus->SetValue(e.errorOutput());
  }
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
    MainWindow* frame = new MainWindow(versionString(), wxSize(800, 500));
    frame->Show();
    return true;
  }

  virtual void HandleEvent(wxEvtHandler* handler, wxEventFunction func,
                           wxEvent& event) const override {
    try {
      wxApp::HandleEvent(handler, func, event);
    }
    catch (const std::runtime_error& e) {
      std::cerr << "A fatal exception occurred: " << std::endl;
      std::cerr << e.what();
      exit(1);
    }
  }
};

wxIMPLEMENT_APP(Application);

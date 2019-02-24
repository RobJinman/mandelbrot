#include <sstream>
#include <wx/gbsizer.h>
#include "main_window.hpp"
#include "config.hpp"

struct WidgetAndHBox {
  wxWindow* widget;
  wxBoxSizer* hbox;
};

static WidgetAndHBox constructTextField(wxWindow* parent, const wxString& label,
                                        bool multiline = false) {

  WidgetAndHBox ctrl;
  ctrl.widget = new wxTextCtrl(parent, wxID_ANY, "",
                               wxDefaultPosition, wxDefaultSize,
                               multiline ? wxTE_MULTILINE : 0L);

  ctrl.hbox = new wxBoxSizer(wxHORIZONTAL);

  if (label.length() > 0) {
    auto lblLabel = new wxStaticText(parent, wxID_ANY, label);
    ctrl.hbox->Add(lblLabel, 0, wxRIGHT, 8);
  }

  ctrl.hbox->Add(ctrl.widget, 1);

  return ctrl;
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
  m_canvas = new Canvas(m_splitter, args, *m_mandelbrot);
  m_canvas->Bind(FLY_THROUGH_MODE_TOGGLED, &MainWindow::onFlyThroughModeToggle,
                 this);
}

static wxStaticBox* constructFlyThroughPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Fly-Through Mode"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto strFlyThrough = "With the left panel in focus, press the Z key to toggle"
                       " Fly-Through mode.";

  auto ctrlFlyThrough = constructTextField(box, "", true);
  auto txtFlyThrough = dynamic_cast<wxTextCtrl*>(ctrlFlyThrough.widget);
  txtFlyThrough->SetEditable(false);
  txtFlyThrough->SetValue(wxGetTranslation(strFlyThrough));

  vbox->Add(ctrlFlyThrough.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

static wxStaticBox* constructColourSchemePanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Colour Scheme"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto lblShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 getColour(int i) {");
  auto ctrlShaderCode = constructTextField(box, "  ", true);
  auto lblShaderCodePost = new wxStaticText(box, wxID_ANY, "}");

  vbox->Add(lblShaderCodePre);
  vbox->Add(ctrlShaderCode.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(lblShaderCodePost);

  return box;
}

wxStaticBox* MainWindow::constructParamsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY,
                             wxGetTranslation("Render Params"));

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  auto ctrlMaxIterations = constructTextField(box, "Max iterations");
  m_txtMaxIterations = dynamic_cast<wxTextCtrl*>(ctrlMaxIterations.widget);
  m_txtMaxIterations->AppendText(std::to_string(DEFAULT_MAX_ITERATIONS));

  vbox->Add(ctrlMaxIterations.hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

void MainWindow::constructRightPanel() {
  m_rightPanel = new wxPanel(m_splitter, wxID_ANY);

  auto grid = new wxGridBagSizer(6, 6);
  m_rightPanel->SetSizer(grid);

  auto flyThroughPanel = constructFlyThroughPanel(m_rightPanel);
  auto colourSchemePanel = constructColourSchemePanel(m_rightPanel);
  auto paramsPanel = constructParamsPanel(m_rightPanel);

  auto btnApply = new wxButton(m_rightPanel, wxID_ANY, "Apply");
  btnApply->Bind(wxEVT_BUTTON, &MainWindow::onBtnApplyClick, this);

  grid->Add(flyThroughPanel, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND);
  grid->Add(colourSchemePanel, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND);
  grid->Add(paramsPanel, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND);
  grid->Add(btnApply, wxGBPosition(3, 1), wxGBSpan(1, 1),
            wxEXPAND | wxLEFT | wxRIGHT);

  grid->AddGrowableCol(1, 1);
  grid->AddGrowableRow(0, 1);
  grid->AddGrowableRow(1, 1);
  grid->AddGrowableRow(2, 1);
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

void MainWindow::onBtnApplyClick(wxCommandEvent& e) {
  int maxI = 0;
  if (tryGetIntFromTextCtrl(*m_txtMaxIterations, maxI)) {
    m_mandelbrot->setMaxIterations(maxI);
    m_canvas->Refresh();
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

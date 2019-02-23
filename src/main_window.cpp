#include <sstream>
#include "main_window.hpp"
#include "config.hpp"

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(wxID_EXIT, MainWindow::onExit)
  EVT_MENU(wxID_ABOUT, MainWindow::onAbout)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {

  m_mandelbrot.reset(new Mandelbrot(400, 400));

  m_hbox = new wxBoxSizer(wxHORIZONTAL);
  SetSizer(m_hbox);
  SetAutoLayout(true);

  constructMenu();
  constructGlCanvas();

  CreateStatusBar();
  SetStatusText("");
}

void MainWindow::constructGlCanvas() {
  int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
  m_canvas = new Canvas(this, args, *m_mandelbrot);
  m_canvas->Bind(FLY_THROUGH_MODE_TOGGLED, &MainWindow::onFlyThroughModeToggle,
                 this);

  m_hbox->Add(m_canvas, 1, wxEXPAND);
}

void MainWindow::constructMenu() {
  wxMenu* mnuFile = new wxMenu;
  mnuFile->Append(wxID_EXIT);
  
  wxMenu* mnuHelp = new wxMenu;
  mnuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(mnuFile, "&File");
  menuBar->Append(mnuHelp, "&Help");

  SetMenuBar(menuBar);
}

void MainWindow::onFlyThroughModeToggle(wxCommandEvent& e) {
  if (e.GetInt() == TOGGLED_ON) {
    SetStatusText("Fly through mode activated");
  }
  else if (e.GetInt() == TOGGLED_OFF) {
    SetStatusText("Fly through mode deactivated");
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
  ss << "The Mandelbrot fractal rendered on the GPU inside a fragment shader"
     << std::endl << std::endl;
  ss << "Author: Rob Jinman <jinmanr@gmail.com>" << std::endl << std::endl;
  ss << "Copyright Rob Jinman 2019. All rights reserved.";

  wxMessageBox(ss.str(), versionString(), wxOK | wxICON_INFORMATION);
}

class Application : public wxApp {
public:
  virtual bool OnInit() override {
    MainWindow* frame = new MainWindow(versionString(), wxSize(400, 400));
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

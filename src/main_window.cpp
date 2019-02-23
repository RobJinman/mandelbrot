#include "main_window.hpp"

enum {
  ID_hello = 1
};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(ID_hello, MainWindow::onHello)
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
  SetStatusText("Welcome to wxWidgets!");
}

void MainWindow::constructGlCanvas() {
  int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
  m_glPane = new Canvas(this, args, *m_mandelbrot);

  m_hbox->Add(m_glPane, 1, wxEXPAND);
}

void MainWindow::constructMenu() {
  wxMenu* mnuFile = new wxMenu;
  mnuFile->Append(ID_hello, "&hello...\tCtrl-H",
                  "Help string shown in status bar for this menu item");
  mnuFile->AppendSeparator();
  mnuFile->Append(wxID_EXIT);
  
  wxMenu* mnuHelp = new wxMenu;
  mnuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(mnuFile, "&File");
  menuBar->Append(mnuHelp, "&Help");

  SetMenuBar(menuBar);
}

void MainWindow::onHello(wxCommandEvent& event) {
  wxLogMessage("Hello world from wxWidgets!");
}

void MainWindow::onExit(wxCommandEvent& event) {
  Close(true);
}

void MainWindow::onAbout(wxCommandEvent& event) {
  wxMessageBox("Blah blah", "Hello", wxOK | wxICON_INFORMATION);
}

class MyApp : public wxApp {
public:
  virtual bool OnInit() override {
    MainWindow* frame = new MainWindow("Mandelbrot", wxSize(400, 400));
    frame->Show();
    return true;
  }
};

wxIMPLEMENT_APP(MyApp);

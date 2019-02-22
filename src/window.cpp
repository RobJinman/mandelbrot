#include "window.hpp"

MyFrame::MyFrame(const wxString& title, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {

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

  CreateStatusBar();
  SetStatusText("Welcome to wxWidgets!");
}

void MyFrame::onHello(wxCommandEvent& event) {
  wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::onExit(wxCommandEvent& event) {
  Close(true);
}

void MyFrame::onAbout(wxCommandEvent& event) {
  wxMessageBox("Blah blah", "Hello", wxOK | wxICON_INFORMATION);
}

class MyApp : public wxApp {
public:
  virtual bool OnInit() override {
    MyFrame* frame = new MyFrame("Mandelbrot", wxSize(400, 400));
    frame->Show();
    return true;
  }
};

//wxIMPLEMENT_APP(MyApp);

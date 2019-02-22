#pragma once

#include <wx/wx.h>

class MyFrame : public wxFrame {
public:
  MyFrame(const wxString& title, const wxSize& size);

private:
  void onHello(wxCommandEvent& event);
  void onExit(wxCommandEvent& event);
  void onAbout(wxCommandEvent& event);

  wxDECLARE_EVENT_TABLE();
};

enum {
  ID_hello = 1
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(ID_hello, MyFrame::onHello)
  EVT_MENU(wxID_EXIT, MyFrame::onExit)
  EVT_MENU(wxID_ABOUT, MyFrame::onAbout)
wxEND_EVENT_TABLE()

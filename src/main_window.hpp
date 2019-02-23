#pragma once

#include <memory>
#include <wx/wx.h>
#include "canvas.hpp"
#include "mandelbrot.hpp"

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructGlCanvas();

  void onHello(wxCommandEvent& event);
  void onExit(wxCommandEvent& event);
  void onAbout(wxCommandEvent& event);

  std::unique_ptr<Mandelbrot> m_mandelbrot;
  wxBoxSizer* m_hbox;
  Canvas* m_glPane;

  wxDECLARE_EVENT_TABLE();
};

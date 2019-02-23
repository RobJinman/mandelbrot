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

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);

  std::unique_ptr<Mandelbrot> m_mandelbrot;
  wxBoxSizer* m_hbox;
  Canvas* m_canvas;

  wxDECLARE_EVENT_TABLE();
};

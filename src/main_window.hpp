#pragma once

#include <memory>
#include <wx/wx.h>
#include <wx/splitter.h>
#include "canvas.hpp"
#include "mandelbrot.hpp"

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructLeftPanel();
  void constructRightPanel();
  wxStaticBox* constructParamsPanel(wxWindow* parent);

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onBtnApplyClick(wxCommandEvent& e);

  std::unique_ptr<Mandelbrot> m_mandelbrot;
  wxSplitterWindow* m_splitter;
  wxBoxSizer* m_vbox;
  wxPanel* m_rightPanel;
  Canvas* m_canvas;
  wxTextCtrl* m_txtMaxIterations;

  wxDECLARE_EVENT_TABLE();
};

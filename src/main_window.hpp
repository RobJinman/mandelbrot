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
  wxStaticBox* constructFlyThroughPanel();
  wxStaticBox* constructColourSchemePanel();
  wxStaticBox* constructParamsPanel();

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParamsClick(wxCommandEvent& e);
  void onApplyColourSchemeClick(wxCommandEvent& e);

  std::unique_ptr<Mandelbrot> m_mandelbrot;
  wxSplitterWindow* m_splitter;
  wxBoxSizer* m_vbox;
  wxPanel* m_rightPanel;
  Canvas* m_canvas;
  wxTextCtrl* m_txtMaxIterations;
  wxTextCtrl* m_txtComputeColourImpl;
  wxTextCtrl* m_txtComputeColourImplCompileStatus;

  wxDECLARE_EVENT_TABLE();
};

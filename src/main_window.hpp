#pragma once

#include <memory>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include "canvas.hpp"
#include "mandelbrot.hpp"

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructLeftPanel();
  void constructRightPanel();
  wxStaticBox* constructFlyThroughPanel(wxWindow* parent);
  wxStaticBox* constructColourSchemePanel(wxWindow* parent);
  wxStaticBox* constructParamsPanel(wxWindow* parent);
  wxStaticBox* constructInfoPanel(wxWindow* parent);
  void constructInfoPage();
  void constructParamsPage();
  void constructColourSchemePage();

  void onRender();

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParamsClick(wxCommandEvent& e);
  void onApplyColourSchemeClick(wxCommandEvent& e);

  std::unique_ptr<Mandelbrot> m_mandelbrot;
  wxSplitterWindow* m_splitter;
  wxBoxSizer* m_vbox;
  wxNotebook* m_rightPanel;
  Canvas* m_canvas;
  wxTextCtrl* m_txtMaxIterations;
  wxTextCtrl* m_txtComputeColourImpl;
  wxTextCtrl* m_txtComputeColourImplCompileStatus;

  struct {
    wxStaticText* txtMagLevel;
    wxStaticText* txtXMin;
    wxStaticText* txtXMax;
    wxStaticText* txtYMin;
    wxStaticText* txtYMax;
  } m_dataFields;

  wxDECLARE_EVENT_TABLE();
};

#pragma once

#include <memory>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include "canvas.hpp"
#include "renderer.hpp"

const int WINDOW_W = 1000;
const int WINDOW_H = 600;

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructLeftPanel();
  void constructRightPanel();
  wxStaticBox* constructInfoPanel(wxWindow* parent);
  wxStaticBox* constructColourSchemePanel(wxWindow* parent);
  wxStaticBox* constructRenderParamsPanel(wxWindow* parent);
  wxStaticBox* constructFlyThroughParamsPanel(wxWindow* parent);
  wxStaticBox* constructDataPanel(wxWindow* parent);
  wxStaticBox* constructExportPanel(wxWindow* parent);
  void constructInfoPage();
  void constructParamsPage();
  void constructColourSchemePage();
  void constructExportPage();

  void onRender();
  void applyColourScheme();
  void adjustExportSize(bool adjustWidth);
  void makeGlContextCurrent();

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParamsClick(wxCommandEvent& e);
  void onApplyColourSchemeClick(wxCommandEvent& e);
  void onExportClick(wxCommandEvent& e);
  void onSelectColourScheme(wxCommandEvent& e);
  void onCanvasResize(wxSizeEvent& e);
  void onExportHeightChange(wxCommandEvent& e);
  void onExportWidthChange(wxCommandEvent& e);
  void onCanvasGainFocus(wxFocusEvent& e);
  void onCanvasLoseFocus(wxFocusEvent& e);

  std::unique_ptr<Renderer> m_renderer;
  wxString m_exportFilePath;
  wxSplitterWindow* m_splitter;
  wxBoxSizer* m_vbox;
  wxNotebook* m_rightPanel;
  wxPanel* m_leftPanel;
  Canvas* m_canvas;
  wxTextCtrl* m_txtMaxIterations;
  wxTextCtrl* m_txtZoomAmount;
  wxTextCtrl* m_txtTargetFps;
  wxTextCtrl* m_txtZoomPerFrame;
  wxTextCtrl* m_txtComputeColourImpl;
  wxTextCtrl* m_txtCompileStatus;
  wxTextCtrl* m_txtExportWidth;
  wxTextCtrl* m_txtExportHeight;
  wxGauge* m_exportProgressBar;

  struct {
    wxStaticText* txtMagLevel;
    wxStaticText* txtXMin;
    wxStaticText* txtXMax;
    wxStaticText* txtYMin;
    wxStaticText* txtYMax;
  } m_dataFields;

  wxDECLARE_EVENT_TABLE();
};

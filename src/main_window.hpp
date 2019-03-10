#pragma once

#include <memory>
#include <map>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include "canvas.hpp"
#include "renderer.hpp"
#include "colour_scheme_panel.hpp"

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
  ColourSchemePanel* constructColourSchemePanel(wxWindow* parent);
  wxStaticBox* constructRenderParamsPanel(wxWindow* parent);
  wxStaticBox* constructFlyThroughParamsPanel(wxWindow* parent);
  wxStaticBox* constructDataPanel(wxWindow* parent);
  wxStaticBox* constructExportPanel(wxWindow* parent);
  void constructInfoPage();
  void constructParamsPage();
  void constructColourSchemePage();
  void constructExportPage();

  void onRender();
  void adjustExportSize(bool adjustWidth);
  void makeGlContextCurrent();
  uint8_t* beginExport(int w, int h);
  void endExport(const wxString& exportFilePath, int w, int h, uint8_t* data);
  void applyColourScheme(const std::string& code);

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParamsClick(wxCommandEvent& e);
  void onExportClick(wxCommandEvent& e);
  void onCanvasResize(wxSizeEvent& e);
  void onExportHeightChange(wxCommandEvent& e);
  void onExportWidthChange(wxCommandEvent& e);
  void onCanvasGainFocus(wxFocusEvent& e);
  void onCanvasLoseFocus(wxFocusEvent& e);
  void onClose(wxCloseEvent& e);

  bool m_quitting = false;
  std::unique_ptr<Renderer> m_renderer;
  wxSplitterWindow* m_splitter;
  wxBoxSizer* m_vbox;
  wxNotebook* m_rightPanel;
  wxPanel* m_leftPanel;
  Canvas* m_canvas;
  ColourSchemePanel* m_colourSchemePanel;
  
  struct {
    wxTextCtrl* txtMaxIterations;
    wxTextCtrl* txtZoomAmount;
    wxTextCtrl* txtTargetFps;
    wxTextCtrl* txtZoomPerFrame;
    wxButton* btnApply;
  } m_params;

  struct {
    bool busy = false;
    wxTextCtrl* txtWidth;
    wxTextCtrl* txtHeight;
    wxButton* btnExport;
    wxGauge* progressBar;
  } m_export;

  struct {
    wxStaticText* txtMagLevel;
    wxStaticText* txtXMin;
    wxStaticText* txtXMax;
    wxStaticText* txtYMin;
    wxStaticText* txtYMax;
  } m_dataFields;

  wxDECLARE_EVENT_TABLE();
};

#pragma once

#include <memory>
#include <map>
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include "canvas.hpp"
#include "renderer.hpp"

const int WINDOW_W = 1000;
const int WINDOW_H = 600;

typedef std::map<std::string, std::string> StringMap;

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
  void loadColourSchemes();
  void saveColourSchemes();
  void selectColourScheme(const wxString& name);
  void selectColourScheme(int idx);
  void applyColourScheme();
  void updateColourSchemeSelector();
  void adjustExportSize(bool adjustWidth);
  void makeGlContextCurrent();
  uint8_t* beginExport(int w, int h);
  void endExport(const wxString& exportFilePath, int w, int h, uint8_t* data);

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParamsClick(wxCommandEvent& e);
  void onApplyColourSchemeClick(wxCommandEvent& e);
  void onDeleteColourSchemeClick(wxCommandEvent& e);
  void onRestoreColourSchemeClick(wxCommandEvent& e);
  void onSaveColourSchemeClick(wxCommandEvent& e);
  void onExportClick(wxCommandEvent& e);
  void onSelectColourScheme(wxCommandEvent& e);
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
  
  struct {
    wxTextCtrl* txtMaxIterations;
    wxTextCtrl* txtZoomAmount;
    wxTextCtrl* txtTargetFps;
    wxTextCtrl* txtZoomPerFrame;
    wxButton* btnApply;
  } m_params;

  struct {
    wxString filePath;
    wxComboBox* cboSelector;
    wxTextCtrl* txtCode;
    wxTextCtrl* txtCompileStatus;
    wxButton* btnApply;
    wxButton* btnDelete;
    wxButton* btnRestore;
    wxButton* btnSave;
    StringMap colourSchemes;
  } m_colourScheme;

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

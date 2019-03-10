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

class ColourSchemePage;
class InfoPage;
class ParamsPage;
class ApplyParamsEvent;

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructLeftPanel();
  void constructRightPanel();
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
  void onApplyParams(ApplyParamsEvent& e);
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
  ColourSchemePage* m_colourSchemePage;
  InfoPage* m_infoPage;
  ParamsPage* m_paramsPage;

  struct {
    bool busy = false;
    wxTextCtrl* txtWidth;
    wxTextCtrl* txtHeight;
    wxButton* btnExport;
    wxGauge* progressBar;
  } m_export;

  wxDECLARE_EVENT_TABLE();
};

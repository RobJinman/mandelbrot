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
class ExportPage;
class ExportEvent;
class ParamsPage;
class ApplyParamsEvent;

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString& title, const wxSize& size);

private:
  void constructMenu();
  void constructLeftPanel();
  void constructRightPanel();
  void constructInfoPage();
  void constructParamsPage();
  void constructColourSchemePage();
  void constructExportPage();

  void onRender();
  void makeGlContextCurrent();
  void applyColourScheme(const std::string& code);
  uint8_t* beginExport(int w, int h);
  void endExport(const wxString& exportFilePath, int w, int h, uint8_t* data);

  void onExit(wxCommandEvent& e);
  void onAbout(wxCommandEvent& e);
  void onFlyThroughModeToggle(wxCommandEvent& e);
  void onApplyParams(ApplyParamsEvent& e);
  void onExport(ExportEvent& e);
  void onCanvasResize(wxSizeEvent& e);
  void onCanvasGainFocus(wxFocusEvent& e);
  void onCanvasLoseFocus(wxFocusEvent& e);
  void onClose(wxCloseEvent& e);

  bool m_quitting = false;
  bool m_doingExport = false;
  std::unique_ptr<Renderer> m_renderer;
  wxSplitterWindow* m_splitter = nullptr;
  wxBoxSizer* m_vbox = nullptr;
  wxNotebook* m_rightPanel = nullptr;
  wxPanel* m_leftPanel = nullptr;
  Canvas* m_canvas = nullptr;
  ColourSchemePage* m_colourSchemePage = nullptr;
  InfoPage* m_infoPage = nullptr;
  ParamsPage* m_paramsPage = nullptr;
  ExportPage* m_exportPage = nullptr;

  wxDECLARE_EVENT_TABLE();
};

#include <sstream>
#include "main_window.hpp"
#include "config.hpp"
#include "exception.hpp"
#include "wx_helpers.hpp"
#include "utils.hpp"
#include "colour_scheme_page.hpp"
#include "info_page.hpp"
#include "params_page.hpp"
#include "locations_page.hpp"
#include "export_page.hpp"

using std::string;

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(wxID_EXIT, MainWindow::onExit)
  EVT_MENU(wxID_ABOUT, MainWindow::onAbout)
  EVT_CLOSE(MainWindow::onClose)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {

  m_renderer.reset(new Renderer([this]() {
    makeGlContextCurrent();
  }));

  m_vbox = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_vbox);

  SetAutoLayout(true);

  m_splitter = new wxSplitterWindow(this);
  m_splitter->SetMinimumPaneSize(300);

  m_vbox->Add(m_splitter, 1, wxEXPAND, 0);

  constructMenu();
  constructLeftPanel();
  constructRightPanel();

  m_splitter->SplitVertically(m_leftPanel, m_rightPanel, 0.55 * WINDOW_W);

  CreateStatusBar();
  SetStatusText(wxEmptyString);
}

void MainWindow::onClose(wxCloseEvent& e) {
  m_quitting = true;

  if (m_doingExport) {
    e.Veto();
  }
  else {
    e.Skip();
  }
}

void MainWindow::makeGlContextCurrent() {
  if (m_canvas) {
    m_canvas->makeGlContextCurrent();
  }
}

void MainWindow::constructLeftPanel() {
  m_leftPanel = new wxPanel(m_splitter, wxID_ANY);
  auto vbox = new wxBoxSizer(wxVERTICAL);
  m_leftPanel->SetSizer(vbox);
  m_leftPanel->SetCanFocus(true);

  wxGLAttributes glAttrs;
  glAttrs.PlatformDefaults().Defaults().EndList();

  m_canvas = new Canvas(m_leftPanel, glAttrs, *m_renderer,
                        [this]() { onRender(); });
  m_canvas->Bind(FLY_THROUGH_MODE_TOGGLE_EVENT,
                 &MainWindow::onFlyThroughModeToggle, this);
  m_canvas->Bind(wxEVT_SIZE, &MainWindow::onCanvasResize, this);
  m_canvas->Bind(wxEVT_SET_FOCUS, &MainWindow::onCanvasGainFocus, this);
  m_canvas->Bind(wxEVT_KILL_FOCUS, &MainWindow::onCanvasLoseFocus, this);

  vbox->Add(m_canvas, 1, wxEXPAND | wxALL, 5);
}

void MainWindow::constructInfoPage() {
  m_infoPage = new InfoPage(m_rightPanel);
  m_rightPanel->AddPage(m_infoPage, wxGetTranslation("Info"));
}

void MainWindow::constructParamsPage() {
  m_paramsPage = new ParamsPage(m_rightPanel);
  m_paramsPage->Bind(APPLY_PARAMS_EVENT, &MainWindow::onApplyParams, this);

  m_rightPanel->AddPage(m_paramsPage, wxGetTranslation("Parameters"));
}

void MainWindow::constructColourSchemePage() {
  auto fn = [this](const string& code) {
    applyColourScheme(code);
  };

  m_colourSchemePage = new ColourSchemePage(m_rightPanel, fn);
  m_rightPanel->AddPage(m_colourSchemePage, wxGetTranslation("Colours"));
}

void MainWindow::constructLocationsPage() {
  m_locationsPage = new LocationsPage(m_rightPanel);
  m_locationsPage->Bind(APPLY_LOCATION_EVENT, &MainWindow::onApplyLocation,
                        this);
  m_rightPanel->AddPage(m_locationsPage, wxGetTranslation("Locations"));
}

void MainWindow::constructExportPage() {
  m_exportPage = new ExportPage(m_rightPanel);
  m_exportPage->Bind(EXPORT_EVENT, &MainWindow::onExport, this);

  m_rightPanel->AddPage(m_exportPage, wxGetTranslation("Export"));
}

void MainWindow::constructRightPanel() {
  m_rightPanel = new wxNotebook(m_splitter, wxID_ANY);
  constructInfoPage();
  constructParamsPage();
  constructColourSchemePage();
  constructLocationsPage();
  constructExportPage();
}

void MainWindow::constructMenu() {
  wxMenu* mnuFile = new wxMenu;
  mnuFile->Append(wxID_EXIT);

  wxMenu* mnuHelp = new wxMenu;
  mnuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(mnuFile, wxGetTranslation("&File"));
  menuBar->Append(mnuHelp, wxGetTranslation("&Help"));

  SetMenuBar(menuBar);
}

void MainWindow::onCanvasGainFocus(wxFocusEvent& e) {
  e.Skip();

  m_locationsPage->clearSelection();

  m_leftPanel->SetBackgroundColour(*wxBLUE);
  Refresh();
}

void MainWindow::onCanvasLoseFocus(wxFocusEvent& e) {
  e.Skip();

  m_leftPanel->SetBackgroundColour(*wxLIGHT_GREY);
  Refresh();
}

void MainWindow::onCanvasResize(wxSizeEvent& e) {
  e.Skip();

  if (m_exportPage) {
    m_exportPage->onCanvasSizeChange(m_canvas->GetClientSize().x,
                                     m_canvas->GetClientSize().y);
  }
}

void MainWindow::onRender() {
  m_infoPage->onRender(*m_renderer);
  m_locationsPage->onRender(*m_renderer);
}

uint8_t* MainWindow::beginExport(int w, int h) {
  m_doingExport = true;
  m_exportPage->setBusy(true);
  m_paramsPage->disable();
  m_colourSchemePage->disable();
  m_canvas->disable();
  SetStatusText(wxGetTranslation("Exporting to file..."));

  m_renderer->renderToMainMemoryBuffer(w, h);

  const OfflineRenderStatus& status = m_renderer->continueOfflineRender();

  while (status.progress != 100) {
    m_exportPage->setProgress(status.progress);
    wxYield();

    if (m_quitting) {
      m_doingExport = false;
      m_exportPage->setBusy(false);
      Close();
      return nullptr;
    }

    m_renderer->continueOfflineRender();
  }

  return status.data;
}

void MainWindow::endExport(const wxString& exportFilePath, int w, int h,
                           uint8_t* data) {
  if (data != nullptr) {
    wxImage image(w, h, data);
    image = image.Mirror(false);

    image.SaveFile(exportFilePath, wxBITMAP_TYPE_BMP);
  }

  m_doingExport = false;
  m_exportPage->setBusy(false);
  m_paramsPage->enable();
  m_colourSchemePage->enable();
  m_canvas->enable();
  SetStatusText(wxGetTranslation("Export complete"));

  m_canvas->refresh();
}

void MainWindow::onExport(ExportEvent& e) {
  uint8_t* data = beginExport(e.w, e.h);
  endExport(e.filePath, e.w, e.h, data);
}

void MainWindow::onApplyParams(ApplyParamsEvent& e) {
  m_renderer->setMaxIterations(e.maxI);
  m_canvas->setZoomAmount(e.zoomAmount);
  m_canvas->setTargetFps(e.targetFps);
  m_canvas->setZoomPerFrame(e.zoomPerFrame);

  m_canvas->refresh();
}

void MainWindow::onApplyLocation(ApplyLocationEvent& e) {
  m_renderer->resetZoom();
  m_renderer->graphSpaceZoom(e.x, e.y, e.magnification);
  m_canvas->refresh();
}

void MainWindow::applyColourScheme(const std::string& code) {
  m_renderer->setColourSchemeImpl(code);
  m_canvas->refresh();
}

void MainWindow::onFlyThroughModeToggle(wxCommandEvent& e) {
  if (e.GetInt() == TOGGLED_ON) {
    SetStatusText(wxGetTranslation("Fly-Through mode activated"));
  }
  else if (e.GetInt() == TOGGLED_OFF) {
    SetStatusText(wxGetTranslation("Fly-Through mode deactivated"));
  }
}

void MainWindow::onExit(wxCommandEvent&) {
  Close();
}

void MainWindow::onAbout(wxCommandEvent&) {
  std::stringstream ss;
  ss << "The Mandelbrot fractal rendered on the GPU." << std::endl << std::endl;
  ss << "Author: Rob Jinman <jinmanr@gmail.com>" << std::endl << std::endl;
  ss << "Copyright Rob Jinman 2019. All rights reserved.";

  wxMessageBox(wxGetTranslation(ss.str()), versionString(),
               wxOK | wxICON_INFORMATION);
}

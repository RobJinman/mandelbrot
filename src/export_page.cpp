#include <wx/gbsizer.h>
#include "export_page.hpp"
#include "utils.hpp"
#include "wx_helpers.hpp"

static const long DEFAULT_EXPORT_HEIGHT = 1000;
static const long MIN_EXPORT_WIDTH = 10;
static const long MAX_EXPORT_WIDTH = 10000;
static const long MIN_EXPORT_HEIGHT = 10;
static const long MAX_EXPORT_HEIGHT = 10000;

wxDEFINE_EVENT(EXPORT_EVENT, ExportEvent);

ExportEvent::ExportEvent(int w, int h, const wxString& filePath)
  : wxCommandEvent(EXPORT_EVENT),
    w(w),
    h(h),
    filePath(filePath) {}

ExportEvent::ExportEvent(const ExportEvent& cpy)
  : wxCommandEvent(cpy),
    w(cpy.w),
    h(cpy.h),
    filePath(cpy.filePath) {}

wxEvent* ExportEvent::Clone() const {
  return new ExportEvent(*this);
}

ExportPage::ExportPage(wxWindow* parent)
  : wxNotebookPage(parent, wxID_ANY) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  auto grid = new wxFlexGridSizer(2);
  SetSizer(vbox);

  auto lblWidth = constructLabel(this, wxGetTranslation("Width"));
  m_txtWidth = constructTextBox(this, wxEmptyString);
  m_txtWidth->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_txtWidth->Bind(wxEVT_TEXT, &ExportPage::onExportWidthChange, this);

  auto lblHeight = constructLabel(this, wxGetTranslation("Height"));
  m_txtHeight = constructTextBox(this, std::to_string(DEFAULT_EXPORT_HEIGHT));
  m_txtHeight->SetValidator(wxTextValidator(wxFILTER_DIGITS));
  m_txtHeight->Bind(wxEVT_TEXT, &ExportPage::onExportHeightChange, this);

  m_btnExport = new wxButton(this, wxID_ANY, wxGetTranslation("Export"));
  m_btnExport->Bind(wxEVT_BUTTON, &ExportPage::onExportClick, this);

  m_progressBar = new wxGauge(this, wxID_ANY, 100);

  grid->AddSpacer(20);
  grid->AddSpacer(20);
  grid->Add(lblWidth, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtWidth, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);
  grid->Add(lblHeight, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  grid->Add(m_txtHeight, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 10);
  grid->AddSpacer(10);
  grid->Add(m_btnExport, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

  vbox->Add(grid, 0, wxEXPAND | wxBOTTOM, 10);
  vbox->Add(m_progressBar, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM |
                              wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);

  m_progressBar->Hide();
}

void ExportPage::onCanvasSizeChange(int w, int h) {
  m_canvasW = w;
  m_canvasH = h;
  adjustExportSize(true);
}

void ExportPage::onExportHeightChange(wxCommandEvent&) {
  adjustExportSize(true);
}

void ExportPage::onExportWidthChange(wxCommandEvent&) {
  adjustExportSize(false);
}

void ExportPage::adjustExportSize(bool adjustWidth) {
  float aspect = m_canvasW / m_canvasH;

  if (adjustWidth) {
    long exportH = 0;
    m_txtHeight->GetValue().ToLong(&exportH);

    long exportW = exportH * aspect;
    m_txtWidth->ChangeValue(std::to_string(exportW));
  }
  else {
    long exportW = 0;
    m_txtWidth->GetValue().ToLong(&exportW);

    long exportH = exportW / aspect;
    m_txtHeight->ChangeValue(std::to_string(exportH));
  }
}

void ExportPage::setBusy(bool busy) {
  if (busy) {
    m_progressBar->Show();
    m_btnExport->Disable();
  }
  else {
    m_progressBar->Hide();
    m_btnExport->Enable();
  }
}

void ExportPage::onExportClick(wxCommandEvent&) {
  wxFileDialog fileDialog(this, wxGetTranslation("Save as BMP image"), "", "",
                          "BMP files (*.bmp)|*.bmp", wxFD_SAVE);
  if (fileDialog.ShowModal() == wxID_CANCEL) {
    return;
  }

  long w = getBoundedValue<long>(*m_txtWidth, MIN_EXPORT_WIDTH,
                                 MAX_EXPORT_WIDTH);

  long h = getBoundedValue<long>(*m_txtHeight, MIN_EXPORT_HEIGHT,
                                 MAX_EXPORT_HEIGHT);

  ExportEvent event(w, h, fileDialog.GetPath());
  wxPostEvent(this, event);
}

void ExportPage::setProgress(int progress) {
  m_progressBar->SetValue(progress);
}

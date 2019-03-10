#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>

class ExportEvent;

wxDECLARE_EVENT(EXPORT_EVENT, ExportEvent);

class ExportEvent : public wxCommandEvent {
public:
  ExportEvent(int w, int h, const wxString& filePath);
  ExportEvent(const ExportEvent& cpy);

  wxEvent* Clone() const override;

  int w;
  int h;
  wxString filePath;
};

class ExportPage : public wxNotebookPage {
public:
  ExportPage(wxWindow* parent);

  void setBusy(bool busy);
  void setProgress(int progress);
  void disable();
  void enable();

  void onCanvasSizeChange(int w, int h);

private:
  wxStaticBox* constructExportPanel(wxWindow* parent);
  uint8_t* beginExport(int w, int h);
  void endExport(const wxString& exportFilePath, int w, int h, uint8_t* data);
  void adjustExportSize(bool adjustWidth);

  void onExportClick(wxCommandEvent& e);
  void onExportHeightChange(wxCommandEvent& e);
  void onExportWidthChange(wxCommandEvent& e);

  wxTextCtrl* m_txtWidth;
  wxTextCtrl* m_txtHeight;
  wxButton* m_btnExport;
  wxGauge* m_progressBar;
  float m_canvasW = 0.f;
  float m_canvasH = 0.f;
};

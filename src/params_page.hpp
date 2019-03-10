#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>

class ApplyParamsEvent;

wxDECLARE_EVENT(APPLY_PARAMS_EVENT, ApplyParamsEvent);

class ApplyParamsEvent : public wxCommandEvent {
public:
  ApplyParamsEvent(int maxI, double zoomAmount, double targetFps,
                   double zoomPerFrame);
  
  ApplyParamsEvent(const ApplyParamsEvent& cpy);

  wxEvent* Clone() const override;

  int maxI;
  double zoomAmount;
  double targetFps;
  double zoomPerFrame;
};

class ParamsPage : public wxNotebookPage {
public:
  ParamsPage(wxWindow* parent);

  void disable();
  void enable();

private:
  wxStaticBox* constructRenderParamsPanel(wxWindow* parent);
  wxStaticBox* constructFlyThroughParamsPanel(wxWindow* parent);

  void onApplyParamsClick(wxCommandEvent& e);

  wxTextCtrl* m_txtMaxIterations;
  wxTextCtrl* m_txtZoomAmount;
  wxTextCtrl* m_txtTargetFps;
  wxTextCtrl* m_txtZoomPerFrame;
  wxButton* m_btnApply;
};

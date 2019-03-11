#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>

class LocationsPage : public wxNotebookPage {
public:
  LocationsPage(wxWindow* parent);

  void disable();
  void enable();

private:
  wxStaticBox* constructLocationsPanel(wxWindow* parent);

  void onApplyClick(wxCommandEvent& e);

  wxTextCtrl* m_txtX;
  wxTextCtrl* m_txtY;
  wxTextCtrl* m_txtMagnification;
  wxButton* m_btnApply;
  wxComboBox* m_cboFavourites;
};

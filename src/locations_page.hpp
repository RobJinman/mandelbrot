#pragma once

#include <map>
#include <string>
#include <wx/wx.h>
#include <wx/notebook.h>

class ApplyLocationEvent;

wxDECLARE_EVENT(APPLY_LOCATION_EVENT, ApplyLocationEvent);

class ApplyLocationEvent : public wxCommandEvent {
public:
  ApplyLocationEvent(double x, double y, double magnification);
  ApplyLocationEvent(const ApplyLocationEvent& cpy);

  wxEvent* Clone() const override;

  double x;
  double y;
  double magnification;
};

class Renderer;

class LocationsPage : public wxNotebookPage {
public:
  LocationsPage(wxWindow* parent);

  void onRender(const Renderer& renderer);
  void disable();
  void enable();

private:
  struct Location {
    double x = 0.0;
    double y = 0.0;
    double magnification = 1.0;
  };

  wxStaticBoxSizer* constructCurrentPanel(wxWindow* parent);
  wxStaticBoxSizer* constructFavouritesPanel(wxWindow* parent);
  void loadLocations();
  void saveLocations();
  void populateFields(const Location& loc);
  LocationsPage::Location getCurrentLocation() const;
  void updateFavouritesSelector();
  void showHideButtons();

  void onApplyClick(wxCommandEvent& e);
  void onSelectLocation(wxCommandEvent& e);
  void onLocationTextChange(wxCommandEvent& e);
  void onAddClick(wxCommandEvent& e);
  void onDeleteClick(wxCommandEvent& e);

  wxTextCtrl* m_txtX;
  wxTextCtrl* m_txtY;
  wxTextCtrl* m_txtMagnification;
  wxButton* m_btnApply;
  wxComboBox* m_cboFavourites;
  wxButton* m_btnAdd;
  wxButton* m_btnDelete;
  wxString m_filePath;
  std::map<std::string, Location> m_locations;
};

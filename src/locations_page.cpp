#include "locations_page.hpp"
#include "wx_helpers.hpp"

LocationsPage::LocationsPage(wxWindow* parent)
  : wxNotebookPage(parent, wxID_ANY) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructLocationsPanel(this), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  SetSizer(vbox);
}

wxStaticBox* LocationsPage::constructLocationsPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto grid = new wxFlexGridSizer(2);
  box->SetSizer(grid);

  auto lblX = constructLabel(box, "X");
  m_txtX = constructTextBox(box, "0");
  m_txtX->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  auto lblY = constructLabel(box, "Y");
  m_txtY = constructTextBox(box, "0");
  m_txtY->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  auto lblMagnification = constructLabel(box, "Magnification");
  m_txtMagnification = constructTextBox(box, "1.0");
  m_txtMagnification->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  m_btnApply = new wxButton(box, wxID_ANY, wxGetTranslation("Apply"));
  m_btnApply->Bind(wxEVT_BUTTON, &LocationsPage::onApplyClick, this);

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblX, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtX, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblY, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtY, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblMagnification, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtMagnification, 0, wxEXPAND | wxRIGHT, 10);
  grid->AddSpacer(10);
  grid->Add(m_btnApply, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);

  return box;
}

void LocationsPage::onApplyClick(wxCommandEvent&) {
  // TODO
}

void LocationsPage::disable() {
  m_btnApply->Disable();
}

void LocationsPage::enable() {
  m_btnApply->Enable();
}

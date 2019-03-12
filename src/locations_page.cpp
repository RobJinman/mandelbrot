#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/xml/xml.h>
#include "locations_page.hpp"
#include "wx_helpers.hpp"
#include "renderer.hpp"
#include "utils.hpp"

using std::string;

wxDEFINE_EVENT(APPLY_LOCATION_EVENT, ApplyLocationEvent);

ApplyLocationEvent::ApplyLocationEvent(double x, double y, double magnification)
  : wxCommandEvent(APPLY_LOCATION_EVENT),
    x(x),
    y(y),
    magnification(magnification) {}

ApplyLocationEvent::ApplyLocationEvent(const ApplyLocationEvent& cpy)
  : wxCommandEvent(cpy),
    x(cpy.x),
    y(cpy.y),
    magnification(cpy.magnification) {}

wxEvent* ApplyLocationEvent::Clone() const {
  return new ApplyLocationEvent(*this);
}

LocationsPage::LocationsPage(wxWindow* parent)
  : wxNotebookPage(parent, wxID_ANY) {

  loadLocations();

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructCurrentPanel(this), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(constructFavouritesPanel(this), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  updateFavouritesSelector();

  SetSizer(vbox);
}

wxStaticBox* LocationsPage::constructCurrentPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxGetTranslation("Current"));

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
  grid->Add(m_btnApply, 0, wxRIGHT | wxALIGN_RIGHT, 10);

  grid->AddGrowableCol(0);
  grid->AddGrowableCol(1);

  return box;
}

wxStaticBox* LocationsPage::constructFavouritesPanel(wxWindow* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxGetTranslation("Favourites"));

  auto lblName = constructLabel(box, wxGetTranslation("Location name"));
  m_cboFavourites = new wxComboBox(box, wxID_ANY);
  m_cboFavourites->Bind(wxEVT_TEXT, &LocationsPage::onLocationTextChange, this);
  m_cboFavourites->Bind(wxEVT_COMBOBOX, &LocationsPage::onSelectLocation, this);

  m_btnDelete = new wxButton(box, wxID_ANY, wxGetTranslation("Delete"));
  m_btnDelete->Bind(wxEVT_BUTTON, &LocationsPage::onDeleteClick, this);
  m_btnDelete->Hide();

  m_btnAdd = new wxButton(box, wxID_ANY, wxGetTranslation("Add current"));
  m_btnAdd->Bind(wxEVT_BUTTON, &LocationsPage::onAddClick, this);
  m_btnAdd->Disable();

  auto hbox = new wxBoxSizer(wxHORIZONTAL);
  hbox->AddStretchSpacer();
  hbox->Add(m_btnDelete, 0, wxRIGHT, 10);
  hbox->Add(m_btnAdd, 0, wxRIGHT, 10);

  auto grid = new wxFlexGridSizer(2);
  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblName, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_cboFavourites, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(0);
  grid->AddGrowableCol(1);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(grid, 0, wxEXPAND);
  vbox->Add(hbox, 0, wxEXPAND);
  box->SetSizer(vbox);

  return box;
}

void LocationsPage::onRender(const Renderer& renderer) {
  auto magLevel = formatDouble(renderer.computeMagnification());
  m_txtMagnification->ChangeValue(magLevel);

  double xMin = renderer.getXMin();
  double xMax = renderer.getXMax();
  double yMin = renderer.getYMin();
  double yMax = renderer.getYMax();

  double x = xMin + 0.5 * (xMax - xMin);
  double y = yMin + 0.5 * (yMax - yMin);

  m_txtX->ChangeValue(formatDouble(x));
  m_txtY->ChangeValue(formatDouble(y));
}

void LocationsPage::loadLocations() {
  m_filePath = userDataPath("locations.xml");

  if (wxFile::Exists(m_filePath)) {
    wxXmlDocument doc;
    doc.Load(m_filePath);

    auto xmlLocation = doc.GetRoot()->GetChildren();
    while (xmlLocation) {
      wxString name = xmlLocation->GetAttribute("name");
      wxString strX = xmlLocation->GetAttribute("x");
      wxString strY = xmlLocation->GetAttribute("y");
      wxString strMag = xmlLocation->GetAttribute("magnification");

      double x = 0;
      double y = 0;
      double mag = 1.0;

      strX.ToDouble(&x);
      strY.ToDouble(&y);
      strMag.ToDouble(&mag);

      m_locations[name.ToStdString()] = Location{x, y, mag};

      xmlLocation = xmlLocation->GetNext();
    }
  }
}

void LocationsPage::saveLocations() {
  if (!wxFile::Exists(m_filePath)) {
    wxDir::Make(userDataPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  }

  wxXmlDocument doc;
  auto root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "locations");

  for (auto entry : m_locations) {
    const wxString& name = entry.first;
    const Location& loc = entry.second;

    auto xmlLocation = new wxXmlNode(root, wxXML_ELEMENT_NODE, "location");

    xmlLocation->AddAttribute("name", name);
    xmlLocation->AddAttribute("x", std::to_string(loc.x));
    xmlLocation->AddAttribute("y", std::to_string(loc.y));
    xmlLocation->AddAttribute("magnification",
                              std::to_string(loc.magnification));
  }

  doc.SetRoot(root);
  doc.Save(m_filePath);
}

void LocationsPage::populateFields(const LocationsPage::Location& loc) {
  m_txtX->ChangeValue(std::to_string(loc.x));
  m_txtY->ChangeValue(std::to_string(loc.y));
  m_txtMagnification->ChangeValue(std::to_string(loc.magnification));
}

LocationsPage::Location LocationsPage::getCurrentLocation() const {
  double x = getValue<double>(*m_txtX);
  double y = getValue<double>(*m_txtY);
  double mag = getMinValue<double>(*m_txtMagnification, 0);

  return Location{x, y, mag};
}

void LocationsPage::updateFavouritesSelector() {
  auto& cbo = *m_cboFavourites;

  std::vector<wxString> names;
  for (auto entry : m_locations) {
    names.push_back(entry.first);
  }

  cbo.Clear();

  if (names.size() > 0) {
    cbo.Insert(names, 0);
  }
}

void LocationsPage::onSelectLocation(wxCommandEvent&) {
  showHideButtons();

  string name = m_cboFavourites->GetValue().ToStdString();

  const Location& loc = m_locations.at(name);
  populateFields(loc);

  ApplyLocationEvent event(loc.x, loc.y, loc.magnification);
  wxPostEvent(this, event);
}

void LocationsPage::onLocationTextChange(wxCommandEvent&) {
  showHideButtons();
}

void LocationsPage::showHideButtons() {
  string name = m_cboFavourites->GetValue().ToStdString();

  bool exists = m_locations.count(name);

  m_btnAdd->Enable(!exists && name.length() > 0);
  m_btnDelete->Show(exists);

  m_btnDelete->GetParent()->Layout();
}

void LocationsPage::onAddClick(wxCommandEvent&) {
  string name = m_cboFavourites->GetValue().ToStdString();
  m_locations[name] = getCurrentLocation();

  updateFavouritesSelector();

  saveLocations();
}

void LocationsPage::onApplyClick(wxCommandEvent&) {
  double x = getValue<double>(*m_txtX);
  double y = getValue<double>(*m_txtY);
  double mag = getMinValue<double>(*m_txtMagnification, 0);

  ApplyLocationEvent event(x, y, mag);
  wxPostEvent(this, event);
}

void LocationsPage::onDeleteClick(wxCommandEvent&) {
  string name = m_cboFavourites->GetValue().ToStdString();
  m_locations.erase(name);

  updateFavouritesSelector();
  showHideButtons();

  saveLocations();
}

void LocationsPage::disable() {
  m_btnApply->Disable();
  m_btnDelete->Disable();
}

void LocationsPage::enable() {
  m_btnApply->Enable();
  m_btnDelete->Enable();

  showHideButtons();
}

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/xml/xml.h>
#include "colour_scheme_page.hpp"
#include "wx_helpers.hpp"
#include "defaults.hpp"
#include "exception.hpp"
#include "utils.hpp"

using std::string;

ColourSchemePage::ColourSchemePage(wxWindow* parent,
                                   fnApplyColourScheme_t fnApplyColourScheme)
  : wxNotebookPage(parent, wxID_ANY),
    m_fnApplyColourScheme(fnApplyColourScheme) {

  loadColourSchemes();

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructStaticBox(this), 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  SetSizer(vbox);
}

wxStaticBox* ColourSchemePage::constructStaticBox(wxNotebookPage* parent) {
  auto box = new wxStaticBox(parent, wxID_ANY, wxEmptyString);

  auto vbox = new wxBoxSizer(wxVERTICAL);
  box->SetSizer(vbox);

  wxFont codeFontNormal(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_NORMAL);
  wxFont codeFontBold(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_BOLD);

  auto txtShaderCodePre = new wxStaticText(box, wxID_ANY,
    "vec3 computeColour(int i, float x, float y) {");
  txtShaderCodePre->SetFont(codeFontBold);

  m_cboSelector = new wxComboBox(box, wxID_ANY);
  std::vector<wxString> names;
  for (auto entry : m_colourSchemes) {
    names.push_back(entry.first);
  }
  m_cboSelector->Insert(names, 0);
  m_cboSelector->SetStringSelection(DEFAULT_COLOUR_SCHEME);
  m_cboSelector->Bind(wxEVT_COMBOBOX, &ColourSchemePage::onSelectColourScheme,
                      this);
  m_cboSelector->Bind(wxEVT_TEXT, &ColourSchemePage::onColourSchemeNameChange,
                      this);

  m_txtCode = constructTextBox(box, PRESETS.at(DEFAULT_COLOUR_SCHEME), true,
                               false, true);
  m_txtCode->SetFont(codeFontNormal);

  auto txtShaderCodePost = new wxStaticText(box, wxID_ANY, "}");
  txtShaderCodePost->SetFont(codeFontBold);

  m_txtCompileStatus = constructTextBox(box, wxEmptyString, true, true, true);
  m_txtCompileStatus->SetFont(codeFontNormal);
  m_txtCompileStatus->SetEditable(false);
  m_txtCompileStatus->SetMinSize(wxSize(0, 80));

  m_btnDelete = new wxButton(box, wxID_ANY, wxGetTranslation("Delete"));
  m_btnDelete->Bind(wxEVT_BUTTON, &ColourSchemePage::onDeleteColourSchemeClick,
                    this);
  m_btnDelete->Hide();

  m_btnRestore = new wxButton(box, wxID_ANY, wxGetTranslation("Restore"));
  m_btnRestore->Bind(wxEVT_BUTTON,
                     &ColourSchemePage::onRestoreColourSchemeClick, this);

  m_btnSave = new wxButton(box, wxID_ANY, wxGetTranslation("Save"));
  m_btnSave->Bind(wxEVT_BUTTON, &ColourSchemePage::onSaveColourSchemeClick,
                  this);

  wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
  hbox->AddStretchSpacer(1);
  hbox->Add(m_btnDelete, 0, wxRIGHT, 5);
  hbox->Add(m_btnRestore, 0, wxRIGHT, 5);
  hbox->Add(m_btnSave, 0, wxRIGHT, 0);

  vbox->AddSpacer(10);
  vbox->Add(m_cboSelector, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
  vbox->Add(txtShaderCodePre, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(m_txtCode, 2, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(txtShaderCodePost, 0, wxLEFT | wxRIGHT, 10);
  vbox->Add(hbox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  vbox->Add(m_txtCompileStatus, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);

  return box;
}

void ColourSchemePage::enable() {
  m_btnDelete->Enable();
  m_btnRestore->Enable();
  m_btnSave->Enable();
}

void ColourSchemePage::disable() {
  m_btnDelete->Disable();
  m_btnRestore->Disable();
  m_btnSave->Disable();
}

void ColourSchemePage::applyColourScheme() {
  try {
    string name = m_cboSelector->GetValue().ToStdString();
    string code = m_txtCode->GetValue().ToStdString();
    m_fnApplyColourScheme(code);
    m_txtCompileStatus->SetValue(wxGetTranslation("Success"));
    m_colourSchemes[name] = code;
  }
  catch (const ShaderException& e) {
    m_txtCompileStatus->SetValue(e.errorOutput());
  }
}

void ColourSchemePage::selectColourScheme(int idx) {
  auto& cbo = *m_cboSelector;
  assert(idx >= 0 && idx < static_cast<int>(cbo.GetCount()));

  wxString name = cbo.GetString(idx);
  cbo.SetSelection(idx);

  wxCommandEvent e;
  e.SetString(name);
  onSelectColourScheme(e);
}

void ColourSchemePage::selectColourScheme(const wxString& name) {
  wxCommandEvent e;
  int idx = m_cboSelector->FindString(name);

  if (idx != wxNOT_FOUND) {
    m_cboSelector->SetSelection(idx);
    e.SetString(name);
  }
  else {
    m_cboSelector->SetSelection(0);
    e.SetString(m_cboSelector->GetString(0));
  }

  onColourSchemeNameChange(e);
  onSelectColourScheme(e);
}

void ColourSchemePage::onColourSchemeNameChange(wxCommandEvent&) {
  wxString scheme = m_cboSelector->GetValue();
  bool isPreset = PRESETS.count(scheme.ToStdString()) == 1;

  m_btnDelete->Show(!isPreset);
  m_btnRestore->Show(isPreset);

  m_btnDelete->GetParent()->Layout();
}

void ColourSchemePage::onSelectColourScheme(wxCommandEvent& e) {
  onColourSchemeNameChange(e);

  string scheme = e.GetString().ToStdString();
  m_txtCode->SetValue(m_colourSchemes.at(scheme));
  applyColourScheme();
}

static wxXmlDocument colourSchemesToXmlDoc(const StringMap& schemes) {
  wxXmlDocument doc;
  auto root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "colour_schemes");

  for (auto entry : schemes) {
    const wxString& name = entry.first;
    const wxString& code = entry.second;

    auto xmlColourScheme = new wxXmlNode(root, wxXML_ELEMENT_NODE,
                                         "colour_scheme");

    xmlColourScheme->AddAttribute("name", name);
    xmlColourScheme->AddChild(new wxXmlNode(nullptr, wxXML_TEXT_NODE, "code",
                                            code));
  }

  doc.SetRoot(root);
  return doc;
}

void ColourSchemePage::saveColourSchemes() {
  if (!wxFile::Exists(m_filePath)) {
    wxDir::Make(userDataPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  }

  wxXmlDocument doc = colourSchemesToXmlDoc(m_colourSchemes);
  doc.Save(m_filePath);
}

void ColourSchemePage::onSaveColourSchemeClick(wxCommandEvent&) {
  string name = m_cboSelector->GetValue().ToStdString();
  string code = m_txtCode->GetValue().ToStdString();
  m_colourSchemes[name] = code;

  updateColourSchemeSelector();
  saveColourSchemes();
}

void ColourSchemePage::updateColourSchemeSelector() {
  auto& cbo = *m_cboSelector;
  wxString name = cbo.GetValue();

  std::vector<wxString> names;
  for (auto entry : m_colourSchemes) {
    names.push_back(entry.first);
  }
  cbo.Clear();
  cbo.Insert(names, 0);

  selectColourScheme(name);
}

void ColourSchemePage::onDeleteColourSchemeClick(wxCommandEvent&) {
  auto& cbo = *m_cboSelector;
  string name = cbo.GetValue().ToStdString();

  assert(PRESETS.count(name) == 0);

  m_colourSchemes.erase(name);
  updateColourSchemeSelector();
  saveColourSchemes();
}

void ColourSchemePage::loadColourSchemes() {
  m_filePath = userDataPath("colour_schemes.xml");

  for (auto entry : PRESETS) {
    m_colourSchemes.insert(entry);
  }

  if (wxFile::Exists(m_filePath)) {
    wxXmlDocument doc;
    doc.Load(m_filePath);

    auto xmlColourScheme = doc.GetRoot()->GetChildren();
    while (xmlColourScheme) {
      wxString name = xmlColourScheme->GetAttribute("name");

      auto xmlCode = xmlColourScheme->GetChildren();
      wxString code = xmlCode->GetContent();

      m_colourSchemes[name.ToStdString()] = code.ToStdString();

      xmlColourScheme = xmlColourScheme->GetNext();
    }
  }
}

void ColourSchemePage::onRestoreColourSchemeClick(wxCommandEvent&) {
  string name = m_cboSelector->GetValue().ToStdString();
  m_txtCode->ChangeValue(PRESETS.at(name));

  applyColourScheme();
  saveColourSchemes();
}

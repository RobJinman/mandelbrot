#pragma once

#include <wx/wx.h>
#include <map>
#include <string>

typedef std::map<std::string, std::string> StringMap;
typedef std::function<void(const std::string&)> fnApplyColourScheme_t;

class ColourSchemePanel : public wxStaticBox {
public:
  ColourSchemePanel(wxWindow* parent,
                    fnApplyColourScheme_t fnApplyColourScheme);

  void enable();
  void disable();

private:
  void loadColourSchemes();
  void saveColourSchemes();
  void selectColourScheme(const wxString& name);
  void selectColourScheme(int idx);
  void applyColourScheme();
  void updateColourSchemeSelector();

  void onDeleteColourSchemeClick(wxCommandEvent& e);
  void onRestoreColourSchemeClick(wxCommandEvent& e);
  void onSaveColourSchemeClick(wxCommandEvent& e);
  void onSelectColourScheme(wxCommandEvent& e);
  void onColourSchemeNameChange(wxCommandEvent& e);

  wxString m_filePath;
  wxComboBox* m_cboSelector;
  wxTextCtrl* m_txtCode;
  wxTextCtrl* m_txtCompileStatus;
  wxButton* m_btnDelete;
  wxButton* m_btnRestore;
  wxButton* m_btnSave;
  StringMap m_colourSchemes;
  fnApplyColourScheme_t m_fnApplyColourScheme;
};

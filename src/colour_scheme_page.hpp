#pragma once

#include <map>
#include <string>
#include <wx/wx.h>
#include <wx/notebook.h>

typedef std::map<std::string, std::string> StringMap;
typedef std::function<void(const std::string&)> fnApplyColourScheme_t;

class ColourSchemePage : public wxNotebookPage {
public:
  ColourSchemePage(wxWindow* parent, fnApplyColourScheme_t fnApplyColourScheme);

  void enable();
  void disable();

private:
  void loadColourSchemes();
  void saveColourSchemes();
  void selectColourScheme(const wxString& name);
  void selectColourScheme(int idx);
  void applyColourScheme();
  void updateColourSchemeSelector();
  void showHideButtons();

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

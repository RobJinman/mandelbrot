#include "wx_helpers.hpp"

wxStaticText* constructLabel(wxWindow* parent, const wxString& text) {
  return new wxStaticText(parent, wxID_ANY, text);
}

wxTextCtrl* constructTextBox(wxWindow* parent, const wxString& text,
                             bool multiline, bool readOnly) {

  auto ctrl = new wxTextCtrl(parent, wxID_ANY, text, wxDefaultPosition,
                             wxDefaultSize, multiline ? wxTE_MULTILINE : 0L);

  if (readOnly) {
    ctrl->SetEditable(false);
  }

  return ctrl;
}

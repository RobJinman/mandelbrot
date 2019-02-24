#include <wx/wx.h>

wxStaticText* constructLabel(wxWindow* parent, const wxString& text);

wxTextCtrl* constructTextBox(wxWindow* parent, const wxString& text,
                             bool multiline = false,
                             bool readOnly = false);

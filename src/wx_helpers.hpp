#include <wx/wx.h>

wxStaticText* constructLabel(wxWindow* parent, const wxString& text);

wxTextCtrl* constructTextBox(wxWindow* parent, const wxString& text,
                             bool multiline = false,
                             bool readOnly = false);

std::ostream& operator<<(std::ostream& os, const wxPoint& p);
std::ostream& operator<<(std::ostream& os, const wxSize& sz);

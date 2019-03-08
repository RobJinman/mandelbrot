#include "wx_helpers.hpp"

wxStaticText* constructLabel(wxWindow* parent, const wxString& text) {
  return new wxStaticText(parent, wxID_ANY, text);
}

wxTextCtrl* constructTextBox(wxWindow* parent, const wxString& text,
                             bool multiline, bool readOnly, bool hScroll) {

  long flags = 0;
  if (multiline) {
    flags |= wxTE_MULTILINE;
  }
  if (hScroll) {
    flags |= wxHSCROLL;
  }

  auto ctrl = new wxTextCtrl(parent, wxID_ANY, text, wxDefaultPosition,
                             wxDefaultSize, flags);

  if (readOnly) {
    ctrl->SetEditable(false);
  }

  return ctrl;
}

template<>
long getValue(const wxTextCtrl& textBox) {
  long value = 0;
  textBox.GetValue().ToLong(&value);
  return value;
}

template<>
double getValue(const wxTextCtrl& textBox) {
  double value = 0;
  textBox.GetValue().ToDouble(&value);
  return value;
}

std::ostream& operator<<(std::ostream& os, const wxPoint& p) {
  os << "(" << p.x << ", " << p.y << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const wxSize& sz) {
  os << "(" << sz.x << ", " << sz.y << ")";
  return os;
}

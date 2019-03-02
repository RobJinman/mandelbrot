#include <sstream>
#include "wx_helpers.hpp"
#include "config.hpp"

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

std::string versionString() {
  std::stringstream ss;
  ss << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
     << Mandelbrot_VERSION_MINOR;
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const wxPoint& p) {
  os << "(" << p.x << ", " << p.y << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const wxSize& sz) {
  os << "(" << sz.x << ", " << sz.y << ")";
  return os;
}

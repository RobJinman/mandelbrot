#pragma once

#include <wx/wx.h>

wxStaticText* constructLabel(wxWindow* parent, const wxString& text);

wxTextCtrl* constructTextBox(wxWindow* parent, const wxString& text,
                             bool multiline = false, bool readOnly = false,
                             bool hScroll = false);

template<typename T>
T getValue(const wxTextCtrl& textBox) = delete;

template<>
long getValue(const wxTextCtrl& textBox);

template<>
double getValue(const wxTextCtrl& textBox);

template<typename T>
T getMinValue(wxTextCtrl& textBox, T min) {
  T value = getValue<T>(textBox);

  if (value < min) {
    value = min;
    textBox.ChangeValue(std::to_string(value));
  }

  return value;
}

template<typename T>
T getMaxValue(wxTextCtrl& textBox, T max) {
  T value = getValue<T>(textBox);

  if (value > max) {
    value = max;
    textBox.ChangeValue(std::to_string(value));
  }

  return value;
}

template<typename T>
T getBoundedValue(wxTextCtrl& textBox, T min, T max) {
  assert(min < max);

  T value = getMinValue<T>(textBox, min);
  value = getMaxValue<T>(textBox, max);

  return value;
}

std::ostream& operator<<(std::ostream& os, const wxPoint& p);
std::ostream& operator<<(std::ostream& os, const wxSize& sz);

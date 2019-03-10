#pragma once

#include <string>
#include <wx/wx.h>
#include <wx/notebook.h>

class Renderer;

class InfoPage : public wxNotebookPage {
public:
  InfoPage(wxWindow* parent);

  void onRender(const Renderer& renderer);

private:
  wxStaticBox* constructInfoPanel(wxWindow* parent);
  wxStaticBox* constructDataPanel(wxWindow* parent);

  wxStaticText* m_txtMagLevel;
  wxStaticText* m_txtXMin;
  wxStaticText* m_txtXMax;
  wxStaticText* m_txtYMin;
  wxStaticText* m_txtYMax;
};

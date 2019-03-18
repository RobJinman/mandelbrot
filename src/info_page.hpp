#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>

class Renderer;

class InfoPage : public wxNotebookPage {
public:
  InfoPage(wxWindow* parent);

  void onRender(const Renderer& renderer);

private:
  wxWindow* constructInfoPanel(wxWindow* parent);
  wxStaticBoxSizer* constructDataPanel(wxWindow* parent);

  wxStaticText* m_txtMagLevel;
  wxStaticText* m_txtXMin;
  wxStaticText* m_txtXMax;
  wxStaticText* m_txtYMin;
  wxStaticText* m_txtYMax;
};

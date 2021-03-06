#include <wx/richtext/richtextctrl.h>
#include "info_page.hpp"
#include "utils.hpp"
#include "wx_helpers.hpp"
#include "renderer.hpp"

InfoPage::InfoPage(wxWindow* parent)
  : wxNotebookPage(parent, wxID_ANY) {

  auto vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(constructInfoPanel(this), 1, wxEXPAND | wxALL, 10);
  vbox->Add(constructDataPanel(this), 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,
            10);

  SetSizer(vbox);
}

wxWindow* InfoPage::constructInfoPanel(wxWindow* parent) {
  wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_NORMAL);

  auto txtInfo = new wxRichTextCtrl(parent, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxVSCROLL | wxBORDER_NONE | wxWANTS_CHARS);

  txtInfo->SetFont(font);

  auto addHeading = [txtInfo](const wxString& text) {
    txtInfo->BeginBold();
    txtInfo->BeginUnderline();
    txtInfo->WriteText(wxGetTranslation(text));
    txtInfo->EndUnderline();
    txtInfo->EndBold();
    txtInfo->Newline();
  };

  auto addText = [txtInfo](const wxString& text, bool translate = true,
                           bool newline = true, bool bold = false) {
    if (bold) {
      txtInfo->BeginBold();
    }
    txtInfo->WriteText(translate ? wxGetTranslation(text) : text);
    if (newline) {
      txtInfo->Newline();
    }
    if (bold) {
      txtInfo->EndBold();
    }
  };

  txtInfo->SetEditable(false);
  txtInfo->SetMinSize(wxSize(0, 80));

  addHeading(versionString());
  addText("The Mandelbrot fractal rendered on the GPU.");
  txtInfo->Newline();
  addText("Click and drag the canvas to zoom.");
  txtInfo->Newline();
  addHeading(wxGetTranslation("Controls"));
  addText("R   ", false, false, true);
  addText("Reset view");
  addText("Z   ", false, false, true);
  addText("Toggle Fly-Through mode");
  addText("I   ", false, false, true);
  addText("Zoom in");
  addText("O   ", false, false, true);
  addText("Zoom out");

  return txtInfo;
}

wxStaticBoxSizer* InfoPage::constructDataPanel(wxWindow* parent) {
  auto boxSizer = new wxStaticBoxSizer(wxVERTICAL, parent,
                                       wxGetTranslation("Data"));
  auto box = boxSizer->GetStaticBox();

  auto grid = new wxFlexGridSizer(2);
  boxSizer->Add(grid);

  auto lblMagLevel = constructLabel(box, wxGetTranslation("Magnification"));
  m_txtMagLevel = constructLabel(box, "");

  auto lblXMin = constructLabel(box, "x-min");
  m_txtXMin = constructLabel(box, "");

  auto lblXMax = constructLabel(box, "x-max");
  m_txtXMax = constructLabel(box, "");

  auto lblYMin = constructLabel(box, "y-min");
  m_txtYMin = constructLabel(box, "");

  auto lblYMax = constructLabel(box, "y-max");
  m_txtYMax = constructLabel(box, "");

  grid->AddSpacer(10);
  grid->AddSpacer(10);
  grid->Add(lblMagLevel, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtMagLevel, 0, wxEXPAND | wxRIGHT, 10);
  grid->Add(lblXMin, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtXMin, 1, wxEXPAND, 10);
  grid->Add(lblXMax, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtXMax, 1, wxEXPAND, 10);
  grid->Add(lblYMin, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtYMin, 1, wxEXPAND, 10);
  grid->Add(lblYMax, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
  grid->Add(m_txtYMax, 0, wxEXPAND | wxRIGHT, 10);

  grid->AddGrowableCol(1);

  return boxSizer;
}

void InfoPage::onRender(const Renderer& renderer) {
  auto magLevel = numberToString(renderer.computeMagnification(), true);
  m_txtMagLevel->SetLabel(magLevel);
  m_txtXMin->SetLabel(numberToString(renderer.getXMin(), true));
  m_txtXMax->SetLabel(numberToString(renderer.getXMax(), true));
  m_txtYMin->SetLabel(numberToString(renderer.getYMin(), true));
  m_txtYMax->SetLabel(numberToString(renderer.getYMax(), true));
}

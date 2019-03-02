#include <stdexcept>
#include <wx/wx.h>

class Application : public wxApp {
public:
  virtual bool OnInit() override;
  virtual void HandleEvent(wxEvtHandler* handler, wxEventFunction func,
                           wxEvent& event) const;
};

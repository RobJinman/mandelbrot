#include "application.hpp"
#include "main_window.hpp"
#include "utils.hpp"
#include "exception.hpp"

static void doWithLogging(std::function<void()> fn) {
  try {
    fn();
  }
  catch (const ShaderException& e) {
    std::cerr << "A fatal exception occurred: " << std::endl;
    std::cerr << e.what() << ": " << e.errorOutput() << std::endl;
    exit(1);
  }
  catch (const std::runtime_error& e) {
    std::cerr << "A fatal exception occurred: " << std::endl;
    std::cerr << e.what() << std::endl;
    exit(1);
  }
}

bool Application::OnInit() {
  MainWindow* frame = new MainWindow(versionString(),
                                     wxSize(WINDOW_W, WINDOW_H));
  frame->Show();

  return true;
}

void Application::HandleEvent(wxEvtHandler* handler, wxEventFunction func,
                          wxEvent& event) const {
  doWithLogging([this, handler, &func, &event]() {
    wxApp::HandleEvent(handler, func, event);
  });
}

wxIMPLEMENT_APP(Application);

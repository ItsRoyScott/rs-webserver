#include "WebServer.hpp"
using namespace OlympusWebServer;

int main()
{
  auto webServer = WebServer(8000);

  while (webServer.IsRunning())
  {
    webServer.Update();
  }
}
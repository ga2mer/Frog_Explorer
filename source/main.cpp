#include <stdio.h>
#include <string.h>
#include <switch.h>
#include <thread>

#include <algorithm>
#include <borealis.hpp>
#include "utils/Emulator.hpp"

#include "pages/MainPage.hpp"

int main(int argc, char **argv) {
  if (!brls::Application::init("Frog")) {
    brls::Logger::error("Unable to init Frog");
    return EXIT_FAILURE;
  }
  brls::Application::pushView(new MainPage());
  while (brls::Application::mainLoop())
    ;

  return EXIT_SUCCESS;
}

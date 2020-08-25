#pragma once

#include <borealis.hpp>
#include "MainPage.hpp"

class ExplorerTab : public brls::List {
  MainPage* root;
  std::string currentPath = "/";
  private:
    void setFolder(std::string path = "/", std::string previousPath = "");
    void openProperties(std::string path = "/", bool fromFolder = false);
    void updateFileHint();
  public:
    ExplorerTab(MainPage* root);
};

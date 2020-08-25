#include <switch.h>
#include "MainPage.hpp"

#include <borealis.hpp>

#include "ExplorerTab.hpp"

std::list<std::string> buffer = {};
MainPage::MainPage() : TabFrame() {
  this->setTitle("Frog Explorer");
  this->setIcon("romfs:/icon/borealis.jpg");
  // SetLanguage ourLang;
  // u64 lcode = 0;
  // setInitialize();
  // setGetSystemLanguage(&lcode);
  // setMakeLanguage(lcode, &ourLang);
  // setExit();
  // // u64 LanguageCode=0;
  // // SetLanguage Language=SetLanguage_ENUS;
  // // setGetSystemLanguage(&LanguageCode);
  // // setMakeLanguageCode(SetLanguage_JA, &LanguageCode);
  // this->setFooterText(std::to_string(ourLang));

  // brls::List *testList = new brls::List();
  // brls::List *newTab = new brls::List();

  this->addTab("Frogsplorer", new ExplorerTab(this));
  // this->addTab("New tab", newTab);
  // this->addSeparator();
  // this->addTab("About", newTab);
}
std::list<std::string> MainPage::getBuffer() {
  return this->buffer;
}
void MainPage::pushToBuffer(std::string something) {
  buffer.push_back(something);
  this->setFooterText("Buffer files count: " + std::to_string(buffer.size()));
}

MainPage::BufferType MainPage::getBufferType() {
  return this->bufferType;
}

void MainPage::setBufferType(BufferType bufferType) {
  this->bufferType = bufferType;
}

void MainPage::clearBuffer() {
  this->bufferType = BufferType::NONE;
  this->buffer.clear();
  this->setFooterText("");
}
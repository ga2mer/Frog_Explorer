#pragma once

#include <borealis.hpp>


class MainPage : public brls::TabFrame {
  std::list<std::string> buffer;
  public:
    enum BufferType {
      NONE,
      COPY,
      MOVE
    };
    MainPage();
    std::list<std::string> getBuffer();
    void clearBuffer();
    void pushToBuffer(std::string something);
    BufferType getBufferType();
    void setBufferType(BufferType);
  private:
    BufferType bufferType = BufferType::NONE;
};

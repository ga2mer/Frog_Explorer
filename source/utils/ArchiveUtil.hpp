#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

class ArchiveUtil {
  public:
    static void unpack(std::string archivePath, std::string toPath, std::function<void(uint64_t)> cb);
  private:
};

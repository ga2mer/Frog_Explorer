#include "ExplorerTab.hpp"

#include <sys/stat.h>

#include <borealis/swkbd.hpp>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <list>
#include <set>
#include <thread>

#include "../components/StatusCell.hpp"
#include "../utils/ArchiveUtil.hpp"
#include "../utils/Emulator.hpp"
#include "MainPage.hpp"
namespace fs = std::filesystem;
std::string currentPath = "";

std::list<std::string> selected = {};

void copyFile(std::string srcPath, std::string destPath) {
  std::ifstream src(srcPath, std::ios::binary);
  std::ofstream dest(destPath, std::ios::binary);

  dest << src.rdbuf();

  src.close();
  dest.flush();
  dest.close();
}

void copyDir(std::string srcPath, std::string destPath) {
  fs::create_directory(destPath);
  for (const auto &entry : fs::directory_iterator(srcPath)) {
    std::string newSrcPath = fs::path(srcPath) / entry.path().filename();
    std::string newDestPath = fs::path(destPath) / entry.path().filename();
    if (entry.is_directory()) {
      copyDir(newSrcPath, newDestPath);
    } else if (entry.is_regular_file()) {
      copyFile(newSrcPath, newDestPath);
    }
  }
}

#define DIM(x) (sizeof(x) / sizeof(*(x)))

static const char *sizes[] = {"EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B"};
static const uint64_t exbibytes =
    1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;

char *calculateSize(uint64_t size) {
  char *result = (char *)malloc(sizeof(char) * 20);
  uint64_t multiplier = exbibytes;
  int i;

  for (i = 0; i < DIM(sizes); i++, multiplier /= 1024) {
    if (size < multiplier) continue;
    if (size % multiplier == 0)
      sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes[i]);
    else
      sprintf(result, "%.1f %s", (float)size / multiplier, sizes[i]);
    return result;
  }
  strcpy(result, "0");
  return result;
}

void getFileSize(std::string path, long &f_size, long &f_count, long &d_count) {
  if (fs::exists(path)) {
    if (fs::is_directory(path)) {
      for (const auto &entry : fs::directory_iterator(path)) {
        try {
          if (entry.is_regular_file()) {
            f_size = f_size + entry.file_size();
            f_count = f_count + 1;
          }
          if (entry.is_directory()) {
            getFileSize(entry.path(), f_size, f_count, d_count);
            d_count = d_count + 1;
          }
        } catch (std::exception &e) {
          // std::cout << e.what() << endl;
        }
      }
    } else if (fs::is_regular_file(path)) {
      f_size = f_size + fs::file_size(path);
    }
  }
}

std::string getFileType(std::string path) {
  if (!fs::exists(path)) return "Deleted";
  if (fs::is_directory(path))
    return "Folder";
  else if (fs::is_regular_file(path))
    return "File";
  return "Other";
}

template <typename... Args>
std::string string_format(const std::string &format, Args... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) +
                1;  // Extra space for '\0'
  if (size <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1);  // We don't want the '\0' inside
}

std::string timeStampToHReadble(const time_t rawtime) {
  struct tm *dt;
  char buffer[30];
  dt = localtime(&rawtime);
  sprintf(buffer, "%s", asctime(dt));
  return std::string(buffer);
}

void ExplorerTab::openProperties(std::string path, bool fromFolder) {
  fs::path fsPath(path);
  std::string type = getFileType(path);
  brls::AppletFrame *propertiesFrame = new brls::AppletFrame(0, 0);
  brls::List *pList = new brls::List();
  pList->setMargins(50, 400, 0, 400);
  StatusCell *locationCell = new StatusCell(
      "Location", fromFolder
                      ? fs::path(this->currentPath).parent_path().string()
                      : this->currentPath);
  long f_size = 0;
  long f_count = 0;
  long d_count = 0;
  if (selected.size() > 1) {
    for (auto const &i : selected) {
      getFileSize(i, f_size, f_count, d_count);
    }
    char *s = calculateSize(f_size);

    std::string sizeString = string_format("%s (%d) %d files, %d folders", s,
                                           f_size, f_count, d_count);
    StatusCell *sizeCell = new StatusCell("Size", sizeString);
    pList->addView(locationCell);
    pList->addView(sizeCell);
  } else {
    if (selected.size() == 1) {
      path = selected.front();
      fs::path fsPath(path);
    }
    getFileSize(path, f_size, f_count, d_count);
    char *s = calculateSize(f_size);

    std::string sizeString = string_format("%s (%d)", s, f_size);

    if (type == "Folder") {
      sizeString += string_format(" %d files, %d folders", f_count, d_count);
    }

    struct stat buff;
    stat(path.c_str(), &buff);
    StatusCell *typeCell = new StatusCell("Type", type);
    StatusCell *sizeCell = new StatusCell("Size", sizeString);
    StatusCell *ctCell =
        new StatusCell("Create time", timeStampToHReadble(buff.st_ctime));
    StatusCell *mtCell =
        new StatusCell("Modified time", timeStampToHReadble(buff.st_mtime));
    StatusCell *atCell =
        new StatusCell("Access time", timeStampToHReadble(buff.st_atime));
    pList->addView(typeCell);
    pList->addView(locationCell);
    pList->addView(sizeCell);
    pList->addView(ctCell);
    pList->addView(mtCell);
    pList->addView(atCell);
  }
  pList->addView(new brls::ListItem(""));
  std::string titlePath = path == "/" ? "Root path" : fs::path(path).filename();
  propertiesFrame->setTitle(
      selected.size() > 1 ? string_format("%d files selected", selected.size())
                          : titlePath);
  propertiesFrame->setContentView(pList);
  brls::Application::pushView(propertiesFrame);
}

MainPage *root;
ExplorerTab::ExplorerTab(MainPage *root) {
  this->root = root;
  this->registerAction("Params", brls::Key::L, [this] {
    brls::AppletFrame *frame = new brls::AppletFrame(0, 0);
    brls::List *paramsList = new brls::List();
    if (this->root->getBuffer().size() > 0) {
      brls::ListItem *pasteItem = new brls::ListItem("Paste");
      pasteItem->getClickEvent()->subscribe([=](brls::View *view) {
        MainPage::BufferType bt = this->root->getBufferType();
        brls::ProgressSpinner *prog = new brls::ProgressSpinner();
        brls::Dialog *dialog = new brls::Dialog(prog);
        dialog->setCancelable(false);
        auto bufferThread = [=]() {
          for (auto const &i : this->root->getBuffer()) {
            if (bt == MainPage::BufferType::COPY) {
              try {
                if (fs::is_directory(i)) {
                  copyDir(i, fs::path(currentPath) / fs::path(i).filename());
                } else {
                  copyFile(i, fs::path(currentPath) / fs::path(i).filename());
                }
              } catch (const std::exception &e) {
                brls::Dialog *dialog = new brls::Dialog(e.what());
                brls::GenericEvent::Callback closeCallback =
                    [dialog](brls::View *view) { dialog->close(); };
                dialog->addButton("Continue", closeCallback);
                dialog->open();
              }
            } else if (bt == MainPage::BufferType::MOVE) {
              try {
                fs::rename(i, fs::path(currentPath) / fs::path(i).filename());
              } catch (const std::exception &e) {
                brls::Dialog *dialog = new brls::Dialog(e.what());
                brls::GenericEvent::Callback closeCallback =
                    [dialog](brls::View *view) { dialog->close(); };
                dialog->addButton("Continue", closeCallback);
                dialog->open();
              }
            }
          }
          dialog->close([=]() {
            this->setFolder(this->currentPath);
            brls::Application::unblockInputs();
            if (bt == MainPage::BufferType::MOVE) {
              this->root->setBufferType(MainPage::BufferType::NONE);
              this->root->clearBuffer();
            }
          });
        };
        try {
          brls::Application::popView(brls::ViewAnimation::FADE, [=]() {
            dialog->open();
            new std::thread(bufferThread);
          });
        } catch (const std::exception &e) {
          printToEmu(string(e.what()));
        }
      });
      paramsList->addView(pasteItem);
    }
    brls::ListItem *propertiesItem = new brls::ListItem("Properties");
    propertiesItem->getClickEvent()->subscribe([=](brls::View *view) {
      openProperties(this->currentPath, true);
      // frame->onCancel();
    });
    paramsList->addView(propertiesItem);
    frame->setTitle("Folder params");
    frame->setContentView(paramsList);
    brls::Application::pushView(frame);
    return true;
  });
  // this->registerAction("Remove select", brls::Key::X, [this] {
  //   for (size_t i = 0; i < this->getViewsCount(); i++) {
  //     brls::ListItem *item = dynamic_cast<brls::ListItem
  //     *>(this->getChild(i)); item->setChecked(false);
  //   }
  //   selected.clear();
  //   return true;
  // });
  this->registerAction("Back", brls::Key::B, [this] {
    setFolder(fs::path(this->currentPath).parent_path());
    return true;
  });
  setFolder();
}

void ExplorerTab::updateFileHint() {
  for (size_t i = 0; i < this->getViewsCount(); i++) {
    // brls::ListItem *item = dynamic_cast<brls::ListItem *>(this->getChild(i));
    auto item = this->getChild(i);
    // .. item affects
    if (selected.size() > 0) {
      item->updateActionHint(brls::Key::X, "Selected file params");
    } else {
      item->updateActionHint(brls::Key::X, "File params");
    }
  }
}
// i don't like this
std::list<fs::directory_entry> sortFolderFirst(std::string path) {
  std::list<fs::directory_entry> folders = {};
  std::list<fs::directory_entry> files = {};
  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_directory()) {
      folders.push_back(entry);
    }
    if (entry.is_regular_file() || entry.is_symlink()) {
      files.push_back(entry);
    }
  }
  folders.sort();
  files.sort();
  folders.splice(folders.end(), files);
  return folders;
}

std::set<std::string> archiveExtensions = {".rar", ".7z", ".tar", ".zip"};

void ExplorerTab::setFolder(std::string path, std::string previousPath) {
  if (!fs::is_directory(path)) {
    brls::Dialog *dialog = new brls::Dialog("It's not a directory");
    brls::GenericEvent::Callback closeCallback = [dialog](brls::View *view) {
      dialog->close();
    };
    dialog->addButton("Continue", closeCallback);
    dialog->open();
    return;
  }
  currentPath = path;
  this->clear(false);
  selected.clear();
  this->updateFileHint();
  this->addView(new brls::Header(path == "/" ? "Root path" : path, false));
  // bool isBack = false;
  if (path != "/") {
    // isBack = true;
    brls::ListItem *backItem = new brls::ListItem("..");
    backItem->setThumbnail("romfs:/folder.png");
    backItem->getClickEvent()->subscribe([=](brls::View *view) {
      brls::ListItem *item = dynamic_cast<brls::ListItem *>(view);
      if (!item) return;
      setFolder(fs::path(path).parent_path(), path);
    });
    this->addView(backItem);
  }
  std::list<fs::directory_entry> folderFirst = sortFolderFirst(path);
  for (const auto &entry : folderFirst) {
    brls::ListItem *dialogItem = new brls::ListItem(entry.path().filename());
    dialogItem->getClickEvent()->subscribe([=](brls::View *view) {
      brls::ListItem *item = dynamic_cast<brls::ListItem *>(view);
      if (!item) return;
      if (entry.is_directory()) {
        setFolder(entry.path());
      }
    });
    dialogItem->registerAction("File params", brls::Key::X, [this, entry] {
      brls::AppletFrame *frame = new brls::AppletFrame(0, 0);
      brls::List *paramsList = new brls::List();
      brls::ListItem *copyItem = new brls::ListItem("Copy");
      brls::ListItem *moveItem = new brls::ListItem("Move");
      moveItem->getClickEvent()->subscribe([=](brls::View *view) {
        this->root->clearBuffer();
        if (selected.size() > 0) {
          for (auto const &i : selected) {
            this->root->pushToBuffer(i);
          }
        } else {
          this->root->pushToBuffer(entry.path());
        }
        this->root->setBufferType(MainPage::BufferType::MOVE);
        frame->onCancel();
      });
      brls::ListItem *removeItem = new brls::ListItem("Remove");
      copyItem->getClickEvent()->subscribe([=](brls::View *view) {
        this->root->clearBuffer();
        if (selected.size() > 0) {
          for (auto const &i : selected) {
            this->root->pushToBuffer(i);
          }
        } else {
          this->root->pushToBuffer(entry.path());
        }
        this->root->setBufferType(MainPage::BufferType::COPY);
        frame->onCancel();
      });
      removeItem->getClickEvent()->subscribe([=](brls::View *view) {
        frame->onCancel();
        if (selected.size() > 0) {
          for (auto const &i : selected) {
            if (fs::is_directory(i)) {
              rmdir(i.c_str());
            }
            if (fs::is_regular_file(i) || fs::is_symlink(i)) {
              fs::remove(i);
            }
          }
        } else {
          if (entry.is_directory()) {
            rmdir(entry.path().c_str());
          }
          if (entry.is_regular_file() || entry.is_symlink()) {
            fs::remove(entry.path());
          }
        }
        this->setFolder(this->currentPath);
      });
      paramsList->addView(copyItem);
      paramsList->addView(moveItem);
      if (selected.size() == 0) {
        brls::ListItem *renameItem = new brls::ListItem("Rename");
        renameItem->getClickEvent()->subscribe([=](brls::View *view) {
          brls::Swkbd::openForText(
              [entry, this, frame](std::string text) {
                try {
                  if (entry.path() != entry.path().parent_path() / text) {
                    fs::rename(entry.path(), entry.path().parent_path() / text);
                  }
                  frame->onCancel();
                  this->setFolder(this->currentPath);
                } catch (const std::exception &e) {
                  brls::Dialog *dialog = new brls::Dialog(e.what());
                  brls::GenericEvent::Callback closeCallback =
                      [dialog](brls::View *view) { dialog->close(); };
                  dialog->addButton("Continue", closeCallback);
                  dialog->open();
                }
              },
              "Enter new name", "", 255, entry.path().filename());
        });
        paramsList->addView(renameItem);
      }
      paramsList->addView(removeItem);
      brls::ListItem *propertiesItem = new brls::ListItem("Properties");
      propertiesItem->getClickEvent()->subscribe(
          [=](brls::View *view) { openProperties(entry.path(), false); });
      paramsList->addView(propertiesItem);
      if (archiveExtensions.count(entry.path().filename().extension())) {
        brls::ProgressDisplay *prog = new brls::ProgressDisplay();
        brls::Dialog *dialog = new brls::Dialog(prog);
        dialog->setCancelable(false);
        auto progressCallback = [=](uint64_t readed) {
          prog->setProgress(100 * readed / entry.file_size(), 100);
        };
        auto extractThread = [=](std::string from, std::string to) {
          ArchiveUtil::unpack(from, to, progressCallback);
          dialog->close([=]() {
            this->setFolder(this->currentPath);
            brls::Application::unblockInputs();
          });
        };
        brls::ListItem *extractHereItem = new brls::ListItem("Extract here");
        extractHereItem->getClickEvent()->subscribe([=](brls::View *view) {
          try {
            brls::Application::popView(brls::ViewAnimation::FADE, [=]() {
              dialog->open();
              new std::thread(extractThread, entry.path(), this->currentPath);
            });
          } catch (const std::exception &e) {
            printToEmu(string(e.what()));
          }
        });
        brls::ListItem *extractToItem = new brls::ListItem(string_format(
            "Extract to \"%s\"", entry.path().filename().stem().c_str()));
        extractToItem->getClickEvent()->subscribe([=](brls::View *view) {
          try {
            brls::Application::popView(brls::ViewAnimation::FADE, [=]() {
              dialog->open();
              new std::thread(
                  extractThread, entry.path(),
                  fs::path(this->currentPath) / entry.path().filename().stem());
            });
          } catch (const std::exception &e) {
            printToEmu(string(e.what()));
          }
        });
        paramsList->addView(extractHereItem);
        paramsList->addView(extractToItem);
      }
      frame->setTitle("File params");
      frame->setContentView(paramsList);
      brls::Application::pushView(frame);
      return true;
    });
    dialogItem->registerAction(
        "Select", brls::Key::Y, [dialogItem, entry, this] {
          if ((std::find(selected.begin(), selected.end(), entry.path()) !=
               selected.end())) {
            dialogItem->setChecked(false);
            selected.remove(entry.path());
          } else {
            dialogItem->setChecked(true);
            selected.push_back(entry.path());
          }
          this->updateFileHint();
          return true;
        });
    if (entry.is_directory()) {
      dialogItem->setThumbnail("romfs:/folder.png");
    }
    this->addView(dialogItem);
    if (!previousPath.empty() &&
        fs::path(previousPath).filename() == entry.path().filename()) {
      // brls::Application::giveFocus(dialogItem);
    }
  }
  brls::Application::giveFocus(this->getChild(1));
  willAppear(true);
  // if (isBack && previousPath.empty()) {
  //   brls::Application::giveFocus(this->getChild(1));
  //   willAppear(true);
  // } else {
  //   brls::Application::giveFocus(this->getDefaultFocus());
  // willAppear(true);
  // }
}
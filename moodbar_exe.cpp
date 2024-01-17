/*
  Moodbar audio timeline visualization
  Copyright (C) 2015, 2017  Johannes Sasongko <sasongko@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "moodbar-config.h"
#include "moodbar.h"

#include <gio/gio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <future>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#ifdef _WIN32
#include <vector>
#endif

using namespace std::string_literals;

// RAII helper types
using GCharH = std::unique_ptr<char, decltype(&g_free)>;
using GFileH = std::unique_ptr<GFile, decltype(&g_object_unref)>;
using GFileOutputStreamH =
    std::unique_ptr<GFileOutputStream, decltype(&g_object_unref)>;

namespace {
void printUsage(std::ostream &stream, const char *exe) {
  stream << "Usage: " << exe << " -o OUTPUT INPUT" << std::endl;
}
} // namespace

int main(int argc, char *argv[]) {
  bool isCorrectUsage = false;
  if (argc == 2) {
    std::string argv1 = argv[1];
    if (argv1 == "--version") {
      std::cout << "moodbar " MOODBAR_VERSION_STR << std::endl;
      return 0;
    } else if (argv1 == "--help") {
      printUsage(std::cout, argv[0]);
      return 0;
    }
  } else if (argc == 4 and argv[1] == "-o"s) {
    isCorrectUsage = true;
  }
  if (not isCorrectUsage) {
    printUsage(std::cerr, (argc > 0 ? argv[0] : "moodbar"));
    return 1;
  }

  const char *argOut = argv[2];
  const char *argIn = argv[3];

  // Process input file

  GFileH fileIn{g_file_new_for_commandline_arg(argIn), &g_object_unref};
  GCharH uriIn{g_file_get_uri(fileIn.get()), &g_free};

  std::promise<bool> moodAvailablePromise; // The bool indicates success.
  auto moodAvailable = moodAvailablePromise.get_future();

  moodbar_init();
  MoodbarPipeline mood{uriIn.get()};
  mood.Finished = [&moodAvailablePromise](bool success) {
    moodAvailablePromise.set_value(success);
  };
  mood.Start();

  moodAvailable.wait();
  if (not moodAvailable.get())
    return 1; // There should already be an error message.

  // Write output file

  GFileH fileOut{g_file_new_for_commandline_arg(argOut), &g_object_unref};
  GFileOutputStreamH streamOut{g_file_replace(fileOut.get(), nullptr, FALSE,
                                              G_FILE_CREATE_REPLACE_DESTINATION,
                                              nullptr, nullptr),
                               &g_object_unref};
  if (not streamOut) {
    std::cerr << "ERROR: Failed writing " << argOut << std::endl;
    return 1;
  }
  const auto &data = mood.data();
  bool ok =
      g_output_stream_write_all(G_OUTPUT_STREAM(streamOut.get()), data.data(),
                                data.size(), nullptr, nullptr, nullptr);
  if (not ok) {
    std::cerr << "ERROR: Failed writing " << argOut << std::endl;
    return 1;
  }

  return 0;
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[]) {
  // On Windows, GIO expects paths in UTF-8, so let's just convert all
  // arguments to UTF-8
  auto args_utf8 = std::vector<std::vector<char>>();
  args_utf8.reserve(argc);
  for (int argi = 0; argi < argc; ++argi) {
    auto arg = argv[argi];
    auto size_utf8 =
        WideCharToMultiByte(CP_UTF8, 0, arg, -1, nullptr, 0, nullptr, nullptr) +
        1;
    auto arg_utf8 = std::vector<char>(size_utf8);
    WideCharToMultiByte(CP_UTF8, 0, arg, -1, &arg_utf8[0], size_utf8, nullptr,
                        nullptr);
    args_utf8.push_back(arg_utf8);
  }
  auto args_utf8_cstr = std::vector<char *>(argc);
  std::transform(args_utf8.cbegin(), args_utf8.cend(),
                 std::begin(args_utf8_cstr),
                 [](auto &s) { return const_cast<char *>(&s[0]); });
  return main(argc, &args_utf8_cstr[0]);
}
#endif

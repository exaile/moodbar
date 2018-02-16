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

#include <future>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

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
}  // namespace

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

  std::promise<bool> moodAvailablePromise;  // The bool indicates success.
  auto moodAvailable = moodAvailablePromise.get_future();

  moodbar_init();
  MoodbarPipeline mood{uriIn.get()};
  mood.Finished = [&moodAvailablePromise](bool success) {
    moodAvailablePromise.set_value(success);
  };
  mood.Start();

  moodAvailable.wait();
  if (not moodAvailable.get())
    return 1;  // There should already be an error message.

  // Write output file

  GFileH fileOut{g_file_new_for_commandline_arg(argOut), &g_object_unref};
  GFileOutputStreamH streamOut{
      g_file_replace(fileOut.get(), nullptr, FALSE,
                     G_FILE_CREATE_REPLACE_DESTINATION, nullptr, nullptr),
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

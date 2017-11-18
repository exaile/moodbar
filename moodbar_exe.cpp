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

#include "moodbar.h"

#include <gio/gio.h>

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

using namespace std::string_literals;

// RAII helper types
using GCharH = std::unique_ptr<char, decltype(&g_free)>;
using GFileH = std::unique_ptr<GFile, decltype(&g_object_unref)>;
using GFileOutputStreamH =
    std::unique_ptr<GFileOutputStream, decltype(&g_object_unref)>;

int main(int argc, char* argv[]) {
  if (argc != 4 or argv[1] != "-o"s) {
    std::cerr << "Syntax: " << (argc > 0 ? argv[0] : "moodbar")
              << " -o OUTPUT INPUT" << std::endl;
    return 1;
  }

  const char* argOut = argv[2];
  const char* argIn = argv[3];

  // Process input file

  GFileH fileIn{g_file_new_for_commandline_arg(argIn), &g_object_unref};
  GCharH uriIn{g_file_get_uri(fileIn.get()), &g_free};

  std::mutex mutex;
  std::condition_variable cond;
  int status = -1;  // 0 = success, >0 = error

  moodbar_init();
  MoodbarPipeline mood{uriIn.get()};
  mood.Finished = [&mutex, &status, &cond](bool success) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      status = success ? 0 : 1;
    }
    cond.notify_one();
  };
  mood.Start();

  {  // Wait until the pipeline finishes and we get a status
    std::unique_lock<std::mutex> lock(mutex);
    while (status == -1) cond.wait(lock);
  }
  if (status != 0) return status;  // There should already be an error message.

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
  const auto& data = mood.data();
  bool ok =
      g_output_stream_write_all(G_OUTPUT_STREAM(streamOut.get()), data.data(),
                                data.size(), nullptr, nullptr, nullptr);
  if (not ok) {
    std::cerr << "ERROR: Failed writing " << argOut << std::endl;
    return 1;
  }

  return 0;
}

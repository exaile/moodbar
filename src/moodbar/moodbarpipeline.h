/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOODBARPIPELINE_H
#define MOODBARPIPELINE_H

#include <gst/gst.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class MoodbarBuilder;

// Creates moodbar data for a single local music file.
class MoodbarPipeline {
 public:
  MoodbarPipeline(const std::string& local_filename);
  ~MoodbarPipeline();

  static bool IsAvailable();

  bool success() const { return success_; }
  const std::vector<std::uint8_t>& data() const { return data_; }

  void Start();

  std::function<void(bool success)> Finished;

 private:
  GstElement* CreateElement(const std::string& factory_name);

  void ReportError(GstMessage* message);
  void Stop(bool success);
  void Cleanup();

  static void NewPadCallback(GstElement*, GstPad* pad, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg,
                                         gpointer data);

 private:
  static bool sIsAvailable;
  static const int kBands;

  std::string local_filename_;
  GstElement* pipeline_;
  GstElement* convert_element_;

  std::unique_ptr<MoodbarBuilder> builder_;

  bool success_;
  bool running_;
  std::vector<std::uint8_t> data_;
};

#endif  // MOODBARPIPELINE_H

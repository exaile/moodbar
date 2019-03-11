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

#include "moodbarpipeline.h"

#include "moodbar/moodbarbuilder.h"

#include "gst/moodbar/gstfastspectrum.h"

#include <iostream>

bool MoodbarPipeline::sIsAvailable = false;
const int MoodbarPipeline::kBands = 128;

MoodbarPipeline::MoodbarPipeline(const std::string& local_filename)
    : local_filename_(local_filename),
      pipeline_(nullptr),
      convert_element_(nullptr),
      success_(false),
      running_(false) {}

MoodbarPipeline::~MoodbarPipeline() { Cleanup(); }

bool MoodbarPipeline::IsAvailable() {
  if (!sIsAvailable) {
    GstElementFactory* factory = gst_element_factory_find("fftwspectrum");
    if (!factory) {
      return false;
    }
    gst_object_unref(factory);

    sIsAvailable = true;
  }

  return sIsAvailable;
}

GstElement* MoodbarPipeline::CreateElement(const std::string& factory_name) {
  GstElement* ret =
      gst_element_factory_make(factory_name.c_str(), nullptr);

  if (ret) {
    gst_bin_add(GST_BIN(pipeline_), ret);
  } else {
    std::cerr << "WARNING: Unable to create gstreamer element " << factory_name << std::endl;
  }

  return ret;
}

void MoodbarPipeline::Start() {
  if (pipeline_) {
    return;
  }

  pipeline_ = gst_pipeline_new("moodbar-pipeline");

  GstElement* decodebin = CreateElement("uridecodebin");
  convert_element_ = CreateElement("audioconvert");
  GstElement* spectrum = CreateElement("fastspectrum");
  GstElement* fakesink = CreateElement("fakesink");

  if (!decodebin || !convert_element_ || !spectrum || !fakesink) {
    pipeline_ = nullptr;
    if (Finished)
      Finished(false);
    return;
  }

  // Join them together
  if (!gst_element_link(convert_element_, spectrum) ||
      !gst_element_link(spectrum, fakesink)) {
    std::cerr << "ERROR: Failed to link elements" << std::endl;
    pipeline_ = nullptr;
    if (Finished)
      Finished(false);
    return;
  }

  builder_.reset(new MoodbarBuilder);

  // Set properties
  g_object_set(decodebin, "uri", local_filename_.c_str(),
               nullptr);
  g_object_set(spectrum, "bands", kBands, nullptr);

  GstFastSpectrum* fast_spectrum = GST_FASTSPECTRUM(spectrum);
  fast_spectrum->output_callback = [this](
      double* magnitudes, int size) { builder_->AddFrame(magnitudes, size); };

  // Connect signals
  g_signal_connect(decodebin, "pad-added", G_CALLBACK(&NewPadCallback), this);
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
  gst_bus_set_sync_handler(bus, BusCallbackSync, this, nullptr);
  gst_object_unref(bus);

  // Start playing
  running_ = true;
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void MoodbarPipeline::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  std::string message = error->message;

  g_error_free(error);
  free(debugs);

  std::cerr << "ERROR: Error processing " << local_filename_ << " : " << message << std::endl;
}

void MoodbarPipeline::NewPadCallback(GstElement*, GstPad* pad, gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);

  if (!self->running_) {
    return;
  }

  GstPad* const audiopad =
      gst_element_get_static_pad(self->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    std::cerr << "WARNING: audiopad is already linked, unlinking old pad" << std::endl;
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);

  int rate = 0;
  GstCaps* caps = gst_pad_get_current_caps(pad);
  GstStructure* structure = gst_caps_get_structure(caps, 0);
  gst_structure_get_int(structure, "rate", &rate);
  gst_caps_unref(caps);

  if (self->builder_ != nullptr)
    self->builder_->Init(kBands, rate);
}

GstBusSyncReply MoodbarPipeline::BusCallbackSync(GstBus*, GstMessage* msg,
                                                 gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      self->Stop(true);
      break;

    case GST_MESSAGE_ERROR:
      self->ReportError(msg);
      self->Stop(false);
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

void MoodbarPipeline::Stop(bool success) {
  success_ = success;
  running_ = false;
  if (builder_ != nullptr) {
    data_ = builder_->Finish(1000);
    builder_.reset();
  }

  if (Finished)
    Finished(success);
}

void MoodbarPipeline::Cleanup() {
  running_ = false;
  if (pipeline_) {
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
    gst_object_unref(bus);
    
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
  }
}

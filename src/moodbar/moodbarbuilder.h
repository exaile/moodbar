/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef MOODBARBUILDER_H
#define MOODBARBUILDER_H

#include <cstddef>
#include <cstdint>
#include <vector>

class MoodbarBuilder {
 public:
  MoodbarBuilder();

  void Init(int bands, int rate_hz);
  void AddFrame(const double* magnitudes, std::size_t size);
  std::vector<std::uint8_t> Finish(int width);

 private:
  struct Rgb {
    Rgb() : r(0), g(0), b(0) {}
    Rgb(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}

    double r, g, b;
  };

  int BandFrequency(int band) const;
  static void Normalize(std::vector<Rgb>* vals, double Rgb::*member);

  std::vector<unsigned int> barkband_table_;
  int bands_;
  int rate_hz_;

  std::vector<Rgb> frames_;
};

#endif // MOODBARBUILDER_H

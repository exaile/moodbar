#!/usr/bin/env python3

# Test script for Moodbar
# Copyright (C) 2018  Johannes Sasongko <sasongko@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


import os
import shlex
import subprocess
import tempfile
import unittest

import numpy as np


GST_LAUNCH = shlex.split(os.environ.get('GST_LAUNCH', 'gst-launch-1.0'))
MOODBAR_EXE_DEFAULT = './moodbar'
MOODBAR = shlex.split(os.environ.get('MOODBAR', MOODBAR_EXE_DEFAULT))


def create_dummy_audio(path: str) -> None:
    """Create audio file for testing.

    The file contains low, medium, and high frequency audio (in that order), and
    should show up in Moodbar as red, green, and blue equal-length segments.

    The file uses Vorbis codec in Ogg container.

    This function requires gst-launch-1.0 to be present in PATH.
    """
    cmd = GST_LAUNCH + [
        'audiotestsrc', 'freq=100', 'num-buffers=100', 'volume=0.4', '!', 'concat', 'name=c',
        '!', 'vorbisenc', 'bitrate=32000', '!', 'oggmux',
        '!', 'filesink', 'location='+path,
        'audiotestsrc', 'freq=2000', 'num-buffers=100', 'volume=0.2', '!', 'c.',
        'audiotestsrc', 'freq=4000', 'num-buffers=100', 'volume=0.1', '!', 'c.',
    ]
    subprocess.check_call(cmd, stdout=subprocess.DEVNULL)


def call_moodbar(inpath: str, outpath: str):
    cmd = MOODBAR + ['-o', outpath, inpath]
    try:
        return subprocess.check_call(cmd)
    except FileNotFoundError as e:
        if e.filename == MOODBAR_EXE_DEFAULT:
            raise FileNotFoundError("Could not find moodbar executable. "
                "Make sure you run this test from the build directory.") from e
        raise


class MoodbarTest(unittest.TestCase):
    def assertArrayAlmostEqual(self, first, second, delta, msg=None):
        if not np.all(np.abs(first - second) < delta):
            if msg is None:
                msg = f"{first} != {second} within {delta} delta"
            raise AssertionError(msg)

    def test_unicode(self):
        """Test that non-Latin paths work."""
        with tempfile.TemporaryDirectory(prefix='moodbar-test.') as tmpdir:
            audiopath = os.path.join(tmpdir, '你好.ogg')
            create_dummy_audio(audiopath)
            moodpath = os.path.join(tmpdir, '안녕.mood')
            call_moodbar(audiopath, moodpath)

    def test_colors(self):
        """Test that colors are close enough to the expected ones."""

        with tempfile.TemporaryDirectory(prefix='moodbar-test.') as tmpdir:
            audiopath = os.path.join(tmpdir, 'test.ogg')
            create_dummy_audio(audiopath)
            moodpath = os.path.join(tmpdir, 'test.mood')
            call_moodbar(audiopath, moodpath)
            mood = np.fromfile(moodpath, dtype=np.uint8)
        mood = mood.reshape(-1, 3)

        length = mood.shape[0]
        red = mood[length*1//12 : length*3//12].mean(0)
        green = mood[length*5//12 : length*7//12].mean(0)
        blue = mood[length*9//12 : length*11//12].mean(0)
        delta = [30, 30, 30]
        self.assertArrayAlmostEqual(red, [255, 0, 0], delta)
        self.assertArrayAlmostEqual(green, [0, 255, 0], delta)
        self.assertArrayAlmostEqual(blue, [0, 0, 255], delta)


if __name__ == '__main__':
    unittest.main()

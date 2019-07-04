# Moodbar

Moodbar is a music visualization method that assigns colors to different parts of a track and presents them as a timeline.
Applied to a music player, the main goal is to help the user navigate within a particular track.
For example, if the user wants to skip to the first chorus of a song, a good moodbar implementation should be able to provide a hint of where this might occur in the timeline.

![Moodbar example](https://user-images.githubusercontent.com/8440927/38452195-b894a060-3a2e-11e8-8573-acb542630774.png)

The particular moodbar implementation in this repository is based on the Bandwise Spectral Magnitude method presented in [*On Techniques for Content-Based Visual Annotation to Aid Intra-Track Music Navigation*](https://ismir2005.ismir.net/proceedings/1023.pdf) (Gavin Wood & Simon O'Keefe, 2005).
It divides the track into small chunks and assigns a color to each chunk: the red channel represents audio levels in the low frequencies, green for mid frequencies, and blue for high frequencies.

The code is mostly taken from [Clementine](https://www.clementine-player.org/), with the addition of a command-line interface that is a drop-in alternative to [the original Moodbar program](https://userbase.kde.org/Amarok/Manual/Various/Moodbar).
Clementine's moodbar implementation used to be based on the original project's, but they have since diverged greatly.
Both implement the same idea and the outputs should be roughly similar.

Files in the `gst` and `src` directories are directly taken from Clementine's repository with slight modifications to remove some extra dependencies.
The initial commit is taken from Clementine revision 3886f3d1e4b29d028c1bacf474bc40d1c45c2ea9 (2014-12-22), last sync is at revision 55edcf5321051e44281f067a7e3ee44871982c12 (2019-03-11).


## Requirements

* FFTW 3
* GStreamer 1
  * GStreamer Base Plugins

For building:

* Development files for the above
* C++ compiler
* Meson. If your OS doesn't have a package for this, you can install it through pip (requires Python 3).
* pkgconf or pkg-config
* Ninja

At runtime you may also need other GStreamer plugin packages to read your audio files.
For example, to process MP3 files you may have to install GStreamer Ugly Plugins.


## Building & installing

```sh
meson --buildtype=release build/
cd build/
ninja
sudo ninja install
```

You can add `-Db_lto=true` to the `meson` call to produce slightly more efficient code.


### For packagers

Building with custom flags and prefix, and staging to destdir:

```sh
env CXXFLAGS=... LDFLAGS=... meson --buildtype=plain --prefix=... builddir
ninja -C builddir
env DESTDIR=... ninja -C builddir install
```

See the [Meson documentation](http://mesonbuild.com/Quick-guide.html#using-meson-as-a-distro-packager) for more info.


## Usage

```sh
moodbar -o OUTPUT INPUT
```

This creates an output file containing pixel values in `R1 G1 B1 R2 G2 B2 ...` format.


## Testing

You can run `meson test -v` inside the build directory to run some unit tests.
The tests require the following:

* Python 3
* NumPy
* `gst-launch-1.0`. If this is named differently in your system or is not in PATH, point the GST_LAUNCH environment variable to it.


## Contributing

When you contribute to a file in this project, you agree to release your contribution under the same license specified in the file's license header.
If there is no license header in the file, or if it's a new file, you agree to release your contribution under the GNU General Public License version 3 or later (GPL-3.0+), or you may specify another license that is compatible with GPL-3.0+.

Code in the `gst` and `src` directories must be kept as close as possible to their original versions in the Clementine repository.
The only permitted changes are to remove additional dependencies, to fix build errors/warnings, and to fix serious (e.g. security) issues.

Other C or C++ code must be formatted with clang-format using the default settings.

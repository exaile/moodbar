# Moodbar

An implementation of the [Moodbar](https://sjohannes.wordpress.com/2015/10/16/the-moodbar-audio-visualisation-method/) audio timeline visualization.

This is a drop-in replacement for the outdated [Moodbar](https://userbase.kde.org/Amarok/Manual/Various/Moodbar) program originally developed for Amarok.

Files in the `gst` and `src` directories are taken from [Clementine](https://www.clementine-player.org/) (revision 3886f3d1e4b29d028c1bacf474bc40d1c45c2ea9, 2014-12-22) with slight modifications to remove some dependencies.


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

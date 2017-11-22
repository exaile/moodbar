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
CXXFLAGS=... LDFLAGS=... meson --buildtype=plain --prefix=... builddir
ninja -C builddir
DESTDIR=... ninja -C builddir install
```

See the [Meson documentation](http://mesonbuild.com/Quick-guide.html#using-meson-as-a-distro-packager) for more info.


## Usage

```sh
moodbar -o OUTPUT INPUT
```

This creates an output file containing pixel values in `R1 G1 B1 R2 G2 B2 ...` format.

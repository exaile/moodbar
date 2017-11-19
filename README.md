# Moodbar

An implementation of the [Moodbar](https://sjohannes.wordpress.com/2015/10/16/the-moodbar-audio-visualisation-method/) audio timeline visualization.

This is a drop-in replacement for the outdated [Moodbar](https://userbase.kde.org/Amarok/Manual/Various/Moodbar) program originally developed for Amarok.

Files in the `gst` and `src` directories are taken from [Clementine](https://www.clementine-player.org/) (revision 3886f3d1e4b29d028c1bacf474bc40d1c45c2ea9, 2014-12-22) with slight modifications to remove some dependencies.


## Requirements

* FFTW 3
* GStreamer 1 + base plugins

For building:

* Development files for the above
* C++ compiler
* Meson. If your OS doesn't have a package for this, you can install it through pip (requires Python 3).
* pkgconf or pkg-config
* Ninja


## Building

```sh
meson --buildtype=release build/
cd build/
ninja
```

The output is a single `moodbar` executable.

If you are creating an OS package, use `--buildtype=plain` so you have full control over build flags through the `CXXFLAGS` and `LDFLAGS` environment variables.
See the [Meson documentation](http://mesonbuild.com/Quick-guide.html#using-meson-as-a-distro-packager) for more info.


## Usage

```sh
moodbar -o OUTPUT INPUT
```

This creates an output file containing pixel values in `R1 G1 B1 R2 G2 B2 ...` format.

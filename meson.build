project('moodbar', 'cpp',
  version : '1.3.0',
  license : 'GPL-3.0+',
  meson_version : '>=0.35',
  default_options : 'cpp_std=c++17',
)

conf = configuration_data()
conf.set_quoted('MOODBAR_VERSION_STR', meson.project_version())
configure_file(output : 'moodbar-config.h', configuration : conf)

dep_gstreamer = dependency('gstreamer-1.0')

gstfastspectrum = static_library('gstfastspectrum',
  'gst/moodbar/gstfastspectrum.cpp',
  'gst/moodbar/plugin.cpp',
  dependencies : [
    dep_gstreamer,
    dependency('gstreamer-audio-1.0'),
    dependency('fftw3'),
  ],
)

moodbar = static_library('moodbar',
  'moodbar.cpp',
  'src/moodbar/moodbarbuilder.cpp',
  'src/moodbar/moodbarpipeline.cpp',
  include_directories : include_directories('src'),
  link_with : gstfastspectrum,
  dependencies : [
    dependency('glib-2.0'),
    dep_gstreamer,
    dependency('gstreamer-plugins-base-1.0'),
  ],
)

moodbar_exe = executable('moodbar',
  'moodbar_exe.cpp',
  include_directories : include_directories('src'),
  link_with : moodbar,
  link_args : host_machine.system() == 'windows' ? ['-municode'] : [],
  dependencies : [
    dependency('gio-2.0'),
    dep_gstreamer,
  ],
  install : true,
)

test('test', find_program('test.py'),
  workdir : meson.current_build_dir(),
)

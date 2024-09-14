project(
  'Zycore',
  'c',
  version: '1.5.0',
  license: 'MIT',
  license_files: 'LICENSE',
  meson_version: '>=1.3',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'warning_level=3',
    'b_lto=true',
  ],
)

cc = meson.get_compiler('c')

if cc.get_argument_syntax() == 'msvc'
  if get_option('b_lto')
    add_project_arguments(
      '/GL', # -flto
      language: 'c',
    )
    add_project_link_arguments(
      '/LTCG',
      language: 'c',
    )
  endif
endif

inc = include_directories('include')

dep = []

hdrs_api = files(
  # API
  'include/Zycore/API/Memory.h',
  'include/Zycore/API/Process.h',
  'include/Zycore/API/Synchronization.h',
  'include/Zycore/API/Terminal.h',
  'include/Zycore/API/Thread.h',
)

hdrs_common = files(
  # Common
  'include/Zycore/Allocator.h',
  'include/Zycore/ArgParse.h',
  'include/Zycore/Atomic.h',
  'include/Zycore/Bitset.h',
  'include/Zycore/Comparison.h',
  'include/Zycore/Defines.h',
  'include/Zycore/Format.h',
  'include/Zycore/LibC.h',
  'include/Zycore/List.h',
  'include/Zycore/Object.h',
  'include/Zycore/Status.h',
  'include/Zycore/String.h',
  'include/Zycore/Types.h',
  'include/Zycore/Vector.h',
  'include/Zycore/Zycore.h',
)

hdrs_internal = files(
  'include/Zycore/Internal/AtomicGNU.h',
  'include/Zycore/Internal/AtomicMSVC.h',
)

hdrs = hdrs_api + hdrs_common + hdrs_internal

src = files(
  # API
  'src/API/Memory.c',
  'src/API/Process.c',
  'src/API/Synchronization.c',
  'src/API/Terminal.c',
  'src/API/Thread.c',
  # Common
  'src/Allocator.c',
  'src/ArgParse.c',
  'src/Bitset.c',
  'src/Format.c',
  'src/List.c',
  'src/String.c',
  'src/Vector.c',
  'src/Zycore.c',
)

if host_machine.system() == 'windows'
  windows = import('windows')
  src += windows.compile_resources('resources/VersionInfo.rc')
endif

if get_option('nolibc')
  add_project_arguments(
    '-DZYAN_NO_LIBC',
    language: 'c',
  )
  dep += declare_dependency(link_args: ['-nostdlib', '-nodefaultlibs'])
elif host_machine.system() == 'linux'
  add_project_arguments(
    '-D_GNU_SOURCE',
    language: 'c',
  )
  dep += dependency('threads')
endif

zycore_lib = library(
  'Zycore',
  src + hdrs,
  c_static_args: ['-DZYCORE_STATIC_BUILD'],
  include_directories: inc,
  implicit_include_directories: false,
  dependencies: dep,
  version: meson.project_version(),
  install: true,
)

install_headers(hdrs_common, subdir: 'Zycore')
install_headers(hdrs_api, subdir: 'Zycore/API')
install_headers(hdrs_internal, subdir: 'Zycore/Internal')

zycore_dep = declare_dependency(
  include_directories: inc,
  link_with: zycore_lib,
)

if not get_option('nolibc')
  gtest_dep = dependency('gtest', required: false)
  if gtest_dep.found() and add_languages('cpp', native: false)
    test(
      'string',
      executable(
        'test_string',
        'tests/String.cpp',
        dependencies: [gtest_dep, zycore_dep],
      ),
      protocol: 'gtest',
    )
    test(
      'vector',
      executable(
        'test_vector',
        'tests/Vector.cpp',
        dependencies: [gtest_dep, zycore_dep],
      ),
      protocol: 'gtest',
    )
    test(
      'argparse',
      executable(
        'test_argparse',
        'tests/ArgParse.cpp',
        dependencies: [gtest_dep, zycore_dep],
      ),
      protocol: 'gtest',
    )
  endif
endif

pkg = import('pkgconfig')
pkg.generate(
  zycore_lib,
  name: 'zycore',
  description: 'Zyan Core Library for C',
  url: 'https://github.com/zyantific/zycore-c',
)

meson.override_dependency('zycore', zycore_dep)

project(
    'libgit4cpp',
    [ 'cpp' ],
    version : run_command([
            find_program('cat'),
            meson.current_source_dir() / 'LIBNO'
        ],
        check : true,
    ).stdout().split('=')[1].strip(),
    default_options : [
        'cpp_std=c++17',
        'warning_level=3',
        'buildtype=release',
        'prefix=/usr',
        'libdir=lib',
        'b_lundef=false', # use but do not link GUL
    ],
    meson_version : '>=0.49'
)

pkg_config_name = 'libgit4cpp'

## Determine the version number

# By default, the version number is simply the project version. If an alternative version
# is specified by a Meson command line switch like "-Dversion=21.7.2-precise5", it is
# used instead. The version part has at least 3 unsigned numerals separated by dots X.Y.Z
# with all numerals < 256.
# Any additional patchlevel string, consisting of alnum characters and appended with
# a dot separator, is used unchanged for the dynamic library version (except on Darwin,
# where it is ignored): X.Y.Z.p
# Note that it can be problematic to use '-' or '_' in the p part.

libno_raw = get_option('version')
if libno_raw == ''
    libno_raw = meson.project_version()
endif
libno_parts = libno_raw.split('.')
if libno_parts.length() < 3
    error('Version number "' + libno_raw + '" does not contain three fields X.Y.Z[.p]')
endif
libno = '.'.join([ libno_parts[0], libno_parts[1], libno_parts[2].split('-')[0] ])
libno_full = '.'.join(libno_parts)

## Add slightly modified dependency for std::filesystem during linking (on some Linux OS,
## for example Debian 10, "Buster", we do not have an automated linker option although we
## still building on C++17 standard - amazing)
## See StackOverflow:
## https://stackoverflow.com/questions/52516165/cannot-use-the-c-stdfilesystem-library-with-meson-build
add_project_link_arguments([ '-lstdc++fs' ], language : 'cpp')

inc = [ ]

## Build library
subdir('include')
subdir('src')

gul_dep = dependency('gul17', fallback : [ 'gul17', 'libgul_dep' ])
libgit2_dep = dependency('libgit2')

deps = [
    gul_dep.partial_dependency(compile_args : true, includes : true),
    libgit2_dep,
]

# libgit2 has an API change at some point; a check might become useful in the future
libgit2_version_parts = libgit2_dep.version().split('.')
libgit2_fullversion = 'LIBGIT2_FULLVERSION=@0@'.format(
    (libgit2_version_parts[0].to_int() * 1000 + libgit2_version_parts[1].to_int()) * 1000
    + libgit2_version_parts[2].to_int())
message(libgit2_fullversion)
local_compiler_args = [ '-D' + libgit2_fullversion ]

lib = both_libraries('git4cpp',
    sources,
    dependencies : deps,
    soversion : libno_full,
    darwin_versions : libno,
    include_directories : inc,
    build_rpath : meson.current_build_dir(),
    install_rpath : get_option('prefix') / get_option('libdir'),
    #gnu_symbol_visibility : 'hidden',
    cpp_args : local_compiler_args,
    install : true)


## pkg-config

pkg = import('pkgconfig')
pkg.generate(lib,
    name : pkg_config_name,
    description : 'Taskolib',
    version : libno,
    filebase : pkg_config_name,
    libraries : [ '-Wl,-rpath,${libdir}' ],
    requires : [ 'gul17', 'libgit2' ],
)


## Build tests

libgit4cpp_dep = declare_dependency(
    include_directories : inc,
    link_with : lib,
    dependencies : [ deps, gul_dep, ],
)

subdir('tests')


## Build message

message('Building library version @0@'.format(libno))
if libno != libno_full
    message('(!)          .so version @0@'.format(libno_full))
endif
if meson.project_version().version_compare('!= ' + libno)
    message('(!)      project version @0@'.format(meson.project_version()))
endif

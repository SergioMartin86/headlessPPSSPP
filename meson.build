project('headlessPPSSPP','c','cpp',
  version: '1.0.0',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
  subproject_dir : 'extern'
)

# Grabbing dependencies dependency

jaffarCommonSubproject = subproject('jaffarCommon', default_options : [ 'includeFFmpeg=true', 'includeZlib=true', 'includeSDL2_TTF=true'])
jaffarCommonDependency = jaffarCommonSubproject.get_variable('jaffarCommonDependency')
ffmpegLibrary = jaffarCommonSubproject.get_variable('ffmpegLibrary')
zlibLibrary = jaffarCommonSubproject.get_variable('zlibLibrary')
SDL2_TTFDependency = jaffarCommonSubproject.get_variable('SDL2_TTFDependency')

# Loading dependencies
subdir('source')

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

# Common application flags
commonCompileArgs = [ '-Wall', '-Wfatal-errors' ]

# Building playback tool

if get_option('buildPlayer') == true

  newPlayer = executable('player',
    'source/player.cpp',
    cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
    dependencies        : [ ppssppDependency,
                            jaffarCommonDependency
                          ],
    link_with           : [ ffmpegLibrary, zlibLibrary ],
    include_directories : include_directories(['source']),
    link_args           : [ '-lncurses' ],
  )
  
endif

# Building tester tool

ntester = executable('tester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs ], 
  dependencies        : [ ppssppDependency, jaffarCommonDependency ],
  link_with           : [ ffmpegLibrary, zlibLibrary ],
)

# Building tester tool for the original emulator

# Building tests
#subdir('tests')

endif # If not subproject

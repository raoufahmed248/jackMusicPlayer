set(wavPlayerLibSrc_SOURCE_FILES wavPlayer.cpp)

include_directories(../inc)


add_library(wavPlayerLibSrc STATIC ${wavPlayerLibSrc_SOURCE_FILES})

# Specify the include directories exported by this library
target_include_directories(wavPlayerLibSrc PUBLIC ../inc)

cmake_minimum_required(VERSION 3.4.1)

# Create lists with all source files
file(GLOB jni_SRCS "${CMAKE_SOURCE_DIR}/src/main/jni/*.cpp")
file(GLOB vibrato_SRCS "${CMAKE_SOURCE_DIR}/src/main/jni/Vibrato-effect/BerVibrato/*.cpp")

# Default flags
set_property(SOURCE ${jni_SRCS}
             APPEND_STRING PROPERTY COMPILE_FLAGS "-O3")

# Create shared library called superpoweredeffect and list necessary sources
add_library(superpoweredeffect SHARED
            ${jni_SRCS}
            ${vibrato_SRCS}
            src/main/jni/Superpowered/AndroidIO/SuperpoweredAndroidAudioIO.cpp)

# Add include directories
target_include_directories(superpoweredeffect PRIVATE
                           src/main/jni
                           src/main/jni/Superpowered)
# Print the Android ABI
message(${ANDROID_ABI})

# List additional libraries which need to be linked into/with our library
target_link_libraries(superpoweredeffect
                      android
                      OpenSLES
                      log
                      ${CMAKE_SOURCE_DIR}/src/main/jni/Superpowered/libSuperpoweredAndroid${ANDROID_ABI}.a)
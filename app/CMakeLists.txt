cmake_minimum_required(VERSION 3.4.1)

# Create lists with all source files
file(GLOB jni_SRCS "${CMAKE_SOURCE_DIR}/src/main/jni/*.cpp")
file(GLOB vibrato_SRCS "${CMAKE_SOURCE_DIR}/src/main/jni/Vibrato-effect/BerVibrato/*.cpp")

# Default flags
set_property(SOURCE ${jni_SRCS}
             APPEND_STRING PROPERTY COMPILE_FLAGS "-O3 -Wno-error=format-security")

add_library(superpoweredeffect SHARED
            ${jni_SRCS}
            ${vibrato_SRCS}
            src/main/jni/Superpowered/AndroidIO/SuperpoweredAndroidAudioIO.cpp)

target_include_directories(superpoweredeffect PRIVATE
                           src/main/jni
                           src/main/jni/Superpowered)

message(${ANDROID_ABI})

target_link_libraries(superpoweredeffect
                      android
                      OpenSLES
                      log
                      ${CMAKE_SOURCE_DIR}/src/main/jni/Superpowered/libSuperpoweredAndroid${ANDROID_ABI}.a)
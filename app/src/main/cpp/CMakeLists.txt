# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("androidtinygl")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# TinyGL
set(tinygl_srcs
        tinygl/src/accum.c
        tinygl/src/api.c
        tinygl/src/arrays.c
        tinygl/src/clear.c
        tinygl/src/clip.c
        tinygl/src/get.c
        tinygl/src/image_util.c
        tinygl/src/init.c
        tinygl/src/light.c
        tinygl/src/list.c
        tinygl/src/matrix.c
        tinygl/src/memory.c
        tinygl/src/misc.c
        tinygl/src/msghandling.c
        tinygl/src/select.c
        tinygl/src/specbuf.c
        tinygl/src/texture.c
        tinygl/src/vertex.c
        tinygl/src/zbuffer.c
        tinygl/src/zline.c
        tinygl/src/zmath.c
        tinygl/src/zpostprocess.c
        tinygl/src/zraster.c
        tinygl/src/ztext.c
        tinygl/src/ztriangle.c
        )


add_library( # Sets the name of the library.
        androidtinygl

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp

        kkImage.cpp
        kkFileUtils.cpp

        gl_hello.cpp
        gl_gears.cpp
        gl_texture.cpp
        gl_menu.cpp
        gl_model.cpp
        gl_hello2.cpp

        gl_touch_event.cpp
        renderer_fixed.cpp

        ${tinygl_srcs})
target_include_directories(androidtinygl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tinygl/include)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log

        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        androidtinygl

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}

        jnigraphics

        android # AAsset_close
        )
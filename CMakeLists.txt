cmake_minimum_required(VERSION 3.21)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Declare the project
project(sdl-min LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sanitizer build type options.
# This allows you to build with address/thread sanitizer by using
# -DCMAKE_BUILD_TYPE=Asan or Tsan on the generator.
# From https://stackoverflow.com/a/64294837
if(NOT MSVC)
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
    endif()
    if(NOT "Tsan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Tsan)
    endif()
else()
    set(allowedBuildTypes Asan Tsan Debug Release RelWithDebInfo MinSizeRel)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

# Asan
set(CMAKE_C_FLAGS_ASAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address,leak,undefined -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,leak,undefined -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,leak,undefined" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address,leak,undefined" CACHE STRING
    "Linker lags to be used to create shared libraries for Asan build type." FORCE)

# Tsan
set(CMAKE_C_FLAGS_TSAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=thread,undefined -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_TSAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread,undefined -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_TSAN
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread,undefined" CACHE STRING
    "Linker flags to be used to create executables for Tsan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_TSAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=thread,undefined" CACHE STRING
    "Linker lags to be used to create shared libraries for Tsan build type." FORCE)
endif()

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

include(FetchContent)

# Fetch dependencies
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	e7970a8b26732f1b0df9690f7180546f8c30e48e
)

FetchContent_MakeAvailable(glm)

# Create an executable or a shared library based on the platform and add our sources to it
if (ANDROID)
	# The SDL java code is hardcoded to load libmain.so on android, so we need to change EXECUTABLE_NAME
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
	add_executable(${EXECUTABLE_NAME})
endif()

target_sources(${EXECUTABLE_NAME}
PRIVATE
    src/time.cpp
    src/shaders.cpp
    src/renderer.cpp
    src/main.cpp
    src/img.cpp
    src/font.cpp
    src/common.cpp
    src/button.cpp
    src/block.cpp
    src/block_renderer.cpp
    src/iosLaunchScreen.storyboard
    src/Sample.appxManifest

    # imgui
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
)

# What is iosLaunchScreen.storyboard? This file describes what Apple's mobile platforms
# should show the user while the application is starting up. If you don't include one,
# then you get placed in a compatibility mode that does not allow HighDPI.
# This file is referenced inside Info.plist.in, where it is marked as the launch screen file.
# It is also ignored on non-Apple platforms.

# What is Sample.appxManifest? This file describes deployment settings for Universal Windows Platform
# applications, like splash screen and icon resolutions, which are required for the application to run.
# It is ignored on other platforms. Below, we add those images.
if (WINDOWS_STORE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "src/logo44.png" "src/logo150.png" "src/logo50.png" "src/logo620x300.png")
elseif(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "src/logo.png")
endif()

target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# If targeting Windows UWP, enable Windows Runtime Compilation
# if using a C++ version older than C++20, you also need /ZW
if(WINDOWS_STORE)
    target_compile_options(${EXECUTABLE_NAME} PUBLIC "/EHsc")
endif()

set(SDL_X11_XCURSOR OFF CACHE BOOL "" FORCE)
set(SDL_X11_XINPUT OFF CACHE BOOL "" FORCE)
set(SDL_X11_XFIXES OFF CACHE BOOL "" FORCE)
set(SDL_X11_XRANDR OFF CACHE BOOL "" FORCE)
set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)
set(SDL_X11_XTEST OFF CACHE BOOL "" FORCE)
set(SDL_MAIN_USE_CALLBACKS ON CACHE BOOL "" FORCE)

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# Link SDL to our executable. This also makes its include directory available to us.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# Configure SDL_ttf by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# Link SDL_ttf to our executable. This also makes its include directory available to us.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3_ttf::SDL3_ttf)

# Link GLEW
find_package(GLEW)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE GLEW::GLEW)

# Link OpenGL
find_package(OpenGL REQUIRED)
if(WIN32)
    # pkgconf on MSYS2 doesn't find OpenGL::OpenGL for some reason.
    set(OpenGL_GL_PREFERENCE GLVND)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE OpenGL::GL)
else()
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE OpenGL::OpenGL)
endif()


# Link GLM
target_link_libraries(${EXECUTABLE_NAME} PRIVATE glm)

# Link PNG for cImg
find_package(PNG)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE PNG::PNG)

# Enable PNG support for cImg
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE cimg_use_png)
# Do not include display for cImg (would require linking against x11)
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE cimg_display=0)

# FOR ACTUAL GAME SRC THAT IS NOT SDL:
# We need this directory, and users of the library will need it too.
target_include_directories(${EXECUTABLE_NAME} PUBLIC
    # We go up a directory so that the source files can include the
    # whole path and avoid collisions for the same file names, e.g. "src/time.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

# imgui
target_include_directories(${EXECUTABLE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp
)

# set some extra configs for each platform
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
# note that fixup_bundle does not work on iOS, so you will want to use static libraries
# or manually copy dylibs and set rpaths
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    INSTALL(CODE
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"\")
        "
        COMPONENT Runtime
    )
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
)

# Copy the assets folder into the executable folder.
add_custom_command(
    TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/assets
        ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets
)

# On Windows, copy the DLLs to the build directory
if(WIN32)
    set(MSYS2_BIN "C:/msys64/ucrt64/bin")

    # Manually add the required DLLs.
    # These can be found with ldd, e.g., ldd .build-win64/Debug/sdl-min.exe | grep ucrt64
    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MSYS2_BIN}/glew32.dll"
            "${MSYS2_BIN}/libbrotlicommon.dll"
            "${MSYS2_BIN}/libbrotlidec.dll"
            "${MSYS2_BIN}/libbz2-1.dll"
            "${MSYS2_BIN}/libfreetype-6.dll"
            "${MSYS2_BIN}/libgcc_s_seh-1.dll"
            "${MSYS2_BIN}/libglib-2.0-0.dll"
            "${MSYS2_BIN}/libgraphite2.dll"
            "${MSYS2_BIN}/libharfbuzz-0.dll"
            "${MSYS2_BIN}/libiconv-2.dll"
            "${MSYS2_BIN}/libintl-8.dll"
            "${MSYS2_BIN}/libpcre2-8-0.dll"
            "${MSYS2_BIN}/libpng16-16.dll"
            "${MSYS2_BIN}/libstdc++-6.dll"
            "${MSYS2_BIN}/libwinpthread-1.dll"
            "${MSYS2_BIN}/zlib1.dll"
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
    )
endif()

install(TARGETS ${EXECUTABLE_NAME} BUNDLE DESTINATION ./install)

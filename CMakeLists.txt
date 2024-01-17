cmake_minimum_required(VERSION 3.12)
project(cpp_raytracer
    LANGUAGES CXX)

# Notify user if their build generator is multi-configuration, because this prevents
# us from setting a default `CMAKE_BUILD_TYPE` (as multi-config generators ignore
# the `CMAKE_BUILD_TYPE` variable).
if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "\
NOTE: You are on a multi-configuration generator (VSCode, XCode, etc). This means\n\
the build type (Debug, Release, etc) should be set from within the IDE itself, because\n\
multi-configuration generators ignore the CMAKE_BUILD_TYPE variable)."
    )
endif()

# Set the default build type to Release if `CMAKE_BUILD_TYPE` has not been set previously,
# and if the build generator is single-configuration (because if it is multi-config, then
# it ignores `CMAKE_BUILD_TYPE`). From https://www.kitware.com/cmake-and-the-default-build-type/.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "NOTE: Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE
        STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmakde-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CPP_RAYTRACER_SOURCES
    src/main.cpp)

add_executable(cpp_raytracer ${CPP_RAYTRACER_SOURCES})

# Require C++20 for `cpp_raytracer` (and because I use PUBLIC, also for all targets that link to
# `cpp_raytracer`), and also avoid extensions being added.
target_compile_features(cpp_raytracer PUBLIC cxx_std_20)
set_target_properties(cpp_raytracer PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(cpp_raytracer PRIVATE ${CMAKE_SOURCE_DIR}/include)

# The following is "the modern way to add OpenMP to a target" (cpp_raytracer is the target here).
# (See https://cliutils.gitlab.io/modern-cmake/chapters/packages/OpenMP.html).
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cpp_raytracer PUBLIC OpenMP::OpenMP_CXX)
else()
    message(STATUS "Could not find OpenMP (libomp); compiling without it")
endif()
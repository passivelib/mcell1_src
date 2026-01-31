# tutorial https://www.youtube.com/watch?v=mKZ-i-UfGgQ&ab_channel=kandodev
# version 3.11 or later of CMake needed later for installing GoogleTest
# so let's require it now.

set(CMAKE_C_COMPILER "/daten/software/musl_tools/gcc/bin/x86_64-linux-musl-gcc-9.4.0")
#set(CMAKE_C_COMPILER "/usr/bin/gcc-9")
set(CMAKE_CXX_COMPILER "/daten/software/musl_tools/gcc/bin/x86_64-linux-musl-g++")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++-9")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(ROOT_DIR ${SRC_DIR}/test/top)
set(LIB_DIR ${SRC_DIR}/../lib)

cmake_minimum_required(VERSION 3.11)
project(mcell)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------
# build type
# ------------------------------------------------------
#set(CMAKE_BUILD_TYP Debug)
set(CMAKE_BUILD_TYP Release)

# ------------------------------------------------------
# compiler 
# ------------------------------------------------------
ENABLE_LANGUAGE(C)
ENABLE_LANGUAGE(CXX)
ENABLE_LANGUAGE(Fortran)

# ------------------------------------------------------
# C compiler flags imporatnt for zlib 
# ------------------------------------------------------
set(CMAKE_C_FLAGS "-DHAVE_UNISTD_H")

# ------------------------------------------------------
# C++ compiler flags
# ------------------------------------------------------
#set(CMAKE_CXX_FLAGS "-O2 -DNDEBUG -s")
set(CMAKE_CXX_FLAGS "-O2 -s")
#set(CMAKE_CXX_FLAGS "-O2 -pg")
#set(CMAKE_CXX_FLAGS "-O2 -ffast-math")
#set(CMAKE_CXX_FLAGS "-ggdb3")
#set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
# ------------------------------------------------------
# c++ standard
# ------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ------------------------------------------------------
# linker
# ------------------------------------------------------
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(CMAKE_EXE_LINKER_FLAGS " -static")
#add_link_options("-fuse-ld=lld")

# ------------------------------------------------------
# source files files
# ------------------------------------------------------
file(GLOB APP_SOURCES 
${SRC_DIR}/ext_*/*.cpp 
${SRC_DIR}/ext_*/*.c 
${SRC_DIR}/ext_*/*.cc 
${SRC_DIR}/ext_gdstk/gdstk/*.c 
${SRC_DIR}/ext_gdstk/gdstk/*.cpp 
${SRC_DIR}/ext_gdstk/gdstk/libqhull_r/*.c 
${SRC_DIR}/ext_gdstk/gdstk/libqhull_r/*.cpp 
${SRC_DIR}/ext_gdstk/gdstk/clipperlib/*.cpp 
${ROOT_DIR}/*.cpp ${SRC_DIR}/mcell_error/*.cpp 
${SRC_DIR}/mcell_command_line/*.cpp 
${SRC_DIR}/mcell_components/*.cpp 
${SRC_DIR}/mcell_fast_henry/*.cpp 
${SRC_DIR}/mcell_global/*.cpp 
${SRC_DIR}/mcell_library/*.cpp 
${SRC_DIR}/mcell_technology/*.cpp )

# ------------------------------------------------------
# top
# ------------------------------------------------------
project(mcell VERSION 1.00)
add_executable(mcell ${ROOT_DIR}/mcell.cpp ${APP_SOURCES})



# ------------------------------------------------------
# header files
# ------------------------------------------------------
target_include_directories(mcell PUBLIC
${ROOT_DIR}
${SRC_DIR}/ext_thread_pool
${SRC_DIR}/ext_gdstk
${SRC_DIR}/ext_gdstk/gdstk
${SRC_DIR}/ext_gdstk/gdstk/libqhull_r
${SRC_DIR}/ext_gdstk/gdstk/clipperlib
${SRC_DIR}/ext_zlib
${SRC_DIR}/ext_poly2tri
${SRC_DIR}/ext_mkl
${SRC_DIR}/ext_yaml
${SRC_DIR}/mcell_error
${SRC_DIR}/mcell_technology
${SRC_DIR}/mcell_components
${SRC_DIR}/mcell_library
${SRC_DIR}/mcell_command_line
${SRC_DIR}/mcell_fast_henry
${SRC_DIR}/mcell_global
${SRC_DIR}/mlic_license
) 

# ------------------------------------------------------
# find libraries
# ------------------------------------------------------
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})

# ------------------------------------------------------
# INTEL MKL
# ------------------------------------------------------
# https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl-link-line-advisor.html
# ------------------------------------------------------
#===============================================================================
# Input parameters
#=================
#-------------
# Main options
#-------------
# MKL_ROOT: oneMKL root directory (May be required for non-standard install locations. Optional otherwise.)
#    Default: use location from MKLROOT environment variable or <Full path to this file>/../../../ if MKLROOT is not defined
# MKL_ARCH
#    Values:  ia32 intel64
#    Default: intel64
# MKL_LINK
#    Values:  static, dynamic, sdl
#    Default: dynamic
#       Exceptions:- DPC++ doesn't support sdl
# MKL_THREADING
#    Values:  sequential,
#             intel_thread (Intel OpenMP),
#             gnu_thread (GNU OpenMP),
#             pgi_thread (PGI OpenMP),
#             tbb_thread
#    Default: intel_thread
#       Exceptions:- DPC++ defaults to tbb, PGI compiler on Windows defaults to pgi_thread
# MKL_INTERFACE (for MKL_ARCH=intel64 only)
#    Values:  lp64, ilp64
#       GNU or INTEL interface will be selected based on Compiler.
#    Default: ilp64
# MKL_MPI
#    Values:  intelmpi, mpich, openmpi, msmpi, mshpc
#    Default: intelmpi
#-----------------------------------
# Special options (OFF by default)
#-----------------------------------
# ENABLE_BLAS95:      Enables BLAS Fortran95 API
# ENABLE_LAPACK95:    Enables LAPACK Fortran95 API
# ENABLE_BLACS:       Enables cluster BLAS library
# ENABLE_CDFT:        Enables cluster DFT library
# ENABLE_CPARDISO:    Enables cluster PARDISO functionality
# ENABLE_SCALAPACK:   Enables cluster LAPACK library
# ENABLE_OMP_OFFLOAD: Enables OpenMP Offload functionality
#-----------------------------------
set(MKLROOT /opt/intel/oneapi/mkl/latest)
set(MKL_ARCH intel64)
set(MKL_LINK static)
set(MKL_THREADING sequential)
#set(MKL_THREADING tbb_thread)
set(MKL_NEEDINCLUDES true)
set(MKL_INTERFACE ilp64)
#set(ENABLE_SCALAPACK true)

find_package(MKL CONFIG REQUIRED PATH /opt/intel/oneapi/mkl/latest)
target_compile_options(mcell PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_COMPILE_OPTIONS>)
target_include_directories(mcell PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_INCLUDE_DIRECTORIES>)


# ------------------------------------------------------
# link libraries
# ------------------------------------------------------

target_link_libraries(mcell PUBLIC
#${SRC_DIR}/ext_yaml/libyaml-cpp.a
#Threads::Threads
$<LINK_ONLY:MKL::MKL>
${LIB_DIR}/libyaml-cpp.a
)

target_link_options(mcell PUBLIC -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive )






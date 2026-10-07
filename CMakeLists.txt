cmake_minimum_required(VERSION 3.20)
project(hunspell VERSION 1.7.5 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

file(GLOB HUNSPELL_SOURCES CONFIGURE_DEPENDS src/hunspell/*.cxx)
file(GLOB HUNSPELL_HEADERS CONFIGURE_DEPENDS src/hunspell/*.hxx)

add_library(hunspell ${HUNSPELL_SOURCES} ${HUNSPELL_HEADERS})
target_compile_features(hunspell PUBLIC cxx_std_14)
target_include_directories(hunspell PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/hunspell>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/hunspell>)
target_compile_definitions(hunspell PRIVATE BUILDING_LIBHUNSPELL)
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(hunspell PUBLIC HUNSPELL_STATIC)
endif()

# Only the symbols marked LIBHUNSPELL_DLL_EXPORTED are public, as with autotools.
set_target_properties(hunspell PROPERTIES
  OUTPUT_NAME hunspell-1.7
  # libtool -version-info 2:1:2 (see src/hunspell/Makefile.am)
  VERSION 0.2.1
  SOVERSION 0
  CXX_EXTENSIONS OFF
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)

install(TARGETS hunspell EXPORT HunspellTargets)
install(FILES
  src/hunspell/hunspell.hxx
  src/hunspell/hunspell.h
  src/hunspell/hunvisapi.h
  src/hunspell/hunversion.h
  src/hunspell/w_char.hxx
  src/hunspell/atypes.hxx
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hunspell)

# find_package(Hunspell) support, generated here rather than from a template.
set(HUNSPELL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Hunspell)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/HunspellConfig.cmake
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/HunspellTargets.cmake\")\n")
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/HunspellConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)
install(EXPORT HunspellTargets NAMESPACE Hunspell::
  DESTINATION ${HUNSPELL_CMAKE_DIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/HunspellConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/HunspellConfigVersion.cmake
  DESTINATION ${HUNSPELL_CMAKE_DIR})

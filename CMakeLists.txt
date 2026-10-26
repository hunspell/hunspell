cmake_minimum_required(VERSION 3.15)
project(hunspell VERSION 1.7.6 LANGUAGES CXX)

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

set_target_properties(hunspell PROPERTIES
  OUTPUT_NAME hunspell-1.7
  VERSION 0.2.2
  SOVERSION 0
  CXX_EXTENSIONS OFF
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)

install(TARGETS hunspell EXPORT hunspell-targets)
install(FILES
  src/hunspell/hunspell.hxx
  src/hunspell/hunspell.h
  src/hunspell/hunvisapi.h
  src/hunspell/hunversion.h
  src/hunspell/w_char.hxx
  src/hunspell/atypes.hxx
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hunspell)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/hunspell-config.cmake
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/hunspell-targets.cmake\")\n")
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/hunspell-config-version.cmake
  COMPATIBILITY SameMajorVersion)
install(EXPORT hunspell-targets NAMESPACE hunspell::
  FILE hunspell-targets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hunspell)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/hunspell-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/hunspell-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hunspell)

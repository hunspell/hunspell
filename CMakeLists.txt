# ---------------------------------------------------------------------------- #
cmake_minimum_required(VERSION 3.13)
project(hunspell VERSION 1.7.0)

# ~~~ Options
option(HUNSPELL_BUILD_TOOLS       "Build Hunspell tools and parser."  ON)
option(HUNSPELL_BUILD_STATIC_LIB  "Build static hunspell library."    ON)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Setup paths and vars
set(HUNSPELL_NAMESPACE             "${PROJECT_NAME}::")
set(HUNSPELL_SRC_DIR               "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(HUNSPELL_INCLUDE_INSTALL_DIR   "include")
set(HUNSPELL_LIBRARY_INSTALL_DIR   "lib")
set(HUNSPELL_RUNTIME_INSTALL_DIR   "bin")
set(HUNSPELL_INSTALL_TARGETS       ""  CACHE INTERNAL  "")
set(HUNSPELL_EXTRA_EXPORT_TARGETS  ""  CACHE INTERNAL  "")
set(VERSION                        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Set RPATH for installed executables
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${HUNSPELL_LIBRARY_INSTALL_DIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Compiler options and definitions
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-fvisibility=hidden" HAVE_VISIBILITY)
if(NOT HAVE_VISIBILITY)
    set(HAVE_VISIBILITY  0)
endif()

if(MSVC)
    # Group targets in folders
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    add_compile_definitions(
        "_CRT_SECURE_NO_WARNINGS"
    )
    # Set warning level; disable specific warnings
    add_compile_options(
        "-W4"
        "-wd4251"
        "-wd4267"
        "-wd4706"
    )
endif()
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Macro for adding install targets
macro(add_install_target arg1)
    set(HUNSPELL_INSTALL_TARGETS ${HUNSPELL_INSTALL_TARGETS} ${arg1} CACHE INTERNAL "")
endmacro()

# ~~~ Macro for adding extra export targets in build-tree
macro(add_extra_export_target arg1)
    set(HUNSPELL_EXTRA_EXPORT_TARGETS ${HUNSPELL_EXTRA_EXPORT_TARGETS} ${arg1} CACHE INTERNAL "")
endmacro()

# ~~~ Copy dll, don't use GENERATOR_IS_MULTI_CONFIG as it doesn't get set for some reason
function(copy_dll_if_changed arg1 arg2)
    set(LIB_DIR   "$<TARGET_PROPERTY:${arg1},BINARY_DIR>")
    set(LIB_FILE  "$<TARGET_FILE_NAME:${arg1}>")
    add_custom_command(TARGET ${arg2} POST_BUILD
        COMMAND  ${CMAKE_COMMAND} -E copy_if_different 
            ${LIB_DIR}/$<$<BOOL:CMAKE_CONFIGURATION_TYPES>:$<CONFIG>/>${LIB_FILE}
            $<TARGET_FILE_DIR:${arg2}>
    )
endfunction()

# Add source subdirectory
add_subdirectory(src)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Configure install and export
include(CMakePackageConfigHelpers)

# ~~~ Set paths
set(HUNSPELL_CONFIG_INSTALL_DIR      "${HUNSPELL_LIBRARY_INSTALL_DIR}/cmake/${PROJECT_NAME}")
set(HUNSPELL_PKG_CONFIG_INSTALL_DIR  "${HUNSPELL_LIBRARY_INSTALL_DIR}/pkgconfig")
set(HUNSPELL_TARGETS_EXPORT_NAME     "${PROJECT_NAME}Targets")
set(HUNSPELL_VERSION_CONFIG_FILE     "${PROJECT_NAME}ConfigVersion.cmake")
set(HUNSPELL_PROJECT_CONFIG_FILE     "${PROJECT_NAME}Config.cmake")

# ~~~ Install targets
install(
    TARGETS              ${HUNSPELL_INSTALL_TARGETS}
    EXPORT               ${HUNSPELL_TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION  ${HUNSPELL_RUNTIME_INSTALL_DIR}
    LIBRARY DESTINATION  ${HUNSPELL_LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION  ${HUNSPELL_LIBRARY_INSTALL_DIR}
)

# ~~~ Write config version file
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${HUNSPELL_VERSION_CONFIG_FILE}
    COMPATIBILITY  SameMinorVersion
)

# Install project version file
install(
    FILES        ${CMAKE_CURRENT_BINARY_DIR}/${HUNSPELL_VERSION_CONFIG_FILE}
    DESTINATION  ${HUNSPELL_CONFIG_INSTALL_DIR}
)

# ~~~ Generate and install project config file
install(
    EXPORT       ${HUNSPELL_TARGETS_EXPORT_NAME}
    NAMESPACE    ${HUNSPELL_NAMESPACE}
    DESTINATION  ${HUNSPELL_CONFIG_INSTALL_DIR}
    FILE         ${HUNSPELL_PROJECT_CONFIG_FILE}
)

# Export project config file separately in build tree
export(
    TARGETS
        ${HUNSPELL_INSTALL_TARGETS}
        ${HUNSPELL_EXTRA_EXPORT_TARGETS}
    NAMESPACE  ${HUNSPELL_NAMESPACE}
    FILE       ${HUNSPELL_PROJECT_CONFIG_FILE}
)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Write pkg-config pc file
set(PKG_PC_FILE_IN   ${CMAKE_CURRENT_BINARY_DIR}/hunspell.pc.in)
set(PKG_PC_FILE_OUT  ${CMAKE_CURRENT_BINARY_DIR}/hunspell.pc)

if (UNIX)
    file(WRITE ${PKG_PC_FILE_IN} [=[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${prefix}/@HUNSPELL_LIBRARY_INSTALL_DIR@
includedir=${prefix}/@HUNSPELL_INCLUDE_INSTALL_DIR@

Name: hunspell
Description: Hunspell spellchecking library
Version: @VERSION@
Libs: -L${libdir} -l@HUNSPELL_LIB_OUTPUT_NAME@
Cflags: -I${includedir}/@HUNSPELL_LIB_INCLUDE_PREFIX@
]=])

    configure_file(${PKG_PC_FILE_IN} ${PKG_PC_FILE_OUT} @ONLY)
    install(
        FILES        ${PKG_PC_FILE_OUT}
        DESTINATION  ${HUNSPELL_PKG_CONFIG_INSTALL_DIR}
    )
endif()
# ---------------------------------------------------------------------------- #

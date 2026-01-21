# ---------------------------------------------------------------------------- #
# ~~~ Set target names
set(_ANALYZE_OUTPUT_NAME   "analyze")
set(_CHMORPH_OUTPUT_NAME   "chmorph")
set(_EXAMPLE_OUTPUT_NAME   "example")
set(_HUNSPELL_OUTPUT_NAME  "hunspell")
set(_HUNZIP_OUTPUT_NAME    "hunzip")
set(_HZIP_OUTPUT_NAME      "hzip")
set(_MUNCH_OUTPUT_NAME     "munch")
set(_UNMUNCH_OUTPUT_NAME   "unmunch")

set(HUNSPELL_TOOLS_TARGET_PREFIX    "tools")
set(HUNSPELL_TOOLS_ANALYZE_TARGET   "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_ANALYZE_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_CHMORPH_TARGET   "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_CHMORPH_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_EXAMPLE_TARGET   "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_EXAMPLE_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_HUNSPELL_TARGET  "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_HUNSPELL_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_HUNZIP_TARGET    "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_HUNZIP_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_HZIP_TARGET      "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_HZIP_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_MUNCH_TARGET     "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_MUNCH_OUTPUT_NAME}")
set(HUNSPELL_TOOLS_UNMUNCH_TARGET   "${HUNSPELL_TOOLS_TARGET_PREFIX}_${_UNMUNCH_OUTPUT_NAME}")

set(HUNSPELL_TOOLS_OUTPUT_PREFIX    ""  CACHE STRING "Prefix added to binaries output filename.")
mark_as_advanced(HUNSPELL_TOOLS_OUTPUT_PREFIX)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Hunspell target

# ~~ Add executable
add_executable(${HUNSPELL_TOOLS_HUNSPELL_TARGET} hunspell.cxx)

# ~~~ Check for headers
include(CheckIncludeFile)
check_include_file(locale.h     _HAVE_LOCALE_H)
check_include_file(langinfo.h   _HAVE_LANGINFO_H)

# ~~~ Find libraries  

# ~ Iconv
if(UNIX)
    find_package(Iconv REQUIRED)
endif()

# ~ Curses
if(UNIX)
    option(HUNSPELL_TOOLS_USE_CURSES  "Locate and link curses library."  OFF)

    if(HUNSPELL_TOOLS_USE_CURSES)
        set(CURSES_NEED_WIDE ON)
        find_package(Curses)

        if(NOT CURSES_FOUND)
            message(FATAL_ERROR  "Could not find curses library. Set paths manually.")
        endif()
    endif()
endif()


if(CURSES_FOUND)
    set(_HAVE_CURSES_H    OFF)
    set(_HAVE_NCURSESW_H  OFF)
    set(_CURSES_INC_DIR   "")
    set(_CURSES_LIB       "")

    get_filename_component(_CURSES_LIB_NAME  ${CURSES_NCURSES_LIBRARY}  NAME)

    # libcurses just points to libncurses in most cases
    if(EXISTS "${CURSES_INCLUDE_PATH}/curses.h")
        set(_HAVE_CURSES_H   ON)
        set(_CURSES_INC_DIR  "${CURSES_INCLUDE_PATH}")
        set(_CURSES_LIB      "${CURSES_CURSES_LIBRARY}")
    endif()
    if(EXISTS "${CURSES_INCLUDE_PATH}/ncurses/curses.h")
        set(_HAVE_CURSES_H   ON)
        set(_CURSES_INC_DIR  "${CURSES_INCLUDE_PATH}/ncurses")
        set(_CURSES_LIB      "${CURSES_NCURSES_LIBRARY}")
    endif()
    if(EXISTS "${CURSES_INCLUDE_PATH}/ncursesw/curses.h" AND ${_CURSES_LIB_NAME} MATCHES "w")
        set(_HAVE_NCURSESW_H  ON)
        set(_CURSES_INC_DIR   "${CURSES_INCLUDE_PATH}")
        set(_CURSES_LIB       "${CURSES_NCURSES_LIBRARY}")
    endif()
endif()

# ~ Readline
if(UNIX)
    option(HUNSPELL_TOOLS_USE_READLINE  "Locate and link readline library."  OFF)
endif()

if(HUNSPELL_TOOLS_USE_READLINE)
    find_path(READLINE_INCLUDE_PATH
        NAMES  readline/readline.h
        DOC    "Readline include directory."
    )
    find_library(READLINE_LIBRARY
        NAMES  readline
        DOC    "Readline library."
    )
    mark_as_advanced(
        READLINE_INCLUDE_PATH
        READLINE_LIBRARY
    )
    find_package_handle_standard_args(Readline
        REQUIRED_VARS  
            READLINE_LIBRARY
            READLINE_INCLUDE_PATH
    )
    if(NOT Readline_FOUND)
        message(ERROR  "Could not find readline library. Set paths manually.")
    endif()
endif()

# ~~~ Define VERSION in a dummy config.h header file, other defs are added 
#     through target_compile_definitions()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h.in [=[#define VERSION "@VERSION@"]=])
configure_file(${CMAKE_CURRENT_BINARY_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

# ~~~ Update target
target_compile_definitions(${HUNSPELL_TOOLS_HUNSPELL_TARGET}
    PRIVATE
        $<$<BOOL:${_HAVE_LOCALE_H}>:HAVE_LOCALE_H>
        $<$<BOOL:${_HAVE_LANGINFO_H}>:HAVE_LANGINFO_H>
        $<$<BOOL:${Iconv_FOUND}>:HAVE_ICONV>
        $<$<BOOL:${Iconv_FOUND}>:ICONV_CONST=>
        $<$<BOOL:${_HAVE_CURSES_H}>:HAVE_CURSES_H>
        $<$<BOOL:${_HAVE_NCURSESW_H}>:HAVE_NCURSESW_CURSES_H>
        $<$<BOOL:${Readline_FOUND}>:HAVE_READLINE>
)
target_include_directories(${HUNSPELL_TOOLS_HUNSPELL_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<$<BOOL:${_CURSES_INC_DIR}>:${_CURSES_INC_DIR}>
        $<$<BOOL:${Readline_FOUND}>:${READLINE_INCLUDE_PATH}>
)
target_link_libraries(${HUNSPELL_TOOLS_HUNSPELL_TARGET}
    PRIVATE
        ${HUNSPELL_PARSERS_LIB_TARGET}
        $<$<BOOL:${Iconv_FOUND}>:Iconv::Iconv>
        $<$<BOOL:${_CURSES_LIB}>:${_CURSES_LIB}>
        $<$<BOOL:${Readline_FOUND}>:${READLINE_LIBRARY}>
)

set_target_properties(${HUNSPELL_TOOLS_HUNSPELL_TARGET} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_HUNSPELL_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Analyze
add_executable(${HUNSPELL_TOOLS_ANALYZE_TARGET} analyze.cxx)

target_link_libraries(${HUNSPELL_TOOLS_ANALYZE_TARGET}
    PRIVATE  ${HUNSPELL_LIB_TARGET_SHARED}
)
set_target_properties(${HUNSPELL_TOOLS_ANALYZE_TARGET} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_ANALYZE_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
)

# ~~~ Chmorph target
add_executable(${HUNSPELL_TOOLS_CHMORPH_TARGET} chmorph.cxx)

target_link_libraries(${HUNSPELL_TOOLS_CHMORPH_TARGET}
    PRIVATE  ${HUNSPELL_PARSERS_LIB_TARGET}
)
set_target_properties(${HUNSPELL_TOOLS_CHMORPH_TARGET} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_CHMORPH_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
)

# ~~~ Example target
add_executable(${HUNSPELL_TOOLS_EXAMPLE_TARGET} example.cxx)

target_link_libraries(${HUNSPELL_TOOLS_EXAMPLE_TARGET}
    PRIVATE  ${HUNSPELL_LIB_TARGET_SHARED}
)
set_target_properties(${HUNSPELL_TOOLS_EXAMPLE_TARGET} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_EXAMPLE_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
)

# ~~~ Hunzip target
add_executable(${HUNSPELL_TOOLS_HUNZIP_TARGET} hunzip.cxx)

target_link_libraries(${HUNSPELL_TOOLS_HUNZIP_TARGET}
    PRIVATE  ${HUNSPELL_LIB_TARGET_SHARED}
)
set_target_properties(${HUNSPELL_TOOLS_HUNZIP_TARGET} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_HUNZIP_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
)

if((NOT WIN32) OR MINGW)
    # ~~~ Hzip target
    add_executable(${HUNSPELL_TOOLS_HZIP_TARGET} hzip.cxx)

    set_target_properties(${HUNSPELL_TOOLS_HZIP_TARGET} PROPERTIES
        OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_HZIP_OUTPUT_NAME}
        FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
    )

    # ~~~ Munch target
    add_executable(${HUNSPELL_TOOLS_MUNCH_TARGET} munch.cxx)

    target_include_directories(${HUNSPELL_TOOLS_MUNCH_TARGET}
        PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(${HUNSPELL_TOOLS_MUNCH_TARGET} PROPERTIES
        OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_MUNCH_OUTPUT_NAME}
        FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
    )

    # ~~~ Unmunch target
    add_executable(${HUNSPELL_TOOLS_UNMUNCH_TARGET} unmunch.cxx)

    target_include_directories(${HUNSPELL_TOOLS_UNMUNCH_TARGET}
        PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(${HUNSPELL_TOOLS_UNMUNCH_TARGET} PROPERTIES
        OUTPUT_NAME  ${HUNSPELL_TOOLS_OUTPUT_PREFIX}${_UNMUNCH_OUTPUT_NAME}
        FOLDER       ${HUNSPELL_TOOLS_TARGET_PREFIX}
    )
endif()
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Copy dll to binary dir
if(WIN32 OR MINGW)
    # In case targets are built separately
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_ANALYZE_TARGET})
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_CHMORPH_TARGET})
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_EXAMPLE_TARGET})
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_HUNSPELL_TARGET})
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_HUNZIP_TARGET})
    if(MINGW)
        copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_HZIP_TARGET})
        copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_MUNCH_TARGET})
        copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_TOOLS_UNMUNCH_TARGET})
    endif()
endif()
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Add install targets
add_install_target(${HUNSPELL_TOOLS_ANALYZE_TARGET})
add_install_target(${HUNSPELL_TOOLS_CHMORPH_TARGET})
add_install_target(${HUNSPELL_TOOLS_HUNSPELL_TARGET})
add_install_target(${HUNSPELL_TOOLS_HUNZIP_TARGET})

if((NOT WIN32) OR MINGW)
    add_install_target(${HUNSPELL_TOOLS_HZIP_TARGET})
    add_install_target(${HUNSPELL_TOOLS_MUNCH_TARGET})
    add_install_target(${HUNSPELL_TOOLS_UNMUNCH_TARGET})
endif()
# ---------------------------------------------------------------------------- #
# ---------------------------------------------------------------------------- #
# ~~~ Set target names
set(_LIB_OUTPUT_NAME   "parsers")
set(_TEST_OUTPUT_NAME  "test")

set(HUNSPELL_PARSERS_TARGET_PREFIX  "parsers"                                                 CACHE INTERNAL  "")
set(HUNSPELL_PARSERS_LIB_TARGET     "${HUNSPELL_PARSERS_TARGET_PREFIX}_lib"                   CACHE INTERNAL  "")
set(HUNSPELL_PARSERS_TEST_TARGET    "${HUNSPELL_PARSERS_TARGET_PREFIX}_${_TEST_OUTPUT_NAME}"  CACHE INTERNAL  "")
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Parsers lib target
add_library(${HUNSPELL_PARSERS_LIB_TARGET} STATIC)

target_sources(${HUNSPELL_PARSERS_LIB_TARGET}
    PRIVATE
        firstparser.cxx
        htmlparser.cxx
        latexparser.cxx
        manparser.cxx
        odfparser.cxx
        textparser.cxx
        xmlparser.cxx
)
target_include_directories(${HUNSPELL_PARSERS_LIB_TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${HUNSPELL_SRC_DIR}>
)
target_link_libraries(${HUNSPELL_PARSERS_LIB_TARGET}
    PUBLIC  ${HUNSPELL_LIB_TARGET_SHARED}
)
set_target_properties(${HUNSPELL_PARSERS_LIB_TARGET} PROPERTIES
    OUTPUT_NAME  ${_LIB_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_PARSERS_TARGET_PREFIX}
)

# Create an alias
add_library(${HUNSPELL_NAMESPACE}${HUNSPELL_PARSERS_LIB_TARGET} ALIAS ${HUNSPELL_PARSERS_LIB_TARGET})
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Parsers test executable
add_executable(${HUNSPELL_PARSERS_TEST_TARGET} testparser.cxx)

target_include_directories(${HUNSPELL_PARSERS_TEST_TARGET}
    PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${HUNSPELL_PARSERS_TEST_TARGET}
    PRIVATE
        ${HUNSPELL_PARSERS_LIB_TARGET}
        ${HUNSPELL_LIB_TARGET_SHARED}
)

set_target_properties(${HUNSPELL_PARSERS_TEST_TARGET} PROPERTIES
    OUTPUT_NAME  ${_TEST_OUTPUT_NAME}
    FOLDER       ${HUNSPELL_PARSERS_TARGET_PREFIX}
)

if(WIN32 OR MINGW)
    copy_dll_if_changed(${HUNSPELL_LIB_TARGET_SHARED} ${HUNSPELL_PARSERS_TEST_TARGET})
endif()
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Add extra export targets
add_extra_export_target(${HUNSPELL_PARSERS_LIB_TARGET})
add_extra_export_target(${HUNSPELL_PARSERS_TEST_TARGET})
# ---------------------------------------------------------------------------- #
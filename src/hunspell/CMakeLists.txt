# ---------------------------------------------------------------------------- #
# ~~~ Set target names and sources

set(HUNSPELL_LIB_TARGET_PREFIX      "hunspell"                                  CACHE INTERNAL  "")
set(HUNSPELL_LIB_TARGET_SHARED      "${HUNSPELL_LIB_TARGET_PREFIX}_lib"         CACHE INTERNAL  "")
set(HUNSPELL_LIB_TARGET_STATIC      "${HUNSPELL_LIB_TARGET_PREFIX}_lib_static"  CACHE INTERNAL  "")
set(HUNSPELL_LIB_TARGET_OBJ         "${HUNSPELL_LIB_TARGET_PREFIX}_lib_obj"     CACHE INTERNAL  "")
set(HUNSPELL_LIB_INCLUDE_PREFIX     "${HUNSPELL_LIB_TARGET_PREFIX}"             CACHE INTERNAL  "")

set(HUNSPELL_LIB_OUTPUT_NAME  "${HUNSPELL_LIB_TARGET_PREFIX}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}" CACHE INTERNAL  "")
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Sources
list(APPEND HUNSPELL_LIB_SOURCES
    affentry.cxx
    affixmgr.cxx
    csutil.cxx
    filemgr.cxx
    hashmgr.cxx
    hunspell.cxx
    hunzip.cxx
    phonet.cxx
    replist.cxx
    suggestmgr.cxx
)

# ~~~ Include directories
list(APPEND HUNSPELL_LIB_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${HUNSPELL_INCLUDE_INSTALL_DIR}>
)

# ~~~ Configure header file
configure_file(hunvisapi.h.in hunvisapi.h)

# ~~~ Public headers
list(APPEND HUNSPELL_LIB_PUBLIC_HEADERS
    atypes.hxx
    hunspell.h
    hunspell.hxx
    w_char.hxx
    ${CMAKE_CURRENT_BINARY_DIR}/hunvisapi.h
)

# Install public headers
install(
    FILES        ${HUNSPELL_LIB_PUBLIC_HEADERS}
    DESTINATION  ${HUNSPELL_INCLUDE_INSTALL_DIR}/${HUNSPELL_LIB_INCLUDE_PREFIX}
)
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Object library shared by both shared and static targets
add_library(${HUNSPELL_LIB_TARGET_OBJ} OBJECT)

target_sources(${HUNSPELL_LIB_TARGET_OBJ}
    PRIVATE  ${HUNSPELL_LIB_SOURCES}
)
target_include_directories(${HUNSPELL_LIB_TARGET_OBJ}
    PUBLIC  ${HUNSPELL_LIB_INCLUDE_DIRECTORIES}
)
target_compile_definitions(${HUNSPELL_LIB_TARGET_OBJ}
    PRIVATE  "BUILDING_LIBHUNSPELL"
)

# Shared targets have POSITION_INDEPENDENT_CODE property on by default, object
# libraries do not, unless CMAKE_POSITION_INDEPENDENT_CODE is set to ON.
set_target_properties(${HUNSPELL_LIB_TARGET_OBJ} PROPERTIES
    POSITION_INDEPENDENT_CODE  ON
    CXX_VISIBILITY_PRESET      hidden
    FOLDER                     ${HUNSPELL_LIB_TARGET_PREFIX}
)

# Add install target
add_install_target(${HUNSPELL_LIB_TARGET_OBJ})
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Shared library target
add_library(${HUNSPELL_LIB_TARGET_SHARED} SHARED)

target_link_libraries(${HUNSPELL_LIB_TARGET_SHARED}
    PRIVATE
        ${HUNSPELL_LIB_TARGET_OBJ}
        # Cmake passes linker flags through compiler
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--no-undefined>
        $<$<CXX_COMPILER_ID:Clang>:-Wl,-no-undefined,error>
)
target_include_directories(${HUNSPELL_LIB_TARGET_SHARED}
    INTERFACE  ${HUNSPELL_LIB_INCLUDE_DIRECTORIES}
)

# Properties
set_target_properties(${HUNSPELL_LIB_TARGET_SHARED} PROPERTIES
    OUTPUT_NAME  ${HUNSPELL_LIB_OUTPUT_NAME}
    VERSION      ${PROJECT_VERSION}
    SOVERSION    ${PROJECT_VERSION_MAJOR}
    FOLDER       ${HUNSPELL_LIB_TARGET_PREFIX}
)

# Create alias
add_library(${HUNSPELL_NAMESPACE}${HUNSPELL_LIB_TARGET_SHARED} ALIAS ${HUNSPELL_LIB_TARGET_SHARED})

# Add install target
add_install_target(${HUNSPELL_LIB_TARGET_SHARED})
# ---------------------------------------------------------------------------- #


# ---------------------------------------------------------------------------- #
# ~~~ Static library target
option(HUNSPELL_LIB_STATIC_PIC     "Build static library with position independent code."  ON)
option(HUNSPELL_LIB_STATIC_HIDDEN  "Set the visibility of all symbols to hidden."          OFF)
mark_as_advanced(
    HUNSPELL_LIB_STATIC_PIC
    HUNSPELL_LIB_STATIC_HIDDEN
)

if(HUNSPELL_BUILD_STATIC_LIB)
    add_library(${HUNSPELL_LIB_TARGET_STATIC} STATIC)
    
    # Archive the already created object files
    target_link_libraries(${HUNSPELL_LIB_TARGET_STATIC}
        PRIVATE  ${HUNSPELL_LIB_TARGET_OBJ}
    )
    target_include_directories(${HUNSPELL_LIB_TARGET_STATIC}
        INTERFACE  ${HUNSPELL_LIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(${HUNSPELL_LIB_TARGET_STATIC}
        INTERFACE  "HUNSPELL_STATIC"
    )

    # ~~~ Recompile source files if flags set
    if((NOT HUNSPELL_LIB_STATIC_PIC) OR HUNSPELL_LIB_STATIC_HIDDEN)
        set_target_properties(${HUNSPELL_LIB_TARGET_STATIC} PROPERTIES
            SOURCES  "${HUNSPELL_LIB_SOURCES}"
        )
    endif()

    if(NOT HUNSPELL_LIB_STATIC_PIC)
        # Force target PIC flag to OFF in case CMAKE_POSITION_INDEPENDENT_CODE is set
        set_target_properties(${HUNSPELL_LIB_TARGET_STATIC} PROPERTIES
            POSITION_INDEPENDENT_CODE  OFF
        )
    endif()

    if(HUNSPELL_LIB_STATIC_HIDDEN)
        # Set the visibility of *all* FUNC symbols
        set_target_properties(${HUNSPELL_LIB_TARGET_STATIC} PROPERTIES
            CXX_VISIBILITY_PRESET  hidden
        )
    endif()

    # ~~~ Output name and version
    set_target_properties(${HUNSPELL_LIB_TARGET_STATIC} PROPERTIES
        OUTPUT_NAME  ${HUNSPELL_LIB_OUTPUT_NAME}$<$<BOOL:${WIN32}>:_static>
        VERSION      ${PROJECT_VERSION}
        SOVERSION    ${PROJECT_VERSION_MAJOR}
        FOLDER       ${HUNSPELL_LIB_TARGET_PREFIX}
    )

    # ~~~ Create alias
    add_library(${HUNSPELL_NAMESPACE}${HUNSPELL_LIB_TARGET_STATIC} ALIAS ${HUNSPELL_LIB_TARGET_STATIC})

    # ~~~ Add install target
    add_install_target(${HUNSPELL_LIB_TARGET_STATIC})
endif()
# ---------------------------------------------------------------------------- #

add_subdirectory(hunspell)

if(HUNSPELL_BUILD_TOOLS)
    add_subdirectory(parsers)
    add_subdirectory(tools)
endif()

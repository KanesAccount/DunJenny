function(add_generator name)
    project(${name})

    set(_Generator_Sources
        ${ARGN}
    )

    source_group("" FILES ${_Generator_Sources})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_Generator_Sources})

    set(_Generator_Type)
    if (WIN32)
        set(_Generator_Type WIN32)
    elseif (APPLE)
        set(_Generator_Type MACOSX_BUNDLE)

        file(GLOB _Generator_Resources "${CMAKE_SOURCE_DIR}/Data/*")
        set_source_files_properties(${_Generator_Resources} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources/Data"
        )
        set(_Generator_Icon "${CMAKE_SOURCE_DIR}/Generator/Common/Support/Icon.icns")
        list(APPEND _Generator_Resources ${_Generator_Icon})
        set_source_files_properties(${_Genertor_Icon} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
    endif()

    add_executable(${name} ${_Generator_Type} ${_Generator_Sources} ${_Generator_Resources})

    target_link_libraries(${name} PRIVATE ImGui NodeEditor Shared Application)
    target_link_libraries(${name} PRIVATE MetaStuff)

    set(_GeneratorBinDir ${CMAKE_BINARY_DIR}/Bin)

    set_target_properties(${name} PROPERTIES
        FOLDER "Generator"
        RUNTIME_OUTPUT_DIRECTORY                "${_GeneratorBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${_GeneratorBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_GeneratorBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${_GeneratorBinDir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${_GeneratorBinDir}"
        DEBUG_POSTFIX                           _d
        RELWITHDEBINGO_POSTFIX                  _rd
        MINSIZEREL_POSTFIX                      _r
        VS_DEBUGGER_WORKING_DIRECTORY           ${_GeneratorBinDir}
        MACOSX_BUNDLE_INFO_PLIST                "${CMAKE_SOURCE_DIR}/Generator/Common/Support/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME               "${PACKAGE_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER            "com.sandbox.collisions"
        MACOSX_BUNDLE_LONG_VERSION_STRING       "${PACKAGE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING      "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}"
        MACOSX_BUNDLE_ICON_FILE                 Icon.icns
    )

    add_custom_command(
        TARGET ${name}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ARGS ${_GeneratorBinDir}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ARGS ${CMAKE_SOURCE_DIR}/Data ${_GeneratorBinDir}/Data
    )

endfunction()

add_subdirectory(Common/Shared)
add_subdirectory(Common/Application)

add_subdirectory(DungGenerator)

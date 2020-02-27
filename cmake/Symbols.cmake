set(sym_ext .debug)

function(target_separate_symbols _target)
    if(NOT CMAKE_OBJCOPY)
        message(FATAL_ERROR "objcopy not found")
    endif()

    set(options COMPRESS)
    set(one_value_args "")
    set(multi_value_args "")
    cmake_parse_arguments(options "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(options_COMPRESS AND UNIX)
        # We can only compress sections in ELF files
        add_custom_command(TARGET ${_target}
            POST_BUILD
            VERBATIM
            COMMAND ${CMAKE_OBJCOPY} --compress-debug-sections=zlib $<TARGET_FILE:${_target}>
            COMMENT "Compressing debug symbols of ${_target}"
        )
    endif()
    add_custom_command(TARGET ${_target}
        POST_BUILD
        VERBATIM
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:${_target}> $<TARGET_FILE:${_target}>${sym_ext}
        COMMAND ${CMAKE_OBJCOPY} --strip-debug $<TARGET_FILE:${_target}>
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE:${_target}>${sym_ext} $<TARGET_FILE:${_target}>
        COMMENT "Separating debug symbols from ${_target} into ${_target}${sym_ext} file"
    )
endfunction()

function(target_install_symbols _target)
    set(options "")
    set(one_value_args DESTINATION)
    set(multi_value_args "")
    cmake_parse_arguments(options "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT DEFINED options_DESTINATION)
        message(FATAL_ERROR "target_install_symbols not given a DESTINATION <path> argument.")
    endif()

    if(MSVC) # TODO: Or clang-cl/clang on windows?
        install(FILES $<TARGET_PDB_FILE:${_target}>
            DESTINATION ${options_DESTINATION}
            OPTIONAL # Ignore if file does not exists
        )
    else()
        # In order to be effective, this requires a prior call to
        # target_separate_symbols on given target. Otherwise, it's a no-op.
        install(FILES $<TARGET_FILE:${_target}>${sym_ext}
            DESTINATION ${options_DESTINATION}
            OPTIONAL
        )
    endif()
endfunction()

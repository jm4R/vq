include_guard()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(compile_options
    -Wall
    -pedantic
    -Wextra
    -Wno-unused-parameter
    -Wno-unknown-pragmas
)

macro(__list_to_string _list)
    string(REPLACE ";" " " ${_list} "${${_list}}")
endmacro()

__list_to_string(compile_definitions)
__list_to_string(compile_options)

foreach(lang IN ITEMS C CXX)
    set(CMAKE_${lang}_FLAGS "${compile_definitions} ${compile_options}"
        CACHE STRING "Flags used by the ${lang} compiler during all build types."
    )
    set(CMAKE_${lang}_FLAGS_DEBUG "-gsplit-dwarf"
        CACHE STRING "Flags used by the ${lang} compiler during DEBUG builds."
    )
endforeach()

foreach(mod IN ITEMS EXE MODULE SHARED)
    set(CMAKE_${mod}_LINKER_FLAGS "-fuse-ld=gold -Wl,-z,defs -Wl,--as-needed"
        CACHE STRING "Flags used by the linker during all build types."
    )
    set(CMAKE_${mod}_LINKER_FLAGS_DEBUG "-Wl,--gdb-index"
        CACHE STRING "Flags used by the linker during all build types."
    )
endforeach()

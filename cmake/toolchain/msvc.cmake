include_guard()

set(Boost_USE_STATIC_LIBS ON)
list(APPEND CMAKE_PREFIX_PATH $ENV{NU_SDK}/boost/1.68.0)

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    list(APPEND CMAKE_PREFIX_PATH $ENV{NU_SDK}/Qt/Qt5.13.1/5.13.1/msvc2017_64)
    list(APPEND CMAKE_PREFIX_PATH D:/opt/Qt_VS/5.11.3/msvc2017_64)
else()
    list(APPEND CMAKE_PREFIX_PATH $ENV{NU_SDK}/Qt/Qt5.13.1/5.13.1/msvc2017)
    list(APPEND CMAKE_PREFIX_PATH D:/opt/Qt_VS/5.12.6/msvc2017)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(compile_definitions
    /DWIN32_LEAN_AND_MEAN
    /D_WIN32_WINNT=0x0601
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NON_CONFORMING_SWPRINTFS
    /D_SCL_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_DEPRECATE
    /D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
)

set(compile_options
    /W4
    /wd4100              # unreferenced formal parameter
    /wd4458              # declaration of 'identifier' hides class member
    # Only temporary!
    /wd4244              # 'conversion' conversion from 'type1' to 'type2', possible loss of data
    /wd4245              # 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
    #/permissive- # WinSDK must be at least 10.0.14393.0
    /Zc:inline           # Remove unreferenced function or data if it is COMDAT or has internal linkage only
    /Zc:referenceBinding # A UDT temporary will not bind to an non-const lvalue reference
    /Zc:rvalueCast       # Enforce Standard C++ explicit type conversion rules
    /Zc:strictStrings    # Disable string-literal to char* or wchar_t* conversion
    /Zc:throwingNew      # Assume operator new throws on failure.
)

macro(__list_to_string _list)
    string(REPLACE ";" " " ${_list} "${${_list}}")
endmacro()

__list_to_string(compile_definitions)
__list_to_string(compile_options)

set(CMAKE_C_FLAGS "${compile_definitions} ${compile_options}"
    CACHE STRING "Flags used by the C compiler during all build types."
)
set(CMAKE_CXX_FLAGS "/GR /EHsc ${compile_definitions} ${compile_options}"
    CACHE STRING "Flags used by the CXX compiler during all build types."
)

foreach(lang IN ITEMS C CXX)
    # /Gy - Allows the compiler to package individual functions in the form of packaged functions.
    # /Gw - Package global data in COMDAT sections for optimization.
    # /Oi - Replaces some function calls with intrinsic or otherwise special forms of the function that help your application run faster.
    # /JMC - Just My Code
    # NOTE: /Ob2 is in effect when /O1 or /O2 is used
    set(CMAKE_${lang}_FLAGS_DEBUG "/MDd /JMC /Zi /Od"
        CACHE STRING "Flags used by the ${lang} compiler during DEBUG builds."
    )
    set(CMAKE_${lang}_FLAGS_MINSIZEREL "/MD /O1 /DNDEBUG /Gw /Gy"
        CACHE STRING "Flags used by the ${lang} compiler during MINSIZEREL builds."
    )
    set(CMAKE_${lang}_FLAGS_RELEASE "/MD /O2 /Oi /DNDEBUG /Gw /Gy"
        CACHE STRING "Flags used by the ${lang} compiler during RELEASE builds."
    )
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Oi /DNDEBUG /Gw /Gy"
        CACHE STRING "Flags used by the ${lang} compiler during RELWITHDEBINFO builds."
    )
endforeach()

foreach(mod IN ITEMS EXE MODULE SHARED)
    set(CMAKE_${mod}_LINKER_FLAGS_DEBUG "/DEBUG:FASTLINK /INCREMENTAL"
        CACHE STRING "Flags used by the linker during DEBUG builds."
    )
    # NOTE: By default, /OPT:REF is enabled by the linker unless /OPT:NOREF or /DEBUG is specified. Similarly for /OPT:ICF
    set(CMAKE_${mod}_LINKER_FLAGS_MINSIZEREL "/INCREMENTAL:NO"
        CACHE STRING "Flags used by the linker during MINSIZEREL builds."
    )
    set(CMAKE_${mod}_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO"
        CACHE STRING "Flags used by the linker during RELEASE builds."
    )
    set(CMAKE_${mod}_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF"
        CACHE STRING "Flags used by the linker during RELWITHDEBINFO builds."
    )
endforeach()

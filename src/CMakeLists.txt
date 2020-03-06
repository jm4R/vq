set(CMAKE_CXX_STANDARD 17)

set(GUI_FILES
    gui/Gui.cpp
    gui/Gui.hpp
    gui/ProjectWidget.cpp
    gui/ProjectWidget.hpp
    gui/Window.cpp
    gui/Window.hpp
)

set(MODEL_FILES
    model/OutputGenerator.cpp
    model/OutputGenerator.hpp
    model/ProjectDescription.hpp
    model/TokenizedString.hpp
    model/VcxprojParser.cpp
    model/VcxprojParser.hpp
)

set(QRC_FILES ../res/res.qrc)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_add_resources(RCC_SOURCES ${QRC_FILES})

source_group("gui" FILES ${GUI_FILES})
source_group("model" FILES ${MODEL_FILES})

add_library(vq-lib STATIC
    ${GUI_FILES}
    ${MODEL_FILES}
    ${RCC_SOURCES}
)

target_link_libraries(vq-lib
    PUBLIC Qt5::Widgets
    PUBLIC Qt5::XmlPatterns
)

target_compile_features(vq-lib PRIVATE cxx_std_17)
target_include_directories(vq-lib PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(vq-lib PRIVATE "APP_VERSION=\"${PROJECT_VERSION}\"")

add_executable(vq WIN32
    main.cpp
    main.rc
)

target_link_libraries(vq PUBLIC vq-lib)

set(INSTALL_DIR bin)
if(WIN32)
    include(Windeployqt)
    windeployqt(vq ${INSTALL_DIR})
endif()
install(TARGETS vq RUNTIME DESTINATION ${INSTALL_DIR})

add_executable(R2TConverter EXCLUDE_FROM_ALL
    cmd/main.cpp
    model/GfxConverter.cpp
    model/GfxConverter.hpp
)
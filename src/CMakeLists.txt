# SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
#
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Mqtt Qml Quick)
find_package(MpvQt)

qt_standard_project_setup(REQUIRES 6.5)

set(TARGET_NAME ${PROJECT_NAME}-player)
set(TARGET_URI LeMePlayer)

qt_add_executable(${TARGET_NAME}

# main
    main.cpp
)

include_directories("mqtt")
include_directories("mpv")

qt_add_qml_module(${TARGET_NAME}
    URI ${TARGET_URI}
    QML_FILES
        qml/Main.qml
        qml/Constants.qml
    SOURCES
# mpv
        mpv/declarative-mpv-properties.h
        mpv/declarative-mpv-item.h
        mpv/declarative-mpv-item.cpp
# mqtt
        mqtt/declarative-mqtt-client.h
        mqtt/declarative-mqtt-client.cpp    
)

set(BUILD_CONSTANTS_HEADER_NAME "build-constants")

configure_file(${BUILD_CONSTANTS_HEADER_NAME}.h.in ${BUILD_CONSTANTS_HEADER_NAME}.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET_NAME}
    PRIVATE
        MpvQt::MpvQt
        Qt::Core
        Qt::Gui
        Qt::Mqtt
        Qt::Qml
        Qt::Quick
)

include(GNUInstallDirs)
install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

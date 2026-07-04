cmake_minimum_required(VERSION 3.21)
project(WinPods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Requires the "C++/WinRT" + "Windows 10/11 SDK" components of the VS
# workload (winrt/*.h headers ship inside the SDK since 10.0.19041).
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Widgets Multimedia)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(WinPods WIN32
    src/main.cpp
    src/BleScanner.cpp
    src/BleScanner.h
    src/AirPodsParser.cpp
    src/AirPodsParser.h
    src/AirPodsModel.cpp
    src/AirPodsModel.h
    src/MediaController.cpp
    src/MediaController.h
    resources/app.rc # gives the built .exe its Explorer/taskbar icon
)

qt_add_qml_module(WinPods
    URI WinPods
    VERSION 1.0
    QML_FILES
        qml/Popup.qml
        qml/BatteryGlyph.qml
)

# Embeds the icon into the binary so code can load it via ":/AirPods.ico"
# (tray icon, window icon) without needing the file at runtime.
qt_add_resources(WinPods "app_icon"
    PREFIX "/"
    FILES resources/AirPods.ico
)

# Per-model spin animations shown in the popup instead of drawn icons.
qt_add_resources(WinPods "videos"
    PREFIX "/videos"
    BASE resources
    FILES
        resources/AirPods_1.avi
        resources/AirPods_2.avi
        resources/AirPods_3.avi
)

target_link_libraries(WinPods PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Widgets
    Qt6::Multimedia
    windowsapp # umbrella import lib for all projected WinRT namespaces
)

target_compile_definitions(WinPods PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
)

# ponytail: plain assert-based self-check, not a full test framework -
# just enough to catch a nibble-indexing regression in the BLE parser.
add_executable(test_airpods_parser
    tests/test_airpods_parser.cpp
    src/AirPodsParser.cpp
)
target_link_libraries(test_airpods_parser PRIVATE Qt6::Core)

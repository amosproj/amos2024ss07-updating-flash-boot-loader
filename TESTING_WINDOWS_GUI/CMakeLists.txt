cmake_minimum_required(VERSION 3.5)

project(TESTING_WINDOWS_GUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2019_64/lib/cmake")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        testcases.cpp
        testcases.h
        ../WINDOWS_GUI/GUI_Events/consoleUpdater.h
        ../WINDOWS_GUI/GUI_Events/consoleUpdater.cpp
        ../WINDOWS_GUI/Communication/can_wrapper_event.hpp
        ../WINDOWS_GUI/Communication/Can_Wrapper.cpp
        ../WINDOWS_GUI/Communication/Can_Wrapper.hpp
        ../WINDOWS_GUI/Communication/VirtualDriver.cpp
        ../WINDOWS_GUI/Communication/VirtualDriver.h
        ../WINDOWS_GUI/Communication_Layer/CommInterface.cpp
        ../WINDOWS_GUI/Communication_Layer/CommInterface.h
        ../WINDOWS_GUI/Communication_Layer/Communication.cpp
        ../WINDOWS_GUI/Communication_Layer/Communication.h
        ../WINDOWS_GUI/UDS_Layer/UDS.cpp
        ../WINDOWS_GUI/UDS_Layer/UDS.h
        ../WINDOWS_GUI/UDS_Layer/UDSMsg.cpp
        ../WINDOWS_GUI/UDS_Layer/UDSMsg.h
        ../WINDOWS_GUI/UDS_Layer/UDS_Event.h
        ../WINDOWS_GUI/UDS_Layer/UDS_Event.cpp
        ../WINDOWS_GUI/UDS_Spec/uds_comm_spec.cpp
        ../WINDOWS_GUI/UDS_Spec/uds_comm_spec.h
        )

file(GLOB VECTOR_LIB "C:/Users/Public/Documents/Vector/XL\ Driver\ Library\ */bin")
find_library(VXLAPI vxlapi64.lib PATHS ${VECTOR_LIB})
find_path(VXLAPI_HEADER vxlapi.h PATHS ${VECTOR_LIB})

include_directories(${VXLAPI_HEADER})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TESTING_WINDOWS_GUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TESTING_WINDOWS_GUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TESTING_WINDOWS_GUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TESTING_WINDOWS_GUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TESTING_WINDOWS_GUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${VXLAPI})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TESTING_WINDOWS_GUI)
endif()
set_target_properties(TESTING_WINDOWS_GUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TESTING_WINDOWS_GUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TESTING_WINDOWS_GUI)
endif()

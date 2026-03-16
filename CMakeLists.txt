cmake_minimum_required(VERSION 3.16)

project(DoidsCAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# OpenCascade is connected in the same style as educad:
# CMAKE_PREFIX_PATH or OpenCASCADE_DIR must point to the installed package.
find_package(OpenCASCADE COMPONENTS FoundationClasses REQUIRED)
find_package(OpenCASCADE COMPONENTS ModelingAlgorithms REQUIRED)
find_package(OpenCASCADE COMPONENTS Visualization REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    cadviewport.cpp
    cadviewport.h
    mainwindow.cpp
    mainwindow.h
    operationlistdock.cpp
    operationlistdock.h
    propertyeditordock.cpp
    propertyeditordock.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DoidsCAD
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(DoidsCAD SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(DoidsCAD
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(DoidsCAD PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_link_directories(DoidsCAD PRIVATE ${OpenCASCADE_LIBRARY_DIR})
target_link_libraries(DoidsCAD PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    TKernel
    TKXSBase
    TKMath
    TKGeomBase
    TKG3d
    TKG2d
    TKBRep
    TKTopAlgo
    TKPrim
    TKFillet
    TKOffset
    TKBO
    TKMesh
    TKV3d
    TKOpenGl
    TKService
    TKSTEP
    TKShHealing
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DoidsCAD)
endif()

set_target_properties(DoidsCAD PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DoidsCAD
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DoidsCAD)
endif()

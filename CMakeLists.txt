cmake_minimum_required(VERSION 3.16)
project(ScheduleManagement LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    toptoolbarwidget.cpp
    toptoolbarwidget.h
    schedulelistwidget.cpp
    schedulelistwidget.h
    calendarwidget.cpp
    calendarwidget.h
    scheduleitem.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_standard_project_setup()

    qt_add_executable(ScheduleManagement
        WIN32
        MACOSX_BUNDLE
        ${PROJECT_SOURCES}
    )
else()
    add_executable(ScheduleManagement
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(ScheduleManagement
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
)

install(TARGETS ScheduleManagement
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

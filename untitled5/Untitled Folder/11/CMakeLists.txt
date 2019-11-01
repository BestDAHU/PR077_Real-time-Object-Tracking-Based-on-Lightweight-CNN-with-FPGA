project(tracker)
cmake_minimum_required(VERSION 3.1.0)

find_package(Qt5 COMPONENTS Core Gui Sql Widgets PrintSupport REQUIRED)

find_package(OpenCV COMPONENTS highgui QUIET)
if(NOT(OpenCV_FOUND))
    message(WARNING "OPENCV is disabled or not found, " ${TARGET_NAME} " skipped")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


include_directories(
    ${Qt5Core_INCLUDE_DIRS} 
    ${Qt5Gui_INCLUDE_DIRS} 
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    )
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Qt5Widgets_LIBRARIES})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

qt5_add_resources(qrc_FILES qss.qrc)
add_executable(${PROJECT_NAME} ${SRC_LIST}  ${qrc_FILES})
target_link_libraries(${PROJECT_NAME} ${LIB_LIST} ${OpenCV_LIBS} Qt5::Sql Qt5::Widgets Qt5::PrintSupport)


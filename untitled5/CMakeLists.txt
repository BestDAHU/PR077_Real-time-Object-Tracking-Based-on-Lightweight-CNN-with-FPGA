project(untitled1)
cmake_minimum_required(VERSION 3.1.0)

find_package(Qt5 COMPONENTS Core Gui Sql Widgets PrintSupport REQUIRED)

find_package(OpenCV REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)

set(sources
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/cnn.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/core.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/descriptor.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/detector.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/distance.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/image_reader.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/kuhn_munkres.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/logging.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/pedestrian_tracker_demo.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/tracker.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/utils.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/cnn.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/core.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/detector.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/distance.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/image_reader.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/kuhn_munkres.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/tracker.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/utils.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/tracker_main.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/tracker_main.cpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include/point_trans.hpp
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/src/point_trans.cpp)



include_directories(
    ${Qt5Core_INCLUDE_DIRS} 
    ${Qt5Gui_INCLUDE_DIRS} 
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    /home/dahu/Desktop/pedestrain/pedestrain_tracker/include
    /home/dahu/Desktop/pedestrain/pedestrain_tracker
    )
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Qt5Widgets_LIBRARIES})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

qt5_add_resources(qrc_FILES qss.qrc)
add_executable(${PROJECT_NAME} ${SRC_LIST}  ${qrc_FILES} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES "CMAKE_CXX_FLAGS" "${CMAKE_CXX_FLAGS} -fPIE"
  COMPILE_PDB_NAME ${PROJECT_NAME})
link_directories(${LIB_FOLDER})               # !!!
add_dependencies(${PROJECT_NAME} gflags)      # !!!
# target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
# target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:/home/dahu/Desktop/pedestrain/pedestrain_tracker/include>") # !!!
# target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:/home/dahu/Desktop/pedestrain/pedestrain_tracker>")  # !!!
target_link_libraries(${PROJECT_NAME} IE::ie_cpu_extension ${InferenceEngine_LIBRARIES} ${OpenCV_LIBS} gflags) # link the target file to the library file

target_link_libraries(${PROJECT_NAME} ${LIB_LIST} ${OpenCV_LIBS} Qt5::Sql Qt5::Widgets Qt5::PrintSupport)

if(UNIX)
  target_link_libraries(${PROJECT_NAME} ${LIB_DL} pthread)
endif()
cmake_minimum_required(VERSION 3.8)
project(GomokuRobot)

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(. main_src)
aux_source_directory(hid hid_src)
aux_source_directory(opencv opencv_src)
aux_source_directory(ai ai_src)

add_executable(${PROJECT_NAME} ${main_src} ${hid_src} ${opencv_src} ${ai_src})

find_package(hidapi REQUIRED)
target_link_libraries(${PROJECT_NAME} hidapi::hidapi)

find_package(OpenCV REQUIRED videoio highgui imgproc)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
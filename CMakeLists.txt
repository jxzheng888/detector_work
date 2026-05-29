cmake_minimum_required(VERSION 3.16)

project(detector_work LANGUAGES CXX)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

file(GLOB sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cc")
file(GLOB headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.hh")

add_executable(detector_work ${sources} ${headers})
target_include_directories(detector_work PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(detector_work PRIVATE ${Geant4_LIBRARIES})

set_property(TARGET detector_work PROPERTY CXX_STANDARD 17)
set_property(TARGET detector_work PROPERTY CXX_STANDARD_REQUIRED ON)

file(GLOB macro_files CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/macros/*.mac")
foreach(macro_file ${macro_files})
  file(COPY "${macro_file}" DESTINATION "${PROJECT_BINARY_DIR}/macros")
endforeach()

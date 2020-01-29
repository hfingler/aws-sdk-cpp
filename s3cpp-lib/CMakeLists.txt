cmake_minimum_required(VERSION 3.2)
project(s3cpp)
set (CMAKE_CXX_STANDARD 11)

#set(AWSSDK_ROOT_DIR, "/musl")
find_package(AWSSDK REQUIRED COMPONENTS transfer s3)
add_definitions(-DUSE_IMPORT_EXPORT)

find_package(Boost)
if(Boost_FOUND)   

  include_directories(${Boost_INCLUDE_DIR})

  #include_directories("/aws-sdk-cpp/boost")
  #message("****adding ${CMAKE_CURRENT_SOURCE_DIR} /aws-sdk-cpp/boost")

  # This builds the binary for experimenting
  #set(EXAMPLES "")
  #list(APPEND EXAMPLES "s3test")
  #foreach(EXAMPLE IN LISTS EXAMPLES)
  #  add_executable(${EXAMPLE} ${EXAMPLE}.cpp)
  #  target_link_libraries(${EXAMPLE} ${AWSSDK_LINK_LIBRARIES})
  #endforeach()


  # This builds the static library
  add_library(s3cpp STATIC
    s3cpp_interface.cpp
  )
  target_link_libraries(s3cpp ${AWSSDK_LINK_LIBRARIES})

endif()

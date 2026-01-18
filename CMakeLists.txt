cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(pitchplay VERSION 1.0.0
	DESCRIPTION "listen to sound files, rpgmaker style"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADER_FILES
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Vorbis CONFIG REQUIRED)

find_package(SndFile CONFIG)
if(TARGET SndFile::sndfile)
	message("wav enabled")
	target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)
endif()

find_package(MPG123 CONFIG)
if(TARGET MPG123::libmpg123)
	message("mp3 enabled")
	target_link_libraries(${PROJECT_NAME} PRIVATE MPG123::libmpg123)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Vorbis::vorbis Vorbis::vorbisfile)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
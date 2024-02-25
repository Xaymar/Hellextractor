cmake_minimum_required(VERSION 3.26...4.0)
project(hellextractor)
list(APPEND CMAKE_MESSAGE_INDENT "[${PROJECT}] ")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE "bcrypt")
endif()

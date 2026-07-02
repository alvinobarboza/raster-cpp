cmake_minimum_required(VERSION 4.2)
project(raster-cpp)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(include_dependency libName gitUrl gitTag)
    FetchContent_Declare(
            ${libName}
            GIT_REPOSITORY ${gitUrl}
            GIT_TAG    ${gitTag}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(${libName})
endfunction()

set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 6.0)
endif()

add_executable(${PROJECT_NAME}
        src/main.cpp
        include/transforms/Vec3.h
        include/transforms/Vec2.h
        include/transforms/Matrix4x4.h
        include/transforms/Constants.h
        include/transforms/Transforms.h
        src/transforms/Vec3.cpp
        src/transforms/Vec2.cpp
        src/transforms/Matrix4x4.cpp
        src/transforms/Transforms.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB1})

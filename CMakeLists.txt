cmake_minimum_required(VERSION 4.2)
project(raster-cpp)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

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
        include/transforms/vec3.h
        include/transforms/vec2.h
        include/transforms/matrix4x4.h
        include/transforms/constants.h
        include/transforms/transforms.h
        src/transforms/vec3.cpp
        src/transforms/vec2.cpp
        src/transforms/matrix4x4.cpp
        src/transforms/transforms.cpp
        include/camera/frustum.h
        include/colliders/sphere.h
        src/colliders/sphere.cpp
        include/colliders/aabb.h
        src/colliders/aabb.cpp
        src/camera/frustum.cpp
        include/camera/camera.h
        include/model/triangle.h
        include/material/texture.h
        include/transforms/vec4.h
        src/transforms/vec4.cpp
        src/material/texture.cpp
        src/model/triangle.cpp
        src/camera/camera.cpp
        include/material/color_convertion.h
        src/material/color_convertion.cpp
        include/model/mesh.h
        src/model/mesh.cpp
        include/engine/renderer.h
        include/material/material.h
        src/material/material.cpp
        include/engine/resourse_manager.h
        include/engine/scene.h
        src/engine/resource_manager.cpp
        src/engine/scene.cpp
        src/engine/renderer.cpp
        include/lights/types.h
        include/lights/light.h
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB1})

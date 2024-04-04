cmake_minimum_required(VERSION 3.16)
project(OpenGL C CXX)
set(CMAKE_CXX_STANDARD 17)


set(INC_DIR
        ./Dependencies/glfw/include
        ./Dependencies/glew/include
        ./Dependencies/glad/include
        ./Dependencies/KHR/include
        ./Dependencies/custom/include
        ./Dependencies/stb_image/include
        ./Dependencies/glm/include
        ./Dependencies/cwd/include
        ./Dependencies/assimp/include
        ./Dependencies/imgui/include
        )

#声明链接库路径
set(LINK_DIR
        ./Dependencies/glfw/lib-mingw-w64
        ./Dependencies/glew/lib/Release/x64
        ./Dependencies/assimp/lib
        )

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

file(GLOB_RECURSE IMGUI_SRCS ./Dependencies/imgui/include/*.cpp)

add_executable(OpenGL src/main.cpp ${IMGUI_SRCS})


target_link_libraries(OpenGL glfw3 opengl32 glew32s libassimp-5)

configure_file(./Dependencies/assimp/lib/libassimp.dll.a ${CMAKE_CURRENT_BINARY_DIR}/libassimp.dll.a COPYONLY)
configure_file(./Dependencies/assimp/lib/libassimp-5.dll ${CMAKE_CURRENT_BINARY_DIR}/libassimp-5.dll COPYONLY)
# 创建 Shader 指向的文件夹
execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Shader)
# 拷贝 Shader 文件夹中的内容
file(GLOB allCopyFiles  ./Shader/*)
file(COPY ${allCopyFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Shader)



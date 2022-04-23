cmake_minimum_required(VERSION 3.5)
project(engine)
#查找当前目录下的所有源文件
#aux_source_directory(. DIR_SRCS)

#添加第三方的库

set(GLFW_INCLUDE_DIR external/glfw/include)
set(GLAD_INCLUDE_DIR external/glad/include)
set(GLM_INCLUDE_DIR external/glm/)
set(GLAD_SOURCE_DIR external/glad/src)
set(GLAD_SOURCE_FILE ${GLAD_SOURCE_DIR}/glad.c) 
set(GLFW_LIB_DIR external/glfw/lib-vc2019)
set(GLFW_LIBS glfw3.dll)
set(IMGUI_INCLUDE_DIR external/imgui)
set(IMGUI_SOURCE_DIR external/imgui)
set(STB_IMAGE_DIR external/stb)
set(OBJ_LOADER_DIR external/tiny_obj_loader)
file(GLOB IMGUI_SOURCE_FILES "${IMGUI_SOURCE_DIR}/*.cpp" ABSOLUTE)

set(base 
    base/application.h
    base/application.cpp
    base/camera.h
    base/camera.cpp
    base/input.h
    base/light.h
    base/model.cpp
    base/model.h
    base/object3d.cpp
    base/object3d.h
    base/shader.cpp
    base/shader.h
    base/skybox.cpp
    base/skybox.h
    base/texture.cpp
    base/texture.h
    base/vertex.h
    base/field.h
    external/tiny_obj_loader/tiny_obj_loader.cc
)

set(animation animation/animation.h)
set(src src/main.cpp
        src/texture_mapping.cpp
        src/texture_mapping.h
)

link_directories(${GLFW_LIB_DIR})
# link_libraries(${GLFW_LIB})
#add_executable(origin ${src} ${animation} ${base} ${CURRENT_DIR} ${IMGUI_SOURCE_FILES} ${GLAD_SOURCE_FILE})  #生成可执行文件
add_executable(MassSpring test/main.cpp ${animation} ${base} ${CURRENT_DIR} ${IMGUI_SOURCE_FILES} ${GLAD_SOURCE_FILE})  #生成可执行文件

#target_include_directories(origin PRIVATE animation/ external/imgui/ ${GLM_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${STB_IMAGE_DIR} ${OBJ_LOADER_DIR})
#target_link_libraries(origin ${GLFW_LIBS} )
target_include_directories(MassSpring PRIVATE base/ animation/ external/imgui/ ${GLM_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${STB_IMAGE_DIR} ${OBJ_LOADER_DIR})
target_link_libraries(MassSpring ${GLFW_LIBS} )
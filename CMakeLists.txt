cmake_minimum_required(VERSION 3.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(saba)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
option (SABA_BUILD_MMD2OBJ "Build mmd2obj." off)
if (SABA_BUILD_MMD2OBJ)
    option (SABA_BUILD_OBJ_MODEL "Build obj model." on)
else()
    option (SABA_BUILD_OBJ_MODEL "Build obj model." off)
endif()
option (SABA_BUILD_X_MODEL "Build x model." on)
set (SABA_BULLET_ROOT "" CACHE PATH "Bullet Root Directory")
option (SABA_ENABLE_TEST "Enable Google test." on)
option (SABA_INSTALL "Saba install." off)

set (BULLET_ROOT ${SABA_BULLET_ROOT})
MESSAGE( STATUS "BULLET_ROOT=${BULLET_ROOT}")

find_package (Bullet REQUIRED)

if (SABA_ENABLE_TEST)
    enable_testing()
endif()

if (SABA_ENABLE_EXAMPLE_VULKAN)
    find_package(Vulkan REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 17)

include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/external/stb/include)
include_directories(${PROJECT_SOURCE_DIR}/external/glm)
include_directories(${PROJECT_SOURCE_DIR}/external/gli/include)
include_directories(${PROJECT_SOURCE_DIR}/external/spdlog/include)

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE)
    ADD_DEFINITIONS(-D_UNICODE)
endif(WIN32)

if(MSVC)
    ADD_DEFINITIONS(/MP)
endif()

add_subdirectory(src)
add_subdirectory(gtests)

if (SABA_BUILD_MMD2OBJ)
    add_executable(mmd2obj mmd2obj.cpp)
    target_link_libraries(mmd2obj Saba)
endif()

# Install
if (SABA_INSTALL)
    install (DIRECTORY src/Saba DESTINATION include  FILES_MATCHING PATTERN "*.h")
    # copy glm to include
    install (DIRECTORY external/glm/glm DESTINATION include)
    if (SABA_BUILD_MMD2OBJ)
        install (TARGETS mmd2obj RUNTIME DESTINATION bin)
    endif()
endif()

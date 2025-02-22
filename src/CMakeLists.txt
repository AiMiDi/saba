
# Base
set (
    BASE_SOURCE
    Saba/Base/File.cpp
    Saba/Base/Log.cpp
    Saba/Base/Path.cpp
    Saba/Base/Singleton.cpp
    Saba/Base/Time.cpp
    Saba/Base/UnicodeUtil.cpp
)
set (
    BASE_HEADER
    Saba/Base/File.h
    Saba/Base/Log.h
    Saba/Base/Path.h
    Saba/Base/Singleton.h
    Saba/Base/Time.h
    Saba/Base/UnicodeUtil.h
)

# OBJ Model
if (SABA_BUILD_OBJ_MODEL)
    set (
        MODEL_OBJ_SOURCE
        Saba/Model/OBJ/OBJModel.cpp
        Saba/Model/OBJ/tinyobjloader.cpp
    )
    set (
        MODEL_OBJ_HEADER
        Saba/Model/OBJ/OBJModel.h
    )
endif ()

# XFile Model
if (SABA_BUILD_X_MODEL)
set (
    MODEL_XFILE_SOURCE
    Saba/Model/XFile/XFileModel.cpp
)

set (
    MODEL_XFILE_HEADER
    Saba/Model/XFile/XFileModel.h
)
endif ()

# MMD Model
set (
    MODEL_MMD_SOURCE
    Saba/Model/MMD/MMDIkSolver.cpp
    Saba/Model/MMD/MMDMaterial.cpp
    Saba/Model/MMD/MMDModel.cpp
    Saba/Model/MMD/MMDMorph.cpp
    Saba/Model/MMD/MMDNode.cpp
    Saba/Model/MMD/MMDPhysics.cpp
    Saba/Model/MMD/MMDCamera.cpp
    Saba/Model/MMD/PMDFile.cpp
    Saba/Model/MMD/PMDModel.cpp
    Saba/Model/MMD/PMXFile.cpp
    Saba/Model/MMD/PMXModel.cpp
    Saba/Model/MMD/SjisToUnicode.cpp
    Saba/Model/MMD/VMDAnimation.cpp
    Saba/Model/MMD/VMDCameraAnimation.cpp
    Saba/Model/MMD/VMDFile.cpp
    Saba/Model/MMD/VPDFile.cpp
)
set (
    MODEL_MMD_HEADER
    Saba/Model/MMD/MMDFileString.h
    Saba/Model/MMD/MMDIkSolver.h
    Saba/Model/MMD/MMDMaterial.h
    Saba/Model/MMD/MMDModel.h
    Saba/Model/MMD/MMDMorph.h
    Saba/Model/MMD/MMDNode.h
    Saba/Model/MMD/MMDPhysics.h
    Saba/Model/MMD/MMDCamera.h
    Saba/Model/MMD/PMDFile.h
    Saba/Model/MMD/PMDModel.h
    Saba/Model/MMD/PMXFile.h
    Saba/Model/MMD/PMXModel.h
    Saba/Model/MMD/SjisToUnicode.h
    Saba/Model/MMD/VMDAnimation.h
    Saba/Model/MMD/VMDCameraAnimation.h
    Saba/Model/MMD/VMDAnimationCommon.hpp
    Saba/Model/MMD/VMDFile.h
    Saba/Model/MMD/VPDFile.h
)

add_library(
    Saba
    ${BASE_SOURCE}
    ${BASE_HEADER}
    ${MODEL_OBJ_SOURCE}
    ${MODEL_OBJ_HEADER}
    ${MODEL_XFILE_SOURCE}
    ${MODEL_XFILE_HEADER}
    ${MODEL_MMD_SOURCE}
    ${MODEL_MMD_HEADER}
)

target_include_directories(Saba PUBLIC ${PROJECT_SOURCE_DIR}/external/glm)
target_include_directories(Saba PRIVATE ${PROJECT_SOURCE_DIR}/external/spdlog/include)

if (SABA_BUILD_OBJ_MODEL)
    target_include_directories(Saba PRIVATE ${PROJECT_SOURCE_DIR}/external/tinyobjloader/include)
endif ()
if (SABA_BUILD_X_MODEL)
    target_include_directories(Saba PRIVATE ${PROJECT_SOURCE_DIR}/external/tinyxfileloader/include)
endif ()

set (Saba_LIBRARIES ${BULLET_LIBRARIES})
if (UNIX)
    find_package(Threads REQUIRED)
    list (APPEND Saba_LIBRARIES ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(Saba PRIVATE ${BULLET_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif ()
target_link_libraries(Saba PRIVATE ${Saba_LIBRARIES})

SOURCE_GROUP(Base FILES ${BASE_SOURCE} ${BASE_HEADER})
if (SABA_BUILD_OBJ_MODEL)
    SOURCE_GROUP(Model\\OBJ FILES ${MODEL_OBJ_SOURCE} ${MODEL_OBJ_HEADER})
endif ()
if (SABA_BUILD_X_MODEL)
    SOURCE_GROUP(Model\\XFile FILES ${MODEL_XFILE_SOURCE} ${MODEL_XFILE_HEADER})
endif ()
SOURCE_GROUP(Model\\MMD FILES ${MODEL_MMD_SOURCE} ${MODEL_MMD_HEADER})

# install
if (SABA_INSTALL)
    install (TARGETS Saba DESTINATION lib)
endif()

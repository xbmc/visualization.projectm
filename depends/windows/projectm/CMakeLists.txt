cmake_minimum_required(VERSION 3.5)
project(libprojectM)

find_package(kodi-angle REQUIRED)
get_target_property(OPENGLES_INCLUDE_DIRS kodi::angle::libGLESv2 INTERFACE_INCLUDE_DIRECTORIES)

set(DATADIR "${CMAKE_INSTALL_PREFIX}/share")
configure_file(${PROJECT_SOURCE_DIR}/src/libprojectM/config.inp.in ${PROJECT_SOURCE_DIR}/src/libprojectM/config.inp @ONLY)

set(SOURCES src/libprojectM/ConfigFile.cpp
            src/libprojectM/fftsg.cpp
            src/libprojectM/KeyHandler.cpp
            src/libprojectM/PCM.cpp
            src/libprojectM/PipelineMerger.cpp
            src/libprojectM/PresetChooser.cpp
            src/libprojectM/Preset.cpp
            src/libprojectM/PresetFactory.cpp
            src/libprojectM/PresetFactoryManager.cpp
            src/libprojectM/PresetLoader.cpp
            src/libprojectM/projectM.cpp
            src/libprojectM/TestRunner.cpp
            src/libprojectM/TimeKeeper.cpp
            src/libprojectM/timer.cpp
            src/libprojectM/wipemalloc.cpp
            src/libprojectM/MilkdropPresetFactory/BuiltinFuncs.cpp
            src/libprojectM/MilkdropPresetFactory/BuiltinParams.cpp
            src/libprojectM/MilkdropPresetFactory/CustomShape.cpp
            src/libprojectM/MilkdropPresetFactory/CustomWave.cpp
            src/libprojectM/MilkdropPresetFactory/Eval.cpp
            src/libprojectM/MilkdropPresetFactory/Expr.cpp
            src/libprojectM/MilkdropPresetFactory/Func.cpp
            src/libprojectM/MilkdropPresetFactory/IdlePreset.cpp
            src/libprojectM/MilkdropPresetFactory/InitCond.cpp
            src/libprojectM/MilkdropPresetFactory/MilkdropPreset.cpp
            src/libprojectM/MilkdropPresetFactory/MilkdropPresetFactory.cpp
            src/libprojectM/MilkdropPresetFactory/Param.cpp
            src/libprojectM/MilkdropPresetFactory/Parser.cpp
            src/libprojectM/MilkdropPresetFactory/PerFrameEqn.cpp
            src/libprojectM/MilkdropPresetFactory/PerPixelEqn.cpp
            src/libprojectM/MilkdropPresetFactory/PerPointEqn.cpp
            src/libprojectM/MilkdropPresetFactory/PresetFrameIO.cpp
            src/libprojectM/NativePresetFactory/NativePresetFactory.cpp
            src/libprojectM/Renderer/BeatDetect.cpp
            src/libprojectM/Renderer/Filters.cpp
            src/libprojectM/Renderer/MilkdropWaveform.cpp
            src/libprojectM/Renderer/PerlinNoise.cpp
            src/libprojectM/Renderer/PerlinNoiseWithAlpha.cpp
            src/libprojectM/Renderer/PerPixelMesh.cpp
            src/libprojectM/Renderer/PipelineContext.cpp
            src/libprojectM/Renderer/Pipeline.cpp
            src/libprojectM/Renderer/Renderable.cpp
            src/libprojectM/Renderer/Renderer.cpp
            src/libprojectM/Renderer/RenderItemDistanceMetric.cpp
            src/libprojectM/Renderer/RenderItemMatcher.cpp
            src/libprojectM/Renderer/Shader.cpp
            src/libprojectM/Renderer/ShaderEngine.cpp
            src/libprojectM/Renderer/Texture.cpp
            src/libprojectM/Renderer/TextureManager.cpp
            src/libprojectM/Renderer/VideoEcho.cpp
            src/libprojectM/Renderer/Waveform.cpp
            src/libprojectM/Renderer/SOIL2/etc1_utils.c
            src/libprojectM/Renderer/SOIL2/image_DXT.c
            src/libprojectM/Renderer/SOIL2/image_helper.c
            src/libprojectM/Renderer/SOIL2/SOIL2.c
            src/libprojectM/Renderer/hlslparser/src/CodeWriter.cpp
            src/libprojectM/Renderer/hlslparser/src/Engine.cpp
            src/libprojectM/Renderer/hlslparser/src/GLSLGenerator.cpp
            src/libprojectM/Renderer/hlslparser/src/HLSLParser.cpp
            src/libprojectM/Renderer/hlslparser/src/HLSLTokenizer.cpp
            src/libprojectM/Renderer/hlslparser/src/HLSLTree.cpp)

set(HEADERS src/libprojectM/BackgroundWorker.h
            src/libprojectM/carbontoprojectM.h
            src/libprojectM/cocoatoprojectM.h
            src/libprojectM/ConfigFile.h
            src/libprojectM/dlldefs.h
            src/libprojectM/event.h
            src/libprojectM/fatal.h
            src/libprojectM/fftsg.h
            src/libprojectM/glError.h
            src/libprojectM/lvtoprojectM.h
            src/libprojectM/projectM-opengl.h
            src/libprojectM/resource.h
            src/libprojectM/sdltoprojectM.h
            src/libprojectM/timer.h
            src/libprojectM/wipemalloc.h
            src/libprojectM/Common.hpp
            src/libprojectM/HungarianMethod.hpp
            src/libprojectM/IdleTextures.hpp
            src/libprojectM/KeyHandler.hpp
            src/libprojectM/PCM.hpp
            src/libprojectM/PipelineMerger.hpp
            src/libprojectM/PresetChooser.hpp
            src/libprojectM/PresetFactory.hpp
            src/libprojectM/PresetFactoryManager.hpp
            src/libprojectM/Preset.hpp
            src/libprojectM/PresetLoader.hpp
            src/libprojectM/projectM.hpp
            src/libprojectM/RandomNumberGenerators.hpp
            src/libprojectM/TestRunner.hpp
            src/libprojectM/TimeKeeper.hpp
            src/libprojectM/MilkdropPresetFactory/BuiltinFuncs.hpp
            src/libprojectM/MilkdropPresetFactory/BuiltinParams.hpp
            src/libprojectM/MilkdropPresetFactory/CustomShape.hpp
            src/libprojectM/MilkdropPresetFactory/CustomWave.hpp
            src/libprojectM/MilkdropPresetFactory/CValue.hpp
            src/libprojectM/MilkdropPresetFactory/Eval.hpp
            src/libprojectM/MilkdropPresetFactory/Expr.hpp
            src/libprojectM/MilkdropPresetFactory/Func.hpp
            src/libprojectM/MilkdropPresetFactory/IdlePreset.hpp
            src/libprojectM/MilkdropPresetFactory/InitCond.hpp
            src/libprojectM/MilkdropPresetFactory/InitCondUtils.hpp
            src/libprojectM/MilkdropPresetFactory/JitContext.hpp
            src/libprojectM/MilkdropPresetFactory/MilkdropPresetFactory.hpp
            src/libprojectM/MilkdropPresetFactory/MilkdropPreset.hpp
            src/libprojectM/MilkdropPresetFactory/Param.hpp
            src/libprojectM/MilkdropPresetFactory/ParamUtils.hpp
            src/libprojectM/MilkdropPresetFactory/Parser.hpp
            src/libprojectM/MilkdropPresetFactory/PerFrameEqn.hpp
            src/libprojectM/MilkdropPresetFactory/PerPixelEqn.hpp
            src/libprojectM/MilkdropPresetFactory/PerPointEqn.hpp
            src/libprojectM/MilkdropPresetFactory/PresetFrameIO.hpp
            src/libprojectM/NativePresetFactory/MilkdropCompatability.hpp
            src/libprojectM/NativePresetFactory/NativePresetFactory.hpp
            src/libprojectM/NativePresetFactory/NativePreset.hpp
            src/libprojectM/Renderer/BeatDetect.hpp
            src/libprojectM/Renderer/Filters.hpp
            src/libprojectM/Renderer/MilkdropWaveform.hpp
            src/libprojectM/Renderer/PerlinNoise.hpp
            src/libprojectM/Renderer/PerlinNoiseWithAlpha.hpp
            src/libprojectM/Renderer/PerPixelMesh.hpp
            src/libprojectM/Renderer/PipelineContext.hpp
            src/libprojectM/Renderer/Pipeline.hpp
            src/libprojectM/Renderer/Renderable.hpp
            src/libprojectM/Renderer/Renderer.hpp
            src/libprojectM/Renderer/RenderItemDistanceMetric.hpp
            src/libprojectM/Renderer/RenderItemMatcher.hpp
            src/libprojectM/Renderer/RenderItemMergeFunction.hpp
            src/libprojectM/Renderer/ShaderEngine.hpp
            src/libprojectM/Renderer/Shader.hpp
            src/libprojectM/Renderer/Texture.hpp
            src/libprojectM/Renderer/TextureManager.hpp
            src/libprojectM/Renderer/Transformation.hpp
            src/libprojectM/Renderer/VideoEcho.hpp
            src/libprojectM/Renderer/Waveform.hpp
            src/libprojectM/Renderer/SOIL2/etc1_utils.h
            src/libprojectM/Renderer/SOIL2/image_DXT.h
            src/libprojectM/Renderer/SOIL2/image_helper.h
            src/libprojectM/Renderer/SOIL2/pkm_helper.h
            src/libprojectM/Renderer/SOIL2/pvr_helper.h
            src/libprojectM/Renderer/SOIL2/SOIL2.h
            src/libprojectM/Renderer/SOIL2/stbi_DDS_c.h
            src/libprojectM/Renderer/SOIL2/stbi_DDS.h
            src/libprojectM/Renderer/SOIL2/stbi_ext_c.h
            src/libprojectM/Renderer/SOIL2/stbi_ext.h
            src/libprojectM/Renderer/SOIL2/stb_image.h
            src/libprojectM/Renderer/SOIL2/stb_image_write.h
            src/libprojectM/Renderer/SOIL2/stbi_pkm_c.h
            src/libprojectM/Renderer/SOIL2/stbi_pkm.h
            src/libprojectM/Renderer/SOIL2/stbi_pvr_c.h
            src/libprojectM/Renderer/SOIL2/stbi_pvr.h
            src/libprojectM/Renderer/hlslparser/src/CodeWriter.h
            src/libprojectM/Renderer/hlslparser/src/Engine.h
            src/libprojectM/Renderer/hlslparser/src/GLSLGenerator.h
            src/libprojectM/Renderer/hlslparser/src/HLSLParser.h
            src/libprojectM/Renderer/hlslparser/src/HLSLTokenizer.h
            src/libprojectM/Renderer/hlslparser/src/HLSLTree.h
            src/libprojectM/omptl/omptl_algorithm_par.h
            src/libprojectM/omptl/omptl_algorithm_ser.h
            src/libprojectM/omptl/omptl_numeric_extentions.h
            src/libprojectM/omptl/omptl_numeric_extentions_par.h
            src/libprojectM/omptl/omptl_numeric_extentions_ser.h
            src/libprojectM/omptl/omptl_numeric_par.h
            src/libprojectM/omptl/omptl_numeric_ser.h
            src/libprojectM/omptl/omptl_tools.h)

add_library(projectM STATIC ${SOURCES} ${HEADERS})
set_property(TARGET projectM PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(projectM PRIVATE ${PROJECT_SOURCE_DIR}
                                            ${PROJECT_SOURCE_DIR}/src/libprojectM
                                            ${PROJECT_SOURCE_DIR}/src/libprojectM/MilkdropPresetFactory
                                            ${PROJECT_SOURCE_DIR}/src/libprojectM/NativePresetFactory
                                            ${PROJECT_SOURCE_DIR}/src/libprojectM/Renderer
                                            ${PROJECT_SOURCE_DIR}/src/libprojectM/Renderer/hlslparser/src
                                            ${OPENGLES_INCLUDE_DIRS})
target_compile_definitions(projectM PRIVATE -DDATADIR_PATH="${CMAKE_INSTALL_PREFIX}/share"
                                            -DUSE_GLES
                                            -DHAS_ANGLE
                                            -DprojectM_FONT_TITLE="Vera.ttf"
                                            -DprojectM_FONT_MENU="VeraMono.ttf"
                                            -DSOIL_NO_EGL)

install(FILES ${PROJECT_SOURCE_DIR}/src/libprojectM/Common.hpp
              ${PROJECT_SOURCE_DIR}/src/libprojectM/PCM.hpp
              ${PROJECT_SOURCE_DIR}/src/libprojectM/dlldefs.h
              ${PROJECT_SOURCE_DIR}/src/libprojectM/event.h
              ${PROJECT_SOURCE_DIR}/src/libprojectM/fatal.h
              ${PROJECT_SOURCE_DIR}/src/libprojectM/projectM.hpp
        DESTINATION include/libprojectM)
install(FILES ${PROJECT_SOURCE_DIR}/src/libprojectM/config.inp
        DESTINATION share/projectM)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/fonts
                  ${PROJECT_SOURCE_DIR}/presets
        DESTINATION share/projectM)
install(TARGETS projectM DESTINATION lib)

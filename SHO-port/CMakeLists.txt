# ── sho-game: the whole game, in one executable ──────────────────────────────
#
# Two halves. `play_main.cpp` is the front end -- splash, language, aspect, the
# logo movie, the main menu, the intro clip and the scene queue, all transcribed
# from SLES_551.47 -- and `src/Main.cpp` takes over at the queue's handover
# command and runs the level. One window, one process. See
# include/ClimaxEngine/Game/PlayMain.h.
#
# There is no gameplay library here any more. The one that was (`sho-port`:
# Travis, monsters, combat, puzzles, items) was written without the decompiled
# executable and is kept, unbuilt, under archive/gemini-unverified/ -- see the
# AUDIT.md there. Gameplay code comes back into this directory one recovered
# function at a time, each with its SLES_551.47 address beside it.
set(SHO_GAME_SOURCES
    src/Main.cpp
    ${CMAKE_SOURCE_DIR}/src/play_main.cpp
    ${CMAKE_SOURCE_DIR}/src/Viewer/ViewerGraphics.cpp
    ${CMAKE_SOURCE_DIR}/src/Viewer/ViewerDraw.cpp
    ${CMAKE_SOURCE_DIR}/src/Viewer/ViewerAudio.cpp
    ${CMAKE_SOURCE_DIR}/src/UI/UI.cpp
    ${CMAKE_SOURCE_DIR}/src/Loader/Loader.cpp
    ${CMAKE_SOURCE_DIR}/src/Loader/GeometryDecoder.cpp
    ${CMAKE_SOURCE_DIR}/src/Loader/ResourceLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/Loader/Export.cpp
    ${CMAKE_SOURCE_DIR}/src/SG/SceneObject.cpp
    ${CMAKE_SOURCE_DIR}/src/Platform/Wii/WiiTexture.cpp
    ${CMAKE_SOURCE_DIR}/src/Platform/PS2/RwsAudio.cpp
    ${CMAKE_SOURCE_DIR}/src/Core/Common.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/RenderBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/GLSLBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/CPURasterizer.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/GPUMesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/GLTextureSink.cpp
    ${CMAKE_SOURCE_DIR}/src/Rendering/PlayerModel.cpp
    ${IMGUI_SOURCES}
    ${IMGUIZMO_SOURCES}
    ${IMANIM_SOURCES}
)
if(APPLE)
    list(APPEND SHO_GAME_SOURCES ${CMAKE_SOURCE_DIR}/src/Rendering/MetalBackend.mm)
endif()
# The boot movies and the intro. Optional at configure time: without it the
# front end still runs, it just says so instead of drawing an invented logo.
if(SHO_HAVE_FFMPEG)
    list(APPEND SHO_GAME_SOURCES ${CMAKE_SOURCE_DIR}/src/Rendering/VideoPlayer.cpp)
endif()

add_executable(sho-game ${SHO_GAME_SOURCES})

target_include_directories(sho-game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${IMGUIZMO_DIR}
    ${CMAKE_SOURCE_DIR}/src/UI/ImAnim
)

target_link_libraries(sho-game PRIVATE
    climax-harness
    climax-game
    climax-core
    OpenGL::GL
    GLEW::GLEW
    ${SHO_SDL2_TARGETS}
    ${SHO_SDL2_IMAGE}
    ${EXTRA_LIBS}
)

if(SDL2_INCLUDE_DIRS)
    target_include_directories(sho-game PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

if(SHO_HAVE_FFMPEG)
    target_include_directories(sho-game PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(sho-game PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(sho-game PRIVATE ${FFMPEG_LIBRARIES})
    target_compile_definitions(sho-game PRIVATE CLIMAX_HAVE_FFMPEG=1)
else()
    message(STATUS "libavformat/libavcodec/libswscale not found: sho-game will build without movie playback")
endif()

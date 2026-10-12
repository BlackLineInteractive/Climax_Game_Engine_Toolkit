// ─────────────────────────────────────────────────────────────────────────────
// sho-game — the level half.
//
// The front end (play_main.cpp) runs the boot sequence and the menu out of
// SLES_551.47 and hands over a live window at the scene queue's handover
// command. From there this file loads the scene and lets you walk it.
//
// What is the game's own, read out of the retail data:
//   * the level, its collision (CBSP), cameras and placed objects;
//   * the spawn point (CPlayerSpawner, named after the zone you came from);
//   * which cameras a PlaneTrigger names, one per side;
//   * doors (ZoneTrigger names destination, source, event and button);
//   * Travis's model and his idle / walk / run clips by authored filename;
//   * the fade the scene queue started, carried on past the handover.
//
// What is NOT the game's, and is marked HARNESS below or lives in
// ClimaxEngine/Harness/: input mapping, walk speed, turn rate, the choice of
// clip, the sphere body and its collision, how a camera aims and where its eye
// goes, which side of a PlaneTrigger is which, and how far a door reaches.
// Those belong to CPlayerBehaviour (FUN_001300F0, not yet read --
// SHO-port/docs/PLAYER.md), CStaticCamera / CConstraintCamera, PlaneTrigger
// and ZoneTrigger. The harness is here so the recovered pieces can be seen
// working, and it is the socket the recovered code plugs into. Nothing in it
// claims to be the original.
// ─────────────────────────────────────────────────────────────────────────────
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include "ClimaxEngine/Core/RWS/FileSystem/CArchiveManager.h"
#include "ClimaxEngine/Game/CameraLinks.h"
#include "ClimaxEngine/Game/PlayMain.h"
#include "ClimaxEngine/Game/ZoneLinks.h"
#include "ClimaxEngine/Harness/Body.h"
#include "ClimaxEngine/Harness/CameraView.h"
#include "ClimaxEngine/Loader/Loader.h"
#include "ClimaxEngine/Platform/PS2/RwsAudio.h"
#include "ClimaxEngine/Render/PlayerModel.h"
#include "ClimaxEngine/Render/ViewerState.h"
#include "ClimaxEngine/SG/SceneObject.h"
#include "ClimaxEngine/Viewer/ViewerGraphics.h"

void InstallGLTextureSink();

// Read by the shared renderer (ViewerGraphics.cpp) to place Travis.
glm::vec3   s_playerFeet(0.0f);
float       s_playerYaw = 0.0f;
std::string s_playerClipName;
bool        s_showPlayerPieces = false;

// UI.cpp links against these; the game has no file browser, so they only
// remember which archive was mounted.
static const char *kPrefsPath = "SHO-port.prefs";
void SaveArcPref(const std::string &arcPath) {
    if (arcPath.empty()) return;
    std::ofstream f(kPrefsPath, std::ios::trunc);
    if (f) f << arcPath << "\n";
}
std::string LoadArcPref() {
    std::ifstream f(kPrefsPath);
    std::string line;
    if (f && std::getline(f, line) && !line.empty()) return line;
    return {};
}

int main(int argc, char **argv) {
    std::string arcPath = "game-iso/SHO/SH.ARC";
    std::string levelName = "IntroRoad";

    // --level <name> skips the front end and drops straight into a room. It is
    // for working on the level, not for playing: the retail path is the boot
    // sequence, and that is what happens with no arguments.
    bool skipFrontEnd = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--level" && i + 1 < argc) {
            levelName = argv[++i];
            skipFrontEnd = true;
        } else if (a.size() > 4 && a.substr(a.size() - 4) == ".arc") {
            arcPath = a;
        }
    }

    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    float fadeLevel = 0.0f, fadeRate = 0.0f, fadeTarget = 0.0f;

    // ── the front end ───────────────────────────────────────────────────────
    // Splash, language, memory card, aspect, the logo movie, the main menu,
    // and on a new game the intro clip and the scene queue (docs/TODO.md
    // §6c-6f). It hands back the window it has been drawing into.
    if (!skipFrontEnd) {
        ClimaxEngine::Game::FrontEndExit fe;
        const int rc = ClimaxEngine::Game::RunFrontEnd(argc, argv, fe);
        if (rc != 0) return rc;
        if (!fe.startGame) return 0;
        window = fe.window;
        glContext = fe.context;
        if (!fe.firstScene.empty()) levelName = fe.firstScene;
        if (!fe.archive.empty()) arcPath = fe.archive;
        fadeLevel = fe.fadeLevel;
        fadeRate = fe.fadeRate;
        fadeTarget = fe.fadeTarget;
    }

    if (!window) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
            std::cerr << "[sho-game] SDL_Init failed: " << SDL_GetError() << "\n";
            return 1;
        }
        IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        window = SDL_CreateWindow("Silent Hill: Origins", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, 1280, 720,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                      SDL_WINDOW_ALLOW_HIGHDPI);
        if (!window) {
            std::cerr << "[sho-game] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
            return 1;
        }
        glContext = SDL_GL_CreateContext(window);
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[sho-game] GLEW initialization failed\n";
        return 1;
    }

    // ImGui only draws the harness's door prompt.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    InstallGLTextureSink();

    auto &arcMgr = ClimaxEngine::RWS::FileSystem::CArchiveManager::GetInstance();
    if (!arcMgr.GetFirstArchive() && !arcMgr.Mount(arcPath)) {
        std::cerr << "[sho-game] cannot mount " << arcPath << "\n";
        return 1;
    }

    // Plain presentation: none of the toolkit's markers, labels or overlays.
    state.playMode = true;
    state.autoCameras = true;
    state.showUI = false;
    state.showClumps = false;
    state.showGameObjects = false;
    state.showObjectLabels = false;
    state.showPivotGizmo = false;
    state.showCollision = false;
    state.showBoneOverlay = false;
    state.showWireframe = false;
    // The toolkit's grey backdrop is a viewer setting, not the game's.
    for (float &c : state.skyColorBot) c = 0.0f;
    for (float &c : state.skyColorTop) c = 0.0f;

    ClimaxEngine::Harness::CharacterController body;   // HARNESS
    ClimaxEngine::Game::CameraSwitcher switcher;
    std::vector<ClimaxEngine::Game::ZoneLink> zoneLinks;
    std::string arrivedFrom;   // zone the player came out of

    auto loadLevel = [&](const std::string &name) {
        auto *arc = arcMgr.GetFirstArchive();
        const int idx = arc ? arc->Find(name) : -1;
        if (idx < 0) {
            std::cerr << "[sho-game] no container named " << name << " in the archive\n";
            return false;
        }
        LoadLevelFromArc(idx);
        levelName = name;

        zoneLinks = ClimaxEngine::Game::BuildZoneLinks(g_GameObjects);
        switcher.Reset(ClimaxEngine::Game::BuildCameraSwitches(g_GameObjects, g_Cameras));

        // The spawner you arrive on is named after the zone you came from;
        // on a fresh start there is none, so the level's CPlayerSpawner.
        glm::vec3 spawn(0.0f), facing(0.0f, 0.0f, 1.0f);
        if (!arrivedFrom.empty() &&
            ClimaxEngine::Game::FindZoneSpawn(g_GameObjects, arrivedFrom, spawn, facing)) {
            body.position = spawn;
            s_playerYaw = glm::degrees(atan2f(-facing.x, -facing.z));
        } else if (ClimaxEngine::Game::FindPlayerSpawn(g_GameObjects, spawn)) {
            body.position = spawn;
        } else {
            std::cerr << "[sho-game] " << name << " has no spawn point for '"
                      << arrivedFrom << "'; standing at the origin\n";
            body.position = glm::vec3(0.0f);
        }
        body.velocity = glm::vec3(0.0f);
        body.SnapToGround(g_Collision);
        s_playerFeet = body.position - glm::vec3(0.0f, body.radius, 0.0f);

        state.activeCamera = switcher.Update(body.position);
        if (state.activeCamera < 0 && !g_Cameras.empty()) state.activeCamera = 0;
        return true;
    };

    if (!loadLevel(levelName)) return 1;
    LoadPlayerModel("CPlayerBehaviour.Travis");

    ClimaxEngine::Viewer::ViewerGraphics graphics;
    graphics.Init();

    bool running = true;
    uint32_t lastTicks = SDL_GetTicks();
    std::string doorPrompt;

    while (running) {
        const uint32_t now = SDL_GetTicks();
        const float dt = std::min((now - lastTicks) / 1000.0f, 0.05f);
        lastTicks = now;

        bool usePressed = false;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;
                // HARNESS: E stands in for MSG_PAD_GRAB, the button the
                // ZoneTrigger asks for.
                if (ev.key.keysym.sym == SDLK_e) usePressed = true;
            }
        }

        // ── HARNESS: movement ────────────────────────────────────────────────
        // Camera-relative, as the original is; the speed is the toolkit's
        // walk-mode knob, not a value read from the game.
        const uint8_t *keys = SDL_GetKeyboardState(nullptr);
        glm::vec3 fwd(0, 0, 1), right(1, 0, 0);
        if (state.activeCamera >= 0 && state.activeCamera < (int)g_Cameras.size()) {
            glm::vec3 f = g_Cameras[(size_t)state.activeCamera].forward;
            f.y = 0.0f;
            if (glm::length(f) > 1e-3f) fwd = glm::normalize(f);
            right = glm::normalize(glm::cross(fwd, glm::vec3(0, 1, 0)));
        }
        glm::vec3 wish(0.0f);
        if (keys[SDL_SCANCODE_W]) wish += fwd;
        if (keys[SDL_SCANCODE_S]) wish -= fwd;
        if (keys[SDL_SCANCODE_A]) wish -= right;
        if (keys[SDL_SCANCODE_D]) wish += right;
        if (glm::length(wish) > 1e-4f) wish = glm::normalize(wish);
        const bool moving = glm::length(wish) > 1e-3f;
        const bool running_ = moving && keys[SDL_SCANCODE_LSHIFT];
        const float speed = state.walkSpeed * (running_ ? 2.0f : 1.0f);

        body.Step(g_Collision, wish * speed * dt, dt);
        s_playerFeet = body.position - glm::vec3(0.0f, body.radius, 0.0f);
        if (moving) {
            const float want = glm::degrees(atan2f(wish.x, wish.z));
            float d = want - s_playerYaw;
            while (d > 180.0f) d -= 360.0f;
            while (d < -180.0f) d += 360.0f;
            s_playerYaw += d * glm::min(1.0f, dt * 12.0f);
        }

        // HARNESS: idle / walk / run by what the body is doing. The clips are
        // Travis's own, found by authored name; the choice is not the game's.
        g_Player.Advance(dt);
        if (g_Player.loaded) {
            int want = moving ? (running_ ? g_Player.runClip : g_Player.walkClip)
                              : g_Player.idleClip;
            if (want < 0) want = g_Player.idleClip;
            if (want >= 0 && want != g_Player.currentClip)
                s_playerClipName = g_Player.PlayClipAt(want);
        }

        // ── camera cuts: PlaneTrigger ────────────────────────────────────────
        const int cut = switcher.Update(body.position);
        if (cut >= 0 && cut < (int)g_Cameras.size()) state.activeCamera = cut;

        // ── doors: ZoneTrigger ───────────────────────────────────────────────
        const int link = ClimaxEngine::Game::ZoneLinkAt(zoneLinks, body.position, 0.9f);
        doorPrompt.clear();
        if (link >= 0) {
            doorPrompt = zoneLinks[(size_t)link].toZone;
            if (usePressed) {
                arrivedFrom = zoneLinks[(size_t)link].fromZone;
                std::cout << "[zone] " << arrivedFrom << " -> " << doorPrompt << "\n";
                loadLevel(doorPrompt);
            }
        }

        // ── render ───────────────────────────────────────────────────────────
        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);

        const glm::vec3 subject = body.position + glm::vec3(0.0f, 1.2f, 0.0f);
        glm::vec3 eye = body.position + glm::vec3(0.0f, 2.5f, -4.0f);
        glm::vec3 look = glm::normalize(subject - eye);
        float fov = 50.0f;
        if (state.activeCamera >= 0 && state.activeCamera < (int)g_Cameras.size()) {
            const auto &cam = g_Cameras[(size_t)state.activeCamera];
            if (cam.fovDeg > 5.0f) fov = cam.fovDeg;
            ClimaxEngine::Harness::ResolveCameraView(g_Collision, cam, subject, eye, look);
        }
        const glm::mat4 view = glm::lookAt(eye, eye + look, glm::vec3(0, 1, 0));
        const glm::mat4 proj = glm::perspective(glm::radians(fov),
                                                (float)w / (float)std::max(h, 1), 0.1f, 250.0f);
        const size_t meshes =
            ClimaxEngine::SG::CSceneObjectRegistrar::GetInstance().GetObjects().size();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        graphics.RenderFrame(w, h, w, h, proj * view, eye, look, meshes);

        // HARNESS: the prompt names the destination container. The game's own
        // prompt text is not recovered yet, so no text pretends to be it.
        if (!doorPrompt.empty()) {
            ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h - 70.0f), ImGuiCond_Always,
                                    ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGui::Begin("door", nullptr,
                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
            ImGui::Text("[E] %s", doorPrompt.c_str());
            ImGui::End();
        }
        // The closing transition of the scene queue, still running.
        if (fadeRate != 0.0f) {
            const float step = fadeRate * dt;
            if (std::fabs(fadeTarget - fadeLevel) <= step) {
                fadeLevel = fadeTarget;
                fadeRate = 0.0f;
            } else {
                fadeLevel += fadeTarget > fadeLevel ? step : -step;
            }
        }
        if (fadeLevel > 0.0f)
            ImGui::GetForegroundDrawList()->AddRectFilled(
                ImVec2(0, 0), ImGui::GetIO().DisplaySize,
                IM_COL32(0, 0, 0, (int)(fadeLevel * 255.0f)));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);

    // CAudioRelay is a function-local static whose destructor would otherwise
    // run after SDL_Quit() has torn the audio subsystem down.
    ClimaxEngine::Audio::CAudioRelay::GetInstance().Shutdown();

    IMG_Quit();
    SDL_Quit();
    return 0;
}

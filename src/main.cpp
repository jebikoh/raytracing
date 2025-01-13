#include <SDL.h>

#include "bvh.hpp"
#include "display.hpp"
#include "camera.hpp"
#include "rt.hpp"
#include "scene.hpp"

#include <thread>

// Camera Settings
constexpr int IMAGE_WIDTH    = 800;
constexpr int IMAGE_HEIGHT   = 450;
constexpr int SAMPLES_PER_PX = 200;
constexpr int MAX_DEPTH      = 10;

int main(int argc, char *argv[]) {
    Scene scene;

    std::string path = "../src/assets/f22.obj";
    auto t = jtx::rotateX(25);
    auto m = Material{.type = Material::METAL, .albedo = Color(0.8, 0.6, 0.2), .fuzz = 0.25};
    auto background = Color(0.7, 0.8, 1.0);

    createObjScene(scene, path, t, m, background);

    Camera camera{
            IMAGE_WIDTH,
            IMAGE_HEIGHT,
            scene.cameraProperties,
            SAMPLES_PER_PX,
            MAX_DEPTH};

    Display display(IMAGE_WIDTH + SIDEBAR_WIDTH, IMAGE_HEIGHT, &camera);
    if (!display.init()) {
        return -1;
    }

    BVHTree world(scene, 1);

    display.setWorld(&world);

    bool isRunning = true;
    while (isRunning) {
        display.processEvents(isRunning);
        display.render();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    while (display.isRendering()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    world.destroy();
    display.destroy();
    scene.destroy();
    return 0;
}

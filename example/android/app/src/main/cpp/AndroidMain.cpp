#include <plog/Log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include "renderer.hpp"

X::Renderer g_renderer;
android_app* g_androidAppCtx = nullptr;

bool InitVulkan(android_app* app)
{
    g_androidAppCtx = app;

    if (!g_renderer.Init()) {
        PLOGW.printf("Gaussian Splatting Renderer: Vulkan is unavailable, install vulkan and re-start");
        return false;
    }
    return false;
}

void handle_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            InitVulkan(app);
            break;
        case APP_CMD_TERM_WINDOW:
            g_renderer.Destroy();
            break;
        default:
            PLOGI.printf("Gaussian Splatting Renderer", "event not handled: %d", cmd);
    }
}

void android_main(struct android_app* app) {

    // Set the callback to process system events
    app->onAppCmd = handle_cmd;

    // Used to poll the events in the main loop
    int events;
    android_poll_source* source;

    // Main loop
    do {
        if (ALooper_pollAll(g_renderer.IsReady() ? 1 : 0, nullptr, &events, (void**)&source) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }
        }

        // render if vulkan is ready
        if (g_renderer.IsReady()) {
            g_renderer.DrawFrame();
        }
    } while (app->destroyRequested == 0);
}

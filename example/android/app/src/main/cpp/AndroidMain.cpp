#include "AndroidMain.hpp"

#include "common/LogCommon.hpp"

#include "AndroidWindow.hpp"

android_app* g_androidAppCtx = nullptr;
plog::AndroidAppender<plog::TxtFormatter> androidAppender("3DGS_Vulkan");

void HandleAppCommand(android_app* app, int32_t cmd)
{
    assert(app->userData != nullptr);
    AndroidWindow* window = reinterpret_cast<AndroidWindow*>(app->userData);
    window->HandleAppCommand(cmd);
}

void android_main(android_app* app) {
    INIT_LOGGER(plog::debug, &androidAppender);
    AndroidWindow* window = new AndroidWindow();
    app->userData = window;
    app->onAppCmd = HandleAppCommand;
    g_androidAppCtx = app;

    // window->Init(g_androidAppCtx);
    window->RenderLoop();

    delete window;
}

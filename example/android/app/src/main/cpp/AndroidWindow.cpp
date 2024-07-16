#include "AndroidWindow.hpp"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"
#include "common/LogCommon.hpp"

#include "scenes/Splat.hpp"

#include "common/Constants.hpp"
#include "common/KeycodesCommon.hpp"

void AndroidWindow::InitBackend()
{
    if (!X::Backend::VkContext::GetInstance().IsReady()) {
        X::Backend::VkContext::GetInstance().Init();
    }
    X::Backend::VkResourceManager::GetInstance();
}

void AndroidWindow::InitSurface(ANativeWindow* window)
{
    surface_ = X::Backend::DisplaySurface::Make(window);
    surface_->SetupSwapchain();
    surface_->SetupSwapSurfaces();
}

bool AndroidWindow::Init(android_app* app)
{
    GetDeviceConfig();
    InitBackend();
    InitSurface(app->window);
    renderer_ = std::make_unique<X::GaussianRenderer>();
    if (!renderer_->Init(surface_.get())) {
        XLOGW("Gaussian Splatting Renderer: Vulkan is unavailable, install vulkan and restart");
        return false;
    }
    LoadScene();
    return true;
}

void AndroidWindow::LoadScene()
{
    scene_ = std::make_unique<X::Scene>();
    // TODO: use gui callback to load splat file
    auto splat = X::Splat::MakeUnique(GetAssetPath() + "/train_7000.ply");
    scene_->AddObject(std::move(splat));
    scene_->GetCamera().SetPerspective(50.154269299972504, surface_->GetWidth() * 1164.6601287484507 / 1159.5880733038064 / surface_->GetHeight(), 0.2f, 200.0f);
    scene_->GetCamera().SetPosition({-3.0089893469241797, -0.11086489695181866, -3.7527640949141428});
    scene_->GetCamera().SetRotation({ 2.5534724, 28.6107985, -3.4906808 });
    // scene_->GetCamera().SetRotation({ 3.756929, 28.4939513, -4.5199111 });
    scene_->InitGPUData();
}

void AndroidWindow::HandleAppCommand(int32_t cmd)
{
    switch (cmd)
    {
    case APP_CMD_SAVE_STATE:
        XLOGD("APP_CMD_SAVE_STATE");
        /*
        window->app->savedState = malloc(sizeof(struct saved_state));
        *((struct saved_state*)window->app->savedState) = window->state;
        window->app->savedStateSize = sizeof(struct saved_state);
        */
        break;
    case APP_CMD_INIT_WINDOW:
        XLOGD("APP_CMD_INIT_WINDOW");
        if (g_androidAppCtx->window != nullptr) {
            if (!Init(g_androidAppCtx)) {
                XLOGE("Could not initialize window, exiting!");
                g_androidAppCtx->destroyRequested = 1;
            }
        } else {
            XLOGE("No window assigned!");
        }
        break;
    case APP_CMD_LOST_FOCUS:
        XLOGD("APP_CMD_LOST_FOCUS");
        focused_ = false;
        break;
    case APP_CMD_GAINED_FOCUS:
        XLOGD("APP_CMD_GAINED_FOCUS");
        focused_ = true;
        break;
    case APP_CMD_TERM_WINDOW:
        // Window is hidden or closed, clean up resources
        XLOGD("APP_CMD_TERM_WINDOW");
        break;
    }
}

void AndroidWindow::HandleAppInputs()
{
    android_input_buffer inputBuffer = g_androidAppCtx->inputBuffers[g_androidAppCtx->currentInputBuffer];
    for (int32_t i = 0; i < inputBuffer.keyEventsCount; i++) {
        GameActivityKeyEvent* event = inputBuffer.keyEvents + i;
        int32_t keyCode = event->keyCode;
        int32_t action = event->action;

        if (action == AKEY_EVENT_ACTION_UP) {
            return;
        }

        switch (keyCode) {
            case AKEYCODE_1:
            case AKEYCODE_F1:
            case AKEYCODE_BUTTON_L1:
                scene_->ChangeOverlayState();
                break;
            default:
                KeyPressed(keyCode);
                break;
        };

        XLOGD("Button %d pressed", keyCode);
    }

    for (int32_t i = 0; i < inputBuffer.motionEventsCount; i++) {
        GameActivityMotionEvent* event = inputBuffer.motionEvents + i;
        int32_t eventSource = event->source;
        switch (eventSource) {
            case AINPUT_SOURCE_JOYSTICK: {
                UpdateGamePad(event);
                break;
            }

            case AINPUT_SOURCE_TOUCHSCREEN: {
                int32_t action = event->action;

                switch (action) {
                    case AMOTION_EVENT_ACTION_UP: {
                        lastTapTime_ = event->eventTime;
                        touchPos_.x_ = event->precisionX;
                        touchPos_.y_ = event->precisionY;
                        touchTime_ = 0.0;
                        touchDown_ = false;
                        scene_->GetCamera().keys_.up = false;

                        // Detect single tap
                        int64_t eventTime = event->eventTime;
                        int64_t downTime = event->downTime;
                        if (eventTime - downTime <= Android::TAP_TIMEOUT) {
                            float deadZone = (160.f / screenDensity_) * Android::TAP_SLOP * Android::TAP_SLOP;
                            float x = event->precisionX - touchPos_.x_;
                            float y = event->precisionY - touchPos_.y_;
                            // if ((x * x + y * y) < deadZone) {
                            //     mouseState.buttons.left = true;
                            // }
                        };

                        return;
                        break;
                    }
                    case AMOTION_EVENT_ACTION_DOWN: {
                        // Detect double tap
                        int64_t eventTime = event->eventTime;
                        if (eventTime - lastTapTime_ <= Android::DOUBLE_TAP_TIMEOUT) {
                            float deadZone = (160.f / screenDensity_) * Android::DOUBLE_TAP_SLOP * Android::DOUBLE_TAP_SLOP;
                            float x = event->precisionX - touchPos_.x_;
                            float y = event->precisionY - touchPos_.y_;
                            if ((x * x + y * y) < deadZone) {
                                KeyPressed(TOUCH_DOUBLE_TAP);
                                touchDown_ = false;
                            }
                        } else {
                            touchDown_ = true;
                        }
                        touchPos_.x_ = event->precisionX;
                        touchPos_.y_ = event->precisionY;
                        // mouseState.position.x = AMotionEvent_getX(event, 0);
                        // mouseState.position.y = AMotionEvent_getY(event, 0);
                        break;
                    }
                    case AMOTION_EVENT_ACTION_MOVE: {
                        bool handled = false;
                        ImGuiIO& io = ImGui::GetIO();
                        handled = io.WantCaptureMouse && scene_->OverlayVisible();
                        if (!handled) {
                            int32_t eventX = event->precisionX;
                            int32_t eventY = event->precisionY;

                            float deltaX = (float)(touchPos_.y_ - eventY) * scene_->GetCamera().rotationSpeed_ * 0.5f;
                            float deltaY = (float)(touchPos_.x_ - eventX) * scene_->GetCamera().rotationSpeed_ * 0.5f;

                            scene_->GetCamera().Rotate(glm::vec3(deltaX, 0.0f, 0.0f));
                            scene_->GetCamera().Rotate(glm::vec3(0.0f, -deltaY, 0.0f));

                            touchPos_.x_ = eventX;
                            touchPos_.y_ = eventY;
                        }
                        break;
                    }
                    default:
                        return;
                        break;
                }
            }

            return;
        }
    }
    return;
}

void AndroidWindow::UpdateGamePad(GameActivityMotionEvent* event)
{
    // Left thumbstick
    gamePadState_.axisLeft_.x = GameActivityMotionEvent_getHistoricalAxisValue(event, AMOTION_EVENT_AXIS_X, 0, 0);
    gamePadState_.axisLeft_.y = GameActivityMotionEvent_getHistoricalAxisValue(event, AMOTION_EVENT_AXIS_Y, 0, 0);
    // Right thumbstick
    gamePadState_.axisRight_.x = GameActivityMotionEvent_getHistoricalAxisValue(event, AMOTION_EVENT_AXIS_Z, 0, 0);
    gamePadState_.axisRight_.y = GameActivityMotionEvent_getHistoricalAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0, 0);
}

void AndroidWindow::RenderLoop()
{
    bool destroyRequested = false;
    while (!destroyRequested) {
        int ident;
        int events;
        android_poll_source* source;
        focused_ = true;

        while ((ident = ALooper_pollAll(focused_ ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
            if (source != nullptr) {
                source->process(g_androidAppCtx, source);
            }
            if (g_androidAppCtx->destroyRequested != 0) {
                XLOGD("Android app destroy requested");
                destroyRequested = true;
                break;
            }
        }

        // App destruction requested
        // Exit loop, window will be destroyed in application main
        if (destroyRequested) {
            GameActivity_finish(g_androidAppCtx->activity);
            break;
        }

        this->HandleAppInputs();

        // Render frame
        if (renderer_ && renderer_->IsReady() && scene_ && surface_) {
            auto current = std::chrono::high_resolution_clock::now();
            auto frameTime = std::chrono::duration<double, std::milli>(current - frameStart_).count() / 1000.0f;
            frameStart_ = current;

            scene_->GetCamera().Update(frameTime);
            renderer_->UpdateScene(scene_.get());
            renderer_->DrawFrame();

            // Check touch state (for movement)
            if (touchDown_) {
                touchTime_ += frameTime;
            }
            if (touchTime_ >= 1.0) {
                scene_->GetCamera().keys_.up = true;
            }

            // Check gamepad state
            const float deadZone = 0.0015f;
            // todo : check if gamepad is present
            // todo : time based and relative axis positions
            if (scene_->GetCameraType() != X::Camera::CameraType::FirstPerson) {
                // Rotate
                if (std::abs(gamePadState_.axisLeft_.x) > deadZone) {
                    scene_->GetCamera().Rotate(glm::vec3(0.0f, gamePadState_.axisLeft_.x * 0.5f, 0.0f));
                }
                if (std::abs(gamePadState_.axisLeft_.y) > deadZone) {
                    scene_->GetCamera().Rotate(glm::vec3(gamePadState_.axisLeft_.y * 0.5f, 0.0f, 0.0f));
                }
                // Zoom
                if (std::abs(gamePadState_.axisRight_.y) > deadZone) {
                    scene_->GetCamera().Translate(glm::vec3(0.0f, 0.0f, gamePadState_.axisRight_.y * 0.01f));
                }
            } else {
                scene_->GetCamera().UpdatePad(gamePadState_.axisLeft_, gamePadState_.axisRight_, frameTime);
            }
        }
    }
}

void AndroidWindow::GetDeviceConfig()
{
    // Screen density
    AConfiguration* config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, g_androidAppCtx->activity->assetManager);
    screenDensity_ = AConfiguration_getDensity(config);
    AConfiguration_delete(config);
}
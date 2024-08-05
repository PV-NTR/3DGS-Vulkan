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
//    auto splat = X::Splat::MakeUnique(GetAssetPath() + "/bonsai-7k-mini.ply");
//    auto splat = X::Splat::MakeUnique(GetAssetPath() + "/demo_fox_gs.ply");
    scene_->AddObject(std::move(splat));
    // Camera of train
    scene_->GetCamera().SetPerspective(50.154269299972504, surface_->GetWidth() * 1164.6601287484507 / 1159.5880733038064 / surface_->GetHeight(), 0.2f, 200.0f);
    scene_->GetCamera().SetPosition({-3.0089893469241797, -0.11086489695181866, -3.7527640949141428});
    scene_->GetCamera().SetRotation({ 3.756929, 28.4939513, -4.5199111 });

    // Camera of bonsai
//    scene_->GetCamera().SetPerspective(80.154269299972504, float(surface_->GetWidth()) / surface_->GetHeight(), 0.2f, 200.0f);
//    scene_->GetCamera().SetPosition({3.7212285514226, -1.9830705231664232, 0.2941856450880261});
//    scene_->GetCamera().SetRotation({-14.5064958, -58.9494315, -27.6427182});

    // Camera of fox
//    scene_->GetCamera().SetPerspective(50.154269299972504, float(surface_->GetWidth()) / surface_->GetHeight(), 0.2f, 200.0f);
//    scene_->GetCamera().SetPosition({ -1.86899006, 1.84913456, -3.75276423 });
//    scene_->GetCamera().SetRotation({ 15.7569275, -10.5060425, -4.51991129 });

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
        if (surface_->IsReady()) {
            surface_->CleanSwapchain();
        }
        break;
    }
}

void AndroidWindow::HandleMotionEvent(GameActivityMotionEvent* motionEvent)
{
    if (motionEvent->pointerCount > 0) {
        int eventSource = motionEvent->source;
        int action = motionEvent->action;
        int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        int ptrIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        if (ptrIndex < motionEvent->pointerCount) {
            if (eventSource == AINPUT_SOURCE_TOUCHSCREEN) {
                switch (action) {
                    case AMOTION_EVENT_ACTION_UP: {
                        lastTapTime_ = motionEvent->eventTime;
                        touchPos_.x_ = GameActivityPointerAxes_getX(&motionEvent->pointers[ptrIndex]);
                        touchPos_.y_ = GameActivityPointerAxes_getY(&motionEvent->pointers[ptrIndex]);
                        touchTime_ = 0.0;
                        touchDown_ = false;
                        scene_->GetCamera().keys_.up = false;

                        // Detect single tap
                        int64_t eventTime = motionEvent->eventTime;
                        int64_t downTime = motionEvent->downTime;
                        if (eventTime - downTime <= Android::TAP_TIMEOUT) {
                            float deadZone = (160.f / screenDensity_) * Android::TAP_SLOP * Android::TAP_SLOP;
                            float x = motionEvent->precisionX - touchPos_.x_;
                            float y = motionEvent->precisionY - touchPos_.y_;
                            // if ((x * x + y * y) < deadZone) {
                            //     mouseState.buttons.left = true;
                            // }
                        };
                        break;
                    }
                    case AMOTION_EVENT_ACTION_DOWN: {
                        // Detect double tap
                        int64_t eventTime = motionEvent->eventTime;
                        if (eventTime - lastTapTime_ <= Android::DOUBLE_TAP_TIMEOUT) {
                            float deadZone = (160.f / screenDensity_) * Android::DOUBLE_TAP_SLOP * Android::DOUBLE_TAP_SLOP;
                            float x = GameActivityPointerAxes_getX(&motionEvent->pointers[ptrIndex]) - touchPos_.x_;
                            float y = GameActivityPointerAxes_getY(&motionEvent->pointers[ptrIndex]) - touchPos_.y_;
                            if ((x * x + y * y) < deadZone) {
                                KeyPressed(TOUCH_DOUBLE_TAP);
                                touchDown_ = false;
                            }
                        } else {
                            touchDown_ = true;
                        }
                        touchPos_.x_ = motionEvent->precisionX;
                        touchPos_.y_ = motionEvent->precisionY;
                        break;
                    }
                    case AMOTION_EVENT_ACTION_MOVE: {
                        bool handled = false;
//                    ImGuiIO& io = ImGui::GetIO();
//                    handled = io.WantCaptureMouse && scene_->OverlayVisible();
                        if (!handled) {
                            int32_t eventX = GameActivityPointerAxes_getX(&motionEvent->pointers[ptrIndex]);
                            int32_t eventY = GameActivityPointerAxes_getY(&motionEvent->pointers[ptrIndex]);

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
                        break;
                }
            }
        }
    }
}

void AndroidWindow::HandleAppInputs()
{
    android_input_buffer* inputBuffer = android_app_swap_input_buffers(g_androidAppCtx);
    // TODO: add key event
//    for (int32_t i = 0; i < inputBuffer.keyEventsCount; i++) {
//        XLOGD("Button %d pressed", keyCode);
//    }
    if (inputBuffer) {
        for (int32_t i = 0; i < inputBuffer->motionEventsCount; i++) {
            GameActivityMotionEvent* motionEvent = &inputBuffer->motionEvents[i];
            HandleMotionEvent(motionEvent);
        }
        android_app_clear_motion_events(inputBuffer);
    }
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
        if (renderer_ && renderer_->IsReady() && scene_ && surface_ && surface_->IsReady()) {
            auto current = std::chrono::high_resolution_clock::now();
            auto frameTime = std::chrono::duration<double, std::milli>(current - frameStart_).count() / 1000.0f;
            frameStart_ = current;

            renderer_->UpdateScene(scene_.get());
            auto updatedTimeStamp = std::chrono::high_resolution_clock::now();
            XLOGI("Update time: %f ms", std::chrono::duration<double, std::milli>((updatedTimeStamp - frameStart_).count()) / 1e6);

            renderer_->DrawFrame();
            auto drawnTimeStamp = std::chrono::high_resolution_clock::now();
            XLOGI("Draw time: %f ms", std::chrono::duration<double, std::milli>((drawnTimeStamp - updatedTimeStamp).count()) / 1e6);

            // reset camera update state to false, if moving, update it
            scene_->GetCamera().Update(frameTime);

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
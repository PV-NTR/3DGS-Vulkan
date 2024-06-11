#include "VulkanWindow.hpp"

#include "common/KeycodesCommon.hpp"
#include "imgui.h"

#include "resources/vk/VkContext.hpp"
#include "resources/vk/VkResourceManager.hpp"

void VulkanWindow::HandleMouseMove(int32_t x, int32_t y)
{
    int32_t dx = (int32_t)mouseState_.position_.x - x;
    int32_t dy = (int32_t)mouseState_.position_.y - y;

    bool handled = false;

    if (scene_->OverlayVisible()) {
        ImGuiIO& io = ImGui::GetIO();
        handled = io.WantCaptureMouse;
    }

    if (handled) {
        mouseState_.position_ = glm::vec2((float)x, (float)y);
        return;
    }

    if (mouseState_.buttons_.left) {
        scene_->GetCamera().Rotate(glm::vec3(dy * scene_->GetCamera().GetRotationSpeed(), -dx * scene_->GetCamera().GetRotationSpeed(), 0.0f));
    }
    if (mouseState_.buttons_.right) {
        scene_->GetCamera().Translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
    }
    if (mouseState_.buttons_.middle) {
        scene_->GetCamera().Translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
    }
    mouseState_.position_ = glm::vec2((float)x, (float)y);
}

void VulkanWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CLOSE:
            renderer_->Destroy();
            DestroyWindow(hWnd_);
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            ValidateRect(hWnd_, NULL);
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case KEY_F1:
                    scene_->ChangeOverlayState();
                    break;
                case KEY_F2:
                    scene_->ChangeCameraType();
                    break;
                case KEY_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }

            if (scene_->GetCameraType() == X::Camera::CameraType::FirstPerson) {
                scene_->UpdateCameraState();
                switch (wParam) {
                    case KEY_W:
                        scene_->GetCamera().keys_.up = true;
                        break;
                    case KEY_S:
                        scene_->GetCamera().keys_.down = true;
                        break;
                    case KEY_A:
                        scene_->GetCamera().keys_.left = true;
                        break;
                    case KEY_D:
                        scene_->GetCamera().keys_.right = true;
                        break;
                }
            }

            break;
        case WM_KEYUP:
            if (scene_->GetCameraType() == X::Camera::CameraType::FirstPerson) {
                switch (wParam) {
                    case KEY_W:
                        scene_->GetCamera().keys_.up = false;
                        break;
                    case KEY_S:
                        scene_->GetCamera().keys_.down = false;
                        break;
                    case KEY_A:
                        scene_->GetCamera().keys_.left = false;
                        break;
                    case KEY_D:
                        scene_->GetCamera().keys_.right = false;
                        break;
                }
            }
            break;
        case WM_LBUTTONDOWN:
            mouseState_.position_ = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
            mouseState_.buttons_.left = true;
            break;
        case WM_RBUTTONDOWN:
            mouseState_.position_ = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
            mouseState_.buttons_.right = true;
            break;
        case WM_MBUTTONDOWN:
            mouseState_.position_ = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
            mouseState_.buttons_.middle = true;
            break;
        case WM_LBUTTONUP:
            mouseState_.buttons_.left = false;
            break;
        case WM_RBUTTONUP:
            mouseState_.buttons_.right = false;
            break;
        case WM_MBUTTONUP:
            mouseState_.buttons_.middle = false;
            break;
        case WM_MOUSEWHEEL:
        {
            short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            scene_->GetCamera().Translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f));
            break;
        }
        case WM_MOUSEMOVE:
        {
            HandleMouseMove(LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case WM_SIZE:
            if ((renderer_->IsReady()) && (wParam != SIZE_MINIMIZED))
            {
                if ((resizing_) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
                {
                    width_ = LOWORD(lParam);
                    height_ = HIWORD(lParam);
                    WindowResize();
                }
            }
            break;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
            minMaxInfo->ptMinTrackSize.x = 64;
            minMaxInfo->ptMinTrackSize.y = 64;
            break;
        }
        case WM_ENTERSIZEMOVE:
            resizing_ = true;
            break;
        case WM_EXITSIZEMOVE:
            resizing_ = false;
            break;
    }

    OnHandleMessage(uMsg, wParam, lParam);
}

void VulkanWindow::InitBackend()
{
    X::Backend::VkContext::GetInstance().Init();
    X::Backend::VkResourceManager::GetInstance();
}

void VulkanWindow::InitRenderer()
{
    renderer_ = std::make_unique<X::Renderer>();
    renderer_->Init(surface_.get());
}

void VulkanWindow::InitSurface()
{
    surface_ = X::Backend::Surface::Make(hInstance_, hWnd_);
}

void VulkanWindow::LoadScene()
{

}

void VulkanWindow::RenderLoop()
{
    MSG msg;
    bool quitMessageReceived = false;
    while (!quitMessageReceived) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                quitMessageReceived = true;
                break;
            }
        }
        if (scene_->SceneChanged()) {
            renderer_->UpdateScene(scene_.get());
        }
        renderer_->DrawFrame();
    }
}
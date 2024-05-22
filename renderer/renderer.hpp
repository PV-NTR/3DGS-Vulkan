#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool IsReady();
    bool Init();
    void Destroy();
    void DrawFrame();
private:
    /* data */
};

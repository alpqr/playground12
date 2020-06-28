#include "app.h"

void recordFrame(App *app)
{
    float c[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    app->dev.context->ClearRenderTargetView(app->swapchain.rtv, c);
}

void handleEvent(const Event &e)
{
    log("type=%d x=%d y=%d key=%d ch=%d mods=%x repeat=%d", e.type, e.x, e.y, e.key, e.ch, e.modifiers, e.repeat);
}

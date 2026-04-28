#pragma once

#include <Dodo.h>

using namespace Dodo;

class DebugLayer : public Layer {
  public:
    void Update(float elapsed) {}
    void Render(RenderAPI& renderAPI, AssetManager& assets);
    void OnEvent(const Event& event) {}
};

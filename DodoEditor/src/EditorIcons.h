#pragma once

#include <Dodo.h>
#include <vector>

struct EditorIconSet {
    void* entity     = nullptr;
    void* entitySel  = nullptr;
    void* entityRoot = nullptr;
    bool  ready      = false;
};

class EditorIcons {
  public:
    void Load(Dodo::RenderAPI& api);
    const EditorIconSet& Get() const { return m_Icons; }

  private:
    static std::vector<unsigned char> RasterizeSVG(const char* path, int size);

    Ref<Dodo::Texture> m_EntityTex;
    Ref<Dodo::Texture> m_EntitySelTex;
    Ref<Dodo::Texture> m_EntityRootTex;
    EditorIconSet m_Icons;
};

#pragma once

#include <Dodo.h>
#include <vector>

struct EditorIconSet {
    void* entity     = nullptr;
    void* entitySel  = nullptr;
    void* entityRoot = nullptr;
    void* folder     = nullptr;
    void* texture    = nullptr;
    void* model      = nullptr;
    void* shader     = nullptr;
    void* scene      = nullptr;
    void* file       = nullptr;
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
    Ref<Dodo::Texture> m_FolderTex;
    Ref<Dodo::Texture> m_TextureTex;
    Ref<Dodo::Texture> m_ModelTex;
    Ref<Dodo::Texture> m_ShaderTex;
    Ref<Dodo::Texture> m_SceneTex;
    Ref<Dodo::Texture> m_FileTex;
    EditorIconSet m_Icons;
};

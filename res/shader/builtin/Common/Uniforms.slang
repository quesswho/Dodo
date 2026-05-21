// Uniforms.slang

// Frame-level uniforms (uploaded once per frame)
[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)
{
    float4x4 u_Camera;
    float4x4 u_CameraView;   // view matrix only (no projection), for cascade depth selection
    float4x4 u_SkyboxCamera;
    float4x4 u_LightCamera;
    float3 u_LightDir;
    float u_FramePadding0;
    float3 u_CameraPos;
    float u_FramePadding1;
};

// Per-object uniforms (uploaded per draw)
[[vk::binding(0, 2)]] cbuffer ModelData : register(b2)
{
    float4x4 u_Model;
    float4x4 u_NormalMatrix;
    uint u_AlbedoIdx;    // index into g_Textures[] (slot 0 = 1x1 white fallback)
    uint u_RoughIdx;
    uint u_NormalIdx;
    uint u_MetallicIdx;
    uint u_AoIdx;
    uint u_SpecIdx;
    uint u_SamplerIdx;   // index into g_Samplers[]
    uint u_ModelPad;
};

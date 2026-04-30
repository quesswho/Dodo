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
};

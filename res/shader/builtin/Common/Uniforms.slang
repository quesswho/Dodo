// Uniforms.slang

// Frame-level uniforms (uploaded once per frame)
[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)
{
    float4x4 u_Camera;
    float4x4 u_SkyboxCamera;
    float4x4 u_LightCamera;
    float3 u_LightDir;
    float u_FramePadding0;
    float3 u_CameraPos;
    float u_FramePadding1;
};

// Per-draw uniforms (uploaded per mesh)
[[vk::binding(1, 0)]] cbuffer ModelData : register(b1)
{
    float4x4 u_Model;
    float4x4 u_NormalMatrix;
};

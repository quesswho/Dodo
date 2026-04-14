[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)
{
    float4x4 u_Camera;
    float4x4 u_LightCamera;
    float3 u_LightDir;
    float u_FramePadding0;
    float3 u_CameraPos;
    float u_FramePadding1;
};
[[vk::binding(1, 0)]] cbuffer ModelData : register(b1)
{
    float4x4 u_Model;
    float4x4 u_NormalMatrix;
};

struct VertexInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.texcoord = input.texcoord;
    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

float checker(float2 uv)
{
    float cx = floor(20.0f * uv.x);
    float cy = floor(20.0f * uv.y);
    return sign(fmod(cx + cy, 2.0f));
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float res = lerp(1.0f, 0.0f, checker(input.texcoord));
    return float4(res, res * 0.1f, res * 0.1f, 1.0f);
}
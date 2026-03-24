cbuffer PushConstants : register(b2)
{
    float gamma;
    float3 _pushPadding;
};

Texture2D<float4> u_TextureMap : register(t0);
SamplerState u_TextureSampler : register(s0);

struct VertexInput
{
    float2 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.texCoord = input.texCoord;
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float3 color = u_TextureMap.Sample(u_TextureSampler, input.texCoord).rgb;
    float3 result = pow(color / (color + float3(1.0f, 1.0f, 1.0f)), float3(1.0f / gamma));
    return float4(result, 1.0f);
}

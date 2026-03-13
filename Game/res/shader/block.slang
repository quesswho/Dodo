Texture2D colorTexture;
SamplerState texSampler;

uniform float4x4 u_Model;
uniform float4x4 u_Camera;

struct VertexInput {
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal   : NORMAL;
};

struct VertexOutput {
    float4 position  : SV_Position;
    float3 fragPos   : TEXCOORD1;
    float2 texCoord  : TEXCOORD0;
    float3 normal    : NORMAL;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.fragPos  = mul(u_Model, float4(input.position, 1.0f)).xyz;
    output.texCoord = input.texcoord;
    output.normal   = input.normal;
    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float3 color = colorTexture.Sample(texSampler, input.texCoord).xyz;
    float diff = max(dot(normalize(float3(0.1, 0.8, 0.2)), input.normal), 0.0f);
    float3 diffuse = color * diff;
    return float4(color * 0.8 + diffuse, 1.0f);
}
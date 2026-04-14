// VertexInferface.slang

struct VertexInput
{
    float3 position;
    float2 texCoord;
    float3 normal;
    float3 tangent;
};

// Output from vertex shader
struct VertexOutput
{
    float4 positionSS : SV_Position;
    float3 positionWS;
    float2 texCoord;
    float3 normalWS;   // world-space normal (for TBN reconstruction)
    float3 tangentWS;  // world-space tangent (for TBN reconstruction)
    float4 positionLL; // light-space position for shadow mapping
};
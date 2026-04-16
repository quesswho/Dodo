// VertexInferface.slang

struct VertexInput
{
    float3 position;
    float2 texCoord;
    float3 normal;
    float4 tangent; // xyz = tangent direction, w = bitangent sign (+1 or -1)
};

// Output from vertex shader
struct VertexOutput
{
    float4 positionSS : SV_Position;
    float3 positionWS;
    float2 texCoord;
    float3 normalWS;   // world-space normal (for TBN reconstruction)
    float4 tangentWS;  // world-space tangent xyz, w = bitangent sign (for TBN reconstruction)
    float4 positionLL; // light-space position for shadow mapping
};
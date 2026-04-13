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
    float3 lightDirection;
    float3 tangentCameraPos;
    float3 tangentFragPos;
    float3 cameraPos;
    float3 normalWS;
    float4 positionLL;
};
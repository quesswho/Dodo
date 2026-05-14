// CubemapConvolution.slang
// Convolves an environment cubemap over a hemisphere to produce a diffuse irradiance map.
// For each output texel (direction N), integrates incoming radiance over the upper hemisphere
// using a Riemann sum: irradiance = PI * sum(L * cos(theta) * sin(theta)) / nrSamples.
#include "../Common/Uniforms.slang"

[[vk::binding(0, 1)]] TextureCube<float4> u_EnvMap;
[[vk::binding(1, 1)]] SamplerState u_EnvSampler;

static const float PI = 3.14159265359f;

struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 localPos : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.localPos = input.position;
    output.position = mul(u_Camera, mul(u_Model, float4(input.position, 1.0f)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float3 N = normalize(input.localPos);

    float3 up    = abs(N.y) < 0.999f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 right = normalize(cross(up, N));
    up           = cross(N, right);

    float3 irradiance = float3(0.0f);
    float  nrSamples  = 0.0f;

    float sampleDelta = 0.025f;
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            float3 tangentSample = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
            float3 sampleVec     = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += u_EnvMap.Sample(u_EnvSampler, sampleVec).rgb * cosTheta * sinTheta;
            nrSamples  += 1.0f;
        }
    }

    irradiance = PI * irradiance / nrSamples;
    return float4(irradiance, 1.0f);
}

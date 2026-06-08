// ShadowSampling.slang
// PCF and PCSS sampling from a Texture2DArray cascade shadow map.
// Cascade selection is based on view-space depth of the fragment.

#include "../Common/CsmUniforms.slang"

// Distributes sampleIndex across a unit disk with repulsive point process
float2 vogelDiskSample(int sampleIndex, int sampleCount, float phi)
{
    const float goldenAngle = 2.399963f;
    float r = sqrt((float(sampleIndex) + 0.5f) / float(sampleCount));
    float theta = float(sampleIndex) * goldenAngle + phi;
    return float2(cos(theta), sin(theta)) * r;
}

#include "../Common/Debug.slang"

float3 debugCSMSample(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth,
    float lightSize)
{
    int cascadeIndex = numCascades - 1;
    for (int i = 0; i < numCascades - 1; i++)
    {
        if (viewDepth < u_CascadeSplitDepths[i])
        {
            cascadeIndex = i;
            break;
        }
    }
    return debugFloat(cascadeIndex/float(numCascades-1));
}

float shadowCalculationCSM(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth,
    float lightSize)
{
    // Select the tightest cascade that covers this fragment's depth
    int cascadeIndex = numCascades - 1;
    for (int i = 0; i < numCascades - 1; i++)
    {
        if (viewDepth < u_CascadeSplitDepths[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float4 lightFragPos = mul(u_LightSpaceMatrices[cascadeIndex], float4(positionWS, 1.0f));

    // Perspective divide
    float3 projCoords = lightFragPos.xyz / lightFragPos.w;

    // Outside shadow frustum = no shadow
    if (projCoords.z > 1.0f)
        return 0.0f;

    // Transform to [0,1] UV range
    projCoords.xy = projCoords.xy * 0.5f + 0.5f;

    float currentDepth = projCoords.z;

    // Slope-scaled bias (scale down for far cascades where the texel footprint is large)
    float bias = max(0.005f * (1.0f - dot(normal, lightDirection)), 0.001f);
    bias /= (u_CascadeSplitDepths[cascadeIndex] * 0.5f + 0.5f);

    uint w, h, layers;
    shadowMap.GetDimensions(w, h, layers);
    if (w == 0 || h == 0)
        return 0.0f;

    float2 texelSize = 1.0f / float2(w, h);

    float phi = frac(sin(dot(projCoords.xy, float2(1.0f, 1.0f))) * 1.2443f) * 6.28318f;
    const int pcfSamples = 25;
    float shadow = 0.0f;
    for (int i = 0; i < pcfSamples; i++)
    {
        float2 offset = vogelDiskSample(i, pcfSamples, phi) * texelSize;
        float occluderDepth = shadowMap.Sample(
            shadowSampler,
            float3(projCoords.xy + offset, float(cascadeIndex)));
        shadow += (currentDepth - bias > occluderDepth) ? 1.0f : 0.0f;
    }

    return shadow / (pcfSamples);
}

// PCSS: Soft shadows via blocker search + variable-radius PCF.
// lightSize controls the apparent angular size of the light in light-space texel units.
// Larger lightSize values produce wider, softer penumbras.
float shadowCalculationCSMPCSS(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth,
    float lightSize)
{
    int cascadeIndex = numCascades - 1;
    for (int i = 0; i < numCascades - 1; i++)
    {
        if (viewDepth < u_CascadeSplitDepths[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float4 lightFragPos = mul(u_LightSpaceMatrices[cascadeIndex], float4(positionWS, 1.0f));
    float3 projCoords = lightFragPos.xyz / lightFragPos.w;
    
    float currentDepth = projCoords.z;
    if (projCoords.z > 1.0f)
        return 0.0f;
    
    projCoords = projCoords * 0.5f + 0.5f;
    float bias = max(0.005f * (1.0f - dot(normal, lightDirection)), 0.001f);
    bias /= (u_CascadeSplitDepths[cascadeIndex] * 0.5f + 0.5f);

    uint w, h, layers;
    shadowMap.GetDimensions(w, h, layers);
    if (w == 0 || h == 0)
        return 0.0f;

    float2 texelSize = 1.0f / float2(w, h);

    // Per-fragment rotation angle to decorrelate the Vogel disk pattern across pixels
    const float phi = frac(sin(dot(projCoords.xy, float2(342.21f, 176.f))) * 12000.2443f) * 6.28318f;

    //const float phi = fract(52.9829189f * fract(0.06711056f * float(projCoords.x) + 0.00583715f * float(projCoords.y)));

    // Stage 1: Blocker search: find average depth of occluding samples
    const int blockerSamples = 16;
    float2 searchRadiusUV = lightSize * texelSize;
    float avgBlockerDepth = 0.0f;
    int blockerCount = 0;
    for (int i = 0; i < blockerSamples; i++)
    {
        float2 offset = vogelDiskSample(i, blockerSamples, phi) * searchRadiusUV;
        float occluderDepth = shadowMap.Sample(
            shadowSampler,
            float3(projCoords.xy + offset, float(cascadeIndex)));
        if (occluderDepth < currentDepth - bias)
        {
            avgBlockerDepth += occluderDepth;
            blockerCount++;
        }
    }

    if (blockerCount == 0)
        return 0.0f;

    avgBlockerDepth /= float(blockerCount);
    
    // Stage 2: Penumbra estimation - receiver further from blocker = wider penumbra
    float penumbraWidth = (currentDepth - avgBlockerDepth) / avgBlockerDepth * lightSize;
    float2 filterRadiusUV = max(penumbraWidth, 2.0f) * texelSize;

    // Stage 3: Variable-radius PCF using the estimated penumbra width
    const int pcfSamples = 64;
    float shadow = 0.0f;
    for (int i = 0; i < pcfSamples; i++)
    {
        float2 offset = vogelDiskSample(i, pcfSamples, phi) * filterRadiusUV;
        float occluderDepth = shadowMap.Sample(
            shadowSampler,
            float3(projCoords.xy + offset, float(cascadeIndex)));
        shadow += (currentDepth - bias > occluderDepth) ? 1.0f : 0.0f;
    }

    return shadow / (pcfSamples);
}

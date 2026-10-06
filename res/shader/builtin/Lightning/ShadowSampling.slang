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

// Smallest filter radius in shadow map texels. Anything tighter shows the texel grid as stair-steps.
static const float minFilterTexels = 2.0f;
// Largest filter and blocker search radius in shadow map texels, bounds how sparse the sample disk gets.
static const float maxFilterTexels = 24.0f;
// Steepest receiver slope (tan of the angle between normal and light) the depth bias compensates for.
static const float maxBiasSlope = 5.0f;

// A fragment projected into the cascade that covers it.
struct CascadeFragment
{
    float  layer;         // cascade index, as the array layer to sample
    float2 uv;            // shadow map UV
    float  depth;         // shadow map depth of the fragment
    float2 texelCount;    // shadow map resolution
    float  uvPerWorld;    // shadow map UV covered by one world unit
    float  depthPerWorld; // shadow map depth covered by one world unit
    float  slope;         // tan of the angle between the surface normal and the light
    bool   valid;         // false when the fragment is outside the shadow map
};

// Selects the tightest cascade that covers the given view-space depth.
int selectCascade(float viewDepth)
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
    return cascadeIndex;
}

CascadeFragment projectToCascade(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    float viewDepth)
{
    CascadeFragment frag;
    int cascadeIndex = selectCascade(viewDepth);
    float4x4 lightSpace = u_LightSpaceMatrices[cascadeIndex];

    float4 lightFragPos = mul(lightSpace, float4(positionWS, 1.0f));
    float3 projCoords = lightFragPos.xyz / lightFragPos.w;

    frag.layer = float(cascadeIndex);
    // Transform to [0,1] UV range (V points down, clip-space Y points up)
    frag.uv = projCoords.xy * float2(0.5f, -0.5f) + 0.5f;
    frag.depth = projCoords.z;

    // The cascade projection is orthographic over a rotation-only light view, so its rows are the light axes
    // scaled by the size of the cascade. That gives the world size of a texel and of a depth step, which keeps
    // the filter width and the bias the same in world space for every cascade.
    frag.uvPerWorld = 0.5f * length(lightSpace[0].xyz);
    frag.depthPerWorld = length(lightSpace[2].xyz);

    float NdotL = saturate(dot(normal, lightDirection));
    frag.slope = min(sqrt(1.0f - NdotL * NdotL) / max(NdotL, 1e-4f), maxBiasSlope);

    uint w, h, layers;
    shadowMap.GetDimensions(w, h, layers);
    frag.texelCount = float2(w, h);

    // Outside shadow frustum = no shadow
    frag.valid = projCoords.z <= 1.0f && w != 0 && h != 0;
    return frag;
}

// Depth the occluders are compared against when filtering over a disk of the given UV radius.
// A sample at the rim of the disk sees the receiver itself up to radius * slope closer to the light, so the bias
// has to grow with the filter. One extra texel covers the bilinear footprint and the depth quantization.
float biasedDepth(CascadeFragment frag, float radiusUV)
{
    float texelWorld = 1.0f / (frag.texelCount.x * frag.uvPerWorld);
    float radiusWorld = radiusUV / frag.uvPerWorld + texelWorld;
    return frag.depth - (texelWorld + radiusWorld * frag.slope) * frag.depthPerWorld;
}

// Depths of the 2x2 texels around uv, with the bilinear weights of uv inside that footprint.
// Order follows Gather: x = (0,1), y = (1,1), z = (1,0), w = (0,0).
float4 gatherDepths(
    CascadeFragment frag,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float2 uv,
    out float2 weights)
{
    float2 texelPos = uv * frag.texelCount - 0.5f;
    float2 base = floor(texelPos);
    weights = texelPos - base;
    // Gather at the corner the four texels share, so the footprint always matches the weights
    return shadowMap.Gather(shadowSampler, float3((base + 1.0f) / frag.texelCount, frag.layer));
}

// Bilinearly filtered depth comparison, 0 = lit and 1 = occluded.
// Filtering the comparison results instead of the depths gives a smooth edge that is not tied to the texel grid.
float sampleOcclusion(
    CascadeFragment frag,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float2 uv,
    float compareDepth)
{
    float2 weights;
    float4 depths = gatherDepths(frag, shadowMap, shadowSampler, uv, weights);
    float4 occluded = step(depths, float4(compareDepth));
    return lerp(lerp(occluded.w, occluded.z, weights.x), lerp(occluded.x, occluded.y, weights.x), weights.y);
}

// Averages the occlusion over a Vogel disk of the given UV radius.
float filterOcclusion(
    CascadeFragment frag,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float radiusUV,
    float phi,
    int sampleCount)
{
    float compareDepth = biasedDepth(frag, radiusUV);
    float shadow = 0.0f;
    for (int i = 0; i < sampleCount; i++)
    {
        float2 offset = vogelDiskSample(i, sampleCount, phi) * radiusUV;
        shadow += sampleOcclusion(frag, shadowMap, shadowSampler, frag.uv + offset, compareDepth);
    }
    return shadow / float(sampleCount);
}

// Per-fragment rotation angle to decorrelate the Vogel disk pattern across pixels
float diskRotation(float2 uv)
{
    return frac(sin(dot(uv, float2(143.2f, 364.7f))) * 60158.2443f) * 6.28318f;
}

float3 debugCSMSample(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth,
    float lightSize)
{
    int cascadeIndex = selectCascade(viewDepth);
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
    CascadeFragment frag = projectToCascade(positionWS, normal, lightDirection, shadowMap, viewDepth);
    if (!frag.valid)
        return 0.0f;

    const int pcfSamples = 25;
    float radiusUV = minFilterTexels / frag.texelCount.x;
    return filterOcclusion(frag, shadowMap, shadowSampler, radiusUV, diskRotation(frag.uv), pcfSamples);
}

// PCSS: Soft shadows via blocker search + variable-radius PCF.
// lightSize is the tangent of the angular radius of the light as seen from the scene: a blocker one world unit
// above its receiver casts a penumbra of that radius. Larger values produce wider, softer penumbras.
float shadowCalculationCSMPCSS(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth,
    float lightSize)
{
    CascadeFragment frag = projectToCascade(positionWS, normal, lightDirection, shadowMap, viewDepth);
    if (!frag.valid)
        return 0.0f;

    float texelUV = 1.0f / frag.texelCount.x;
    float minRadiusUV = minFilterTexels * texelUV;
    float maxRadiusUV = maxFilterTexels * texelUV;
    float phi = diskRotation(frag.uv);

    // Stage 1: Blocker search: find average depth of occluding samples.
    // A blocker can be at most as far away as the near plane of the cascade, which bounds the search radius.
    const int blockerSamples = 32;
    float searchRadiusUV =
        clamp(lightSize * frag.depth / frag.depthPerWorld * frag.uvPerWorld, minRadiusUV, maxRadiusUV);
    float blockerCompareDepth = biasedDepth(frag, searchRadiusUV);
    float avgBlockerDepth = 0.0f;
    float blockerCount = 0.0f;
    for (int i = 0; i < blockerSamples; i++)
    {
        float2 offset = vogelDiskSample(i, blockerSamples, phi) * searchRadiusUV;
        float2 weights;
        float4 depths = gatherDepths(frag, shadowMap, shadowSampler, frag.uv + offset, weights);
        float4 isBlocker = step(depths, float4(blockerCompareDepth));
        avgBlockerDepth += dot(depths, isBlocker);
        blockerCount += dot(isBlocker, float4(1.0f));
    }

    if (blockerCount == 0.0f)
        return 0.0f;

    avgBlockerDepth /= blockerCount;

    // Stage 2: Penumbra estimation - receiver further from blocker = wider penumbra.
    // The light is directional, so the penumbra grows linearly with the world distance to the blocker.
    float penumbraWorld = (frag.depth - avgBlockerDepth) / frag.depthPerWorld * lightSize;
    float filterRadiusUV = clamp(penumbraWorld * frag.uvPerWorld, minRadiusUV, searchRadiusUV);

    // Stage 3: Variable-radius PCF using the estimated penumbra width
    const int pcfSamples = 104;
    return filterOcclusion(frag, shadowMap, shadowSampler, filterRadiusUV, phi, pcfSamples);
}

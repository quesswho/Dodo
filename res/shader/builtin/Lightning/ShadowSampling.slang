// ShadowSampling.slang
// PCF sampling from a Texture2DArray cascade shadow map.
// Cascade selection is based on view-space depth of the fragment.

float shadowCalculationCSM(
    float3 positionWS,
    float3 normal,
    float3 lightDirection,
    Texture2DArray<float> shadowMap,
    SamplerState shadowSampler,
    float viewDepth)           // abs(view-space z) of the fragment
{
    // Select the tightest cascade that covers this fragment's depth
    int cascadeIndex = u_NumCascades - 1;
    for (int i = 0; i < u_NumCascades - 1; i++)
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

    // 5x5 PCF kernel sampling the selected cascade layer
    float shadow = 0.0f;
    const int R = 2;
    for (int x = -R; x <= R; x++)
    {
        for (int y = -R; y <= R; y++)
        {
            float closest = shadowMap.Sample(
                shadowSampler,
                float3(projCoords.xy + float2(x, y) * texelSize, (float)cascadeIndex));
            shadow += (currentDepth - bias > closest) ? 1.0f : 0.0f;
        }
    }

    float kernelDiameter = float(R * 2 + 1);
    return shadow / (kernelDiameter * kernelDiameter);
}

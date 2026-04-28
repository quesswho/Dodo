// ShadowSampling.slang


/*
    TODO: Implement PCSS:
    https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
    Our implementation: 
    1. Blocker search on 6x6 kernel:
    For each texel: Compute the mean of all depth texels that satisfy d > d_reciever
    2. w_Penumbra = (d_Receiver – d_Blocker) * w_Light / d_Blocker
    Where w_light is the brightness of the light.
    3. Compute PCF with kernel size proportional to w_Penumbra

    Things needed: 
    1. Add light intensity parameter.
    2. More flexible sample size for PCF, perhaps picking samples from a distribution but we might get noisy results
    

*/

// PCF shadow sampling with 3x3 kernel
float shadowCalculationPCF(
    float4 lightFragPos,
    float3 normal,
    float3 lightDirection,
    Texture2D<float> shadowMap,
    SamplerState shadowSampler
)
{
    // Perspective divide
    float3 projCoords = lightFragPos.xyz / lightFragPos.w;

    // Outside shadow frustum = no shadow
    if (projCoords.z > 1.0f)
        return 0.0f;

    // Transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5f + 0.5f;

    float currentDepth = projCoords.z;

    // Slope-scaled depth bias
    float bias = max(0.005f * (1.0f - dot(normal, lightDirection)), 0.001f);

    // PCF filtering
    uint depthWidth, depthHeight;
    shadowMap.GetDimensions(depthWidth, depthHeight);
    float2 texelSize = 1.0f / float2(depthWidth, depthHeight);

    // No shadow map bound (e.g. no shadow pass), treat as fully lit
    if (depthWidth == 0 || depthHeight == 0)
        return 0.0f;

    float shadow = 0.0f;
    const int kernelRadius = 2;

    for (int x = -kernelRadius; x <= kernelRadius; ++x)
    {
        for (int y = -kernelRadius; y <= kernelRadius; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float closestDepth = shadowMap.Sample(shadowSampler, projCoords.xy + offset);
            if (currentDepth - bias > closestDepth)
                shadow += 1.0f;
        }
    }

    float kernelDiameter = float(kernelRadius * 2 + 1);
    return shadow / (kernelDiameter * kernelDiameter);
}
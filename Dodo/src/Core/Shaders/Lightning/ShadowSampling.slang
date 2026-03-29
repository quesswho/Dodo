// PCF shadow sampling with 3x3 kernel
float shadowCalculationPCF(
    float4 lightFragPos, 
    float3 normal, 
    float3 lightDirection,
    Sampler2D<float> shadowMap
)
{
    // Perspective divide
    float3 projCoords = lightFragPos.xyz / lightFragPos.w;

    // Outside shadow frustum = no shadow
    if (projCoords.z > 1.0f)
        return 0.0f;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;

    float currentDepth = projCoords.z;

    // Slope-scaled depth bias
    float bias = max(0.005f * (1.0f - dot(normal, lightDirection)), 0.001f);

    // PCF filtering
    uint depthWidth, depthHeight;
    shadowMap.GetDimensions(depthWidth, depthHeight);
    float2 texelSize = 1.0f / float2(depthWidth, depthHeight);

    float shadow = 0.0f;
    const int kernelRadius = 1;

    for (int x = -kernelRadius; x <= kernelRadius; ++x)
    {
        for (int y = -kernelRadius; y <= kernelRadius; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float closestDepth = shadowMap.Sample(projCoords.xy + offset);
            if (currentDepth - bias > closestDepth)
                shadow += 1.0f;
        }
    }

    float samples = float(kernelRadius * 2 + 1);
    return shadow / (samples * samples);
}
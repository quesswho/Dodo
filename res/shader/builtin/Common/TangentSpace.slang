// TangentSpace.slang

float3x3 buildTBN(float3 normal, float4 tangent)
{
    float3 N = normalize(normal);
    float3 T = normalize(tangent.xyz);
    T = normalize(T - dot(T, N) * N);  // Gram-Schmidt orthogonalize
    float3 B = cross(N, T) * tangent.w; // tangent.w is +1 or -1 (bitangent handedness)
    return float3x3(T, B, N);
}

// Decodes a tangent-space normal map sample. Materials without a normal map sample the black 1x1
// fallback texture, which is not a valid normal, so those keep the unperturbed surface normal.
float3 decodeNormalTS(float3 encoded)
{
    if (dot(encoded, encoded) == 0.0f)
        return float3(0.0f, 0.0f, 1.0f);
    return encoded * 2.0f - 1.0f;
}

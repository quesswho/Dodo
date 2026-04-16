// TangentSpace.slang

float3x3 buildTBN(float3 normal, float4 tangent)
{
    float3 N = normalize(normal);
    float3 T = normalize(tangent.xyz);
    T = normalize(T - dot(T, N) * N);  // Gram-Schmidt orthogonalize
    float3 B = cross(N, T) * tangent.w; // tangent.w is +1 or -1 (bitangent handedness)
    return float3x3(T, B, N);
}
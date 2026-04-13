// TangentSpace.slang

float3x3 buildTBN(float3 normal, float3 tangent)
{
    float3 N = normalize(normal);
    float3 T = normalize(tangent);
    T = normalize(T - dot(T, N) * N);  // Gram-Schmidt orthogonalize
    float3 B = cross(N, T);
    return float3x3(T, B, N);
}
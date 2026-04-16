// BlinnPhong.slang
// Note: This legacy code is not used anymore

float3 evaluateBlinnPhong(float3 albedo, float3 normal, float specular, float shadow, float3 lightDir, float3 viewDir)
{
    float3 ambient = albedo * 0.1f;
    float3 diffuse = max(dot(lightDir, normal), 0.0f) * albedo;
    
    float3 halfwayDir = normalize(lightDir + viewDir);
    float3 specular = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f) * specular;
    
    return ambient + (1.0f - shadow) * (diffuse + specular);
}

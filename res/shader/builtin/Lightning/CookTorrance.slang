// CookTorrance.slang
// Cook-Torrance BRDF for direct lighting (GGX NDF, Smith geometry, Schlick Fresnel)

static const float PI = 3.14159265359f;

// Trowbridge-Reitz GGX normal distribution function
// Models the distribution of microfacet orientations on the surface.
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

// Schlick-GGX geometry term (single direction)
float GeometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotX / (NdotX * (1.0f - k) + k);
}

// Smith geometry function (both view and light directions)
float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

// Direct Cook-Torrance contribution from a single light.
// All vectors must be in world space.
//   radiance  - light radiance (color, from ILight.Irradiance)
//   albedo    - linear-space surface color
//   N         - surface normal (normalized)
//   V         - view direction (normalized, pointing toward camera)
//   L         - light direction (normalized, pointing toward light source)
//   roughness - perceptual roughness [0, 1]
//   metallic  - metallic factor [0, 1]
//   shadow    - shadow factor (0 = fully lit, 1 = fully shadowed)
float3 evaluateBRDFDirect(
    float3 radiance,
    float3 albedo, float3 N, float3 V, float3 L,
    float roughness, float metallic, float shadow)
{
    float3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.001f);
    float NdotL = max(dot(N, L), 0.001f);
    float HdotV = max(dot(H, V), 0.0f);

    // Reflectance at normal incidence: dielectric ~0.04, metal = albedo tint
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

    float  D = DistributionGGX(N, H, roughness);
    float  G = GeometrySmith(NdotV, NdotL, roughness);
    float3 F = FresnelSchlick(HdotV, F0);

    // Energy-conserving diffuse: non-metals only
    float3 kD = (float3(1.0f) - F) * (1.0f - metallic);

    float3 specular = (D * G * F) / max(4.0f * NdotV * NdotL, 0.001f);

    return (kD * albedo / PI + specular) * radiance * NdotL * (1.0f - shadow);
}

// Constant ambient term, placeholder until IBL irradiance map is available.
// Called once per fragment, outside any per-light loop.
//   albedo - linear-space surface color
//   ao     - ambient occlusion [0, 1]
float3 evaluateAmbient(float3 albedo, float ao)
{
    return float3(0.1f) * albedo * ao;
}

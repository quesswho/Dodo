// BindlessUniforms.slang
// Global bindless resources and sampler table shared across all forward-lit shaders.

// Named sampler indices (match registration order in VulkanRenderAPI::InitDescriptors)
static const uint SAMPLER_LINEAR_REPEAT  = 0;
static const uint SAMPLER_LINEAR_CLAMP   = 1;
static const uint SAMPLER_NEAREST_REPEAT = 2;
static const uint SAMPLER_NEAREST_CLAMP  = 3;
static const uint SAMPLER_SHADOW         = 4; // LINEAR_CLAMP_TO_BORDER_WHITE for shadow maps
static const uint SAMPLER_ANISO16        = 5; // anisotropic 16x (fallback: LINEAR_REPEAT)

// Set 0: global frame resources
[[vk::binding(4, 0)]] SamplerState       g_Samplers[32];           // named sampler table
[[vk::binding(5, 0)]] Texture2DArray<float> g_ShadowMap;           // cascaded shadow depth array
[[vk::binding(6, 0)]] TextureCube<float4>   g_EnvironmentMap;      // full-res skybox cubemap
[[vk::binding(7, 0)]] TextureCube<float4>   g_IrradianceMap;       // convoluted diffuse irradiance cubemap

// Set 1: bindless 2D texture heap (slot 0 = 1x1 white fallback)
[[vk::binding(0, 1)]] Texture2D<float4> g_Textures[];

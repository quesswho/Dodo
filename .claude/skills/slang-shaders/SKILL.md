---
name: slang-shaders
description: >
  Writing and editing Slang shader code (.slang files) for this project. Use this skill when
  the user asks to: write a new shader, add a feature to an existing shader, add specialization
  variants, pass per-draw parameters, add texture channels, create a new render pass, use
  link-time constants, use generics in shaders, or fix shader compilation errors.
  Trigger keywords: .slang, shader, SPIR-V, Slang, vertex shader, fragment shader, compute
  shader, entry point, descriptor set, sampler, normal map, push constant, specialization,
  link-time, config.slang, IChannel, Material<T>, VertexInput, VertexOutput, ForwardLit,
  uniform parameter, extern static const.
  DO NOT use for: GLSL-only code, HLSL-only code, OpenGL shader strings, C++ render backend code,
  CMake, pipeline state objects, descriptor set layout creation.
---

# Slang Shader Skill

## Overview

All shaders in this project are written in **Slang** (`.slang`), located in
`res/shader/builtin/`. Slang is a strongly-typed, generic shading language that compiles
to SPIR-V. For OpenGL, SPIRV-Cross then transpiles to GLSL at runtime.

The SDK version is **2026.5.2**. Docs are in `build/_deps/slang_sdk-src/share/doc/slang/`.

Key doc pages:
- `user-guide/06-interfaces-generics.md` — generics and interfaces
- `user-guide/10-link-time-specialization.md` — `extern static const`, config modules
- `user-guide/08-compiling.md` — three-step compile model

---

## 1. Entry Points and Parameter Passing

### Standard form

```slang
[shader("vertex")]
VertexOutput vertexMain(VertexInput input) { ... }

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target { ... }
```

### Passing per-draw scalars: use `uniform` in the signature

Small per-draw values (gamma, time, alpha threshold, etc.) go directly on the entry point
as `uniform` parameters. The backend uploads them as push constants automatically.

```slang
// CORRECT — uniform parameter on the entry point
[shader("fragment")]
float4 fragmentMain(VertexOutput input, uniform float gamma) : SV_Target
{
    float3 color = textureMap.Sample(textureSampler, input.texCoord).rgb;
    return float4(pow(color, float3(1.0f / gamma)), 1.0f);
}
```

This is how `Gamma.slang` does it today. The `uniform` keyword is the Slang-native way;
it maps to a push constant range on Vulkan and a UBO scalar on OpenGL — no explicit
descriptor annotation needed.

### NEVER use vk::push_constant

```slang
// WRONG — do not do this
[[vk::push_constant]]
struct PushConstants { float gamma; } gPC;
```

This is an HLSL-ism. It bypasses Slang's reflection and breaks the OpenGL backend.

### Large per-frame / per-model data: cbuffer in Uniforms.slang

Heavy uniform blocks (matrices, light data) live in `res/shader/builtin/Common/Uniforms.slang`
and are included via `#include`:

```slang
[[vk::binding(0, 0)]] cbuffer FrameData : register(b0)
{
    float4x4 u_Camera;
    float4x4 u_LightCamera;
    float3   u_LightDir;    float u_FramePadding0;
    float3   u_CameraPos;   float u_FramePadding1;
};

[[vk::binding(1, 0)]] cbuffer ModelData : register(b1)
{
    float4x4 u_Model;
    float4x4 u_NormalMatrix;
};
```

Use `float` padding fields to keep `float3` members 16-byte aligned in cbuffer layout.

---

## 2. Link-time Specialization (instead of #define macros)

**Never use `#define` or `#ifdef` for shader feature toggles.** Use link-time constants
instead. They produce the same dead-code elimination but are type-safe and allow module
precompilation.

### Declare in the shader with a default

```slang
// Shader declares the constant; default value used when no config module overrides it.
extern static const bool hasNormalMap = true;
extern static const bool hasShadows   = true;
extern static const int  kPCFRadius   = 1;
```

The `extern` modifier means "value resolved at link time". The `= true` / `= 1` is the
fallback used when no other module exports the symbol.

### Consume with plain if — the compiler constant-folds it

```slang
[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float3 N;
    if (hasNormalMap)
    {
        float3 normalTS = u_NormalMap.Sample(u_LinearSampler, input.texCoord).rgb * 2.0f - 1.0f;
        float3 N_ws = normalize(input.normalWS);
        float3 T_ws = normalize(input.tangentWS - dot(input.tangentWS, N_ws) * N_ws);
        float3 B_ws = cross(N_ws, T_ws);
        N = normalize(mul(normalTS, float3x3(T_ws, B_ws, N_ws)));
    }
    else
    {
        N = normalize(input.normalWS);
    }
    ...
}
```

When `hasNormalMap` is resolved to `false` at link time, the entire `if` body is eliminated.
No runtime branching, no texture sample for the unused path.

### Override via config.slang module

Create a small config module that exports the overriding values:

```slang
// res/shader/builtin/Passes/config_no_normal.slang
export static const bool hasNormalMap = false;
export static const bool hasShadows   = true;
export static const int  kPCFRadius   = 1;
```

Load it with `import` (Slang search paths) or `#include`:

```slang
// In the pass shader or a composite entry point:
#include "config_no_normal.slang"
```

The C++ side (SlangCompiler) can also load config modules programmatically via
`ISession::loadModuleFromSourceString` to generate variants without touching files.

### Integer link-time constants

```slang
extern static const int kSampleCount = 4;

[ForceUnroll]      // hint to unroll when constant is known
for (int i = 0; i < kSampleCount; i++) { ... }
```

---

## 3. Generics and Interfaces (instead of macro polymorphism)

Slang generics allow zero-cost abstraction — no runtime vtable, fully inlined at
specialization time.

### IChannel pattern (already in this codebase)

`res/shader/builtin/Common/Material.slang`:

```slang
interface IChannel
{
    float4 getValue(float2 uv);
}

struct ConstChannel : IChannel
{
    float4 value;
    float4 getValue(float2 uv) { return value; }
}

struct TextureChannel : IChannel
{
    Texture2D  texture;
    SamplerState sampler;
    float4 getValue(float2 uv) { return texture.Sample(sampler, uv); }
}

struct Material<TAlbedo : IChannel, TRoughness : IChannel,
                TMetallic : IChannel, TNormal : IChannel>
{
    TAlbedo   albedo;
    TRoughness roughness;
    TMetallic  metallic;
    TNormal    normal;
}
```

Usage — specialize at call site or via a typedef:

```slang
// A fully-textured PBR material
typedef Material<TextureChannel, TextureChannel, TextureChannel, TextureChannel>
    PBRMaterial;

// A material with constant albedo, no normal map
typedef Material<ConstChannel, TextureChannel, TextureChannel, ConstChannel>
    SimpleMaterial;
```

### Writing your own generic utility

```slang
interface ILight
{
    float3 Irradiance(float3 worldPos);
}

struct DirectionalLight : ILight
{
    float3 direction;
    float3 color;
    float3 Irradiance(float3 worldPos) { return color; }
}

// Generic function — works with any ILight implementor
float3 shade<TLight : ILight>(TLight light, float3 worldPos, float3 N, float3 V)
{
    float3 L = normalize(-light.direction);
    return evaluateCookTorrance(..., light.Irradiance(worldPos));
}
```

### Conditional<T> pattern for optional fields

Use a generic struct conditioned on a `let bool` type parameter to carry optional data
without runtime cost:

```slang
struct Conditional<T, let enabled : bool>
{
    T _value;
    property T value { get { return _value; } }
}

// Specializes away to nothing when enabled = false
Conditional<float3, hasNormalMap> normalSample;
if (hasNormalMap)
    normalSample._value = u_NormalMap.Sample(u_LinearSampler, uv).rgb;
```

When `enabled` is `false` (from a link-time constant), the field is zero-sized and the
block is dead-code-eliminated.

---

## 4. Descriptor Binding

Always use the double-bracket `[[vk::binding(slot, set)]]` form. Set 0 = frame/model
uniforms, Set 1 = material textures.

```slang
// Set 0 — always included via Uniforms.slang
[[vk::binding(0, 0)]] cbuffer FrameData : register(b0) { ... };
[[vk::binding(1, 0)]] cbuffer ModelData : register(b1) { ... };

// Set 1 — material resources, declared in the pass file
[[vk::binding(0, 1)]] Texture2D<float4>  u_AlbedoMap;
[[vk::binding(1, 1)]] Texture2D<float4>  u_RoughnessMap;  // sample .g
[[vk::binding(2, 1)]] Texture2D<float4>  u_NormalMap;
[[vk::binding(3, 1)]] Texture2D<float>   u_ShadowMap;
[[vk::binding(4, 1)]] SamplerState       u_LinearSampler;
[[vk::binding(5, 1)]] Texture2D<float4>  u_MetallicMap;   // sample .b
[[vk::binding(6, 1)]] Texture2D<float4>  u_AoMap;
```

One sampler (`u_LinearSampler`) is shared across all material textures and the shadow map.
Skybox and Gamma passes have their own set-1 resources and do not include `Uniforms.slang`'s
ModelData.

---

## 5. Module / Include Structure

```
res/shader/builtin/
├── Common/
│   ├── Uniforms.slang        — FrameData + ModelData cbuffers (set 0)
│   ├── VertexInterface.slang — VertexInput / VertexOutput structs
│   ├── Material.slang        — IChannel, ConstChannel, TextureChannel, Material<T>
│   ├── TangentSpace.slang    — buildTBN() helper
│   └── Fallback.slang        — checker pattern fallback pass
├── Lightning/
│   ├── Lights.slang          — ILight, DirectionalLight, PointLight
│   ├── CookTorrance.slang    — evaluateCookTorrance()
│   ├── BlinnPhong.slang      — evaluateBlinnPhong()
│   └── ShadowSampling.slang  — shadowCalculationPCF()
└── Passes/
    ├── ForwardLit.slang      — main PBR pass
    ├── Shadow.slang          — depth-only shadow pass
    ├── Skybox.slang          — cubemap skybox
    └── Gamma.slang           — post-process gamma correction
```

Include order in a new pass:
```slang
#include "../Common/Uniforms.slang"         // always first — declares FrameData/ModelData
#include "../Common/VertexInterface.slang"  // VertexInput / VertexOutput
#include "../Common/Material.slang"         // optional — if using IChannel
#include "../Lightning/ShadowSampling.slang"
#include "../Lightning/CookTorrance.slang"
// then your own extern static const declarations and entry points
```

---

## 6. Common Mistakes and Corrections

| Wrong (HLSL idiom) | Correct (native Slang) |
|---|---|
| `[[vk::push_constant]] struct PC { float x; };` | `uniform float x` in entry point signature |
| `#define HAS_NORMAL 1` | `extern static const bool hasNormal = true;` |
| `#ifdef HAS_NORMAL ... #endif` | `if (hasNormal) { ... }` |
| `[vk::binding(0,1)]` (single bracket) | `[[vk::binding(0, 1)]]` (double bracket) |
| `float3 n = normalize(input.normalWS); #ifdef ... n = normalMap... #endif` | `if (hasNormalMap) { ... }` — same effect |
| `import "Uniforms.slang"` | `#include "../Common/Uniforms.slang"` (this codebase uses includes) |
| Generic `T` without constraint | `T : IChannel` — always constrain to an interface |
| Padding floats named `_pad` | Name them `u_FramePadding0`, `u_ModelPadding0` to match convention |

---

## 7. Codebase Quick Reference

| Symbol | File | Purpose |
|---|---|---|
| `VertexInput` / `VertexOutput` | `Common/VertexInterface.slang` | Standard VS in/out structs |
| `u_Camera`, `u_LightDir`, `u_CameraPos` | `Common/Uniforms.slang` (FrameData) | Per-frame data |
| `u_Model`, `u_NormalMatrix` | `Common/Uniforms.slang` (ModelData) | Per-draw matrices |
| `IChannel` | `Common/Material.slang` | Interface for texture/const channels |
| `Material<...>` | `Common/Material.slang` | Generic PBR material struct |
| `buildTBN()` | `Common/TangentSpace.slang` | Gram-Schmidt TBN construction |
| `evaluateCookTorrance()` | `Lightning/CookTorrance.slang` | Full PBR BRDF |
| `shadowCalculationPCF()` | `Lightning/ShadowSampling.slang` | 3x3 PCF shadow |
| `ILight` | `Lightning/Lights.slang` | Light interface |

### Typical fragment shader anatomy for a new pass

```slang
#include "../Common/Uniforms.slang"
#include "../Common/VertexInterface.slang"
#include "../Lightning/CookTorrance.slang"

// Link-time feature flags (defaults; override with config.slang)
extern static const bool hasNormalMap = true;
extern static const bool hasShadows   = true;

// Material textures (set 1)
[[vk::binding(0, 1)]] Texture2D<float4> u_AlbedoMap;
[[vk::binding(1, 1)]] SamplerState      u_Sampler;
// ... more bindings ...

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput o;
    o.positionWS = mul(u_Model, float4(input.position, 1.0f)).xyz;
    o.positionSS = mul(u_Camera, float4(o.positionWS, 1.0f));
    o.texCoord   = input.texCoord;
    o.normalWS   = normalize(mul(u_NormalMatrix, float4(input.normal,  0.0f)).xyz);
    o.tangentWS  = normalize(mul(u_NormalMatrix, float4(input.tangent, 0.0f)).xyz);
    o.positionLL = mul(u_LightCamera, float4(o.positionWS, 1.0f));
    return o;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float4 albedo = u_AlbedoMap.Sample(u_Sampler, input.texCoord);
    if (albedo.a < 0.1f) discard;

    float3 N;
    if (hasNormalMap)
    {
        float3 N_ws = normalize(input.normalWS);
        float3 T_ws = normalize(input.tangentWS - dot(input.tangentWS, N_ws) * N_ws);
        float3 B_ws = cross(N_ws, T_ws);
        float3 nTS  = u_NormalMap.Sample(u_Sampler, input.texCoord).rgb * 2.0f - 1.0f;
        N = normalize(mul(nTS, float3x3(T_ws, B_ws, N_ws)));
    }
    else
    {
        N = normalize(input.normalWS);
    }

    float3 V = normalize(u_CameraPos - input.positionWS);
    float3 L = normalize(-u_LightDir);
    // ... lighting ...
    return float4(result, albedo.a);
}
```

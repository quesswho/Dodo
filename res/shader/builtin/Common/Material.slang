// Material.slang

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
    Texture2D texture;
    SamplerState sampler;

    float4 getValue(float2 uv) { return texture.Sample(sampler, uv); }
}

struct Material<TAlbedo:IChannel, TRoughness:IChannel, TMetallic:IChannel, TNormal:IChannel>
{
    TAlbedo albedo;
    TRoughness roughness;
    TMetallic metallic;
    TNormal normal;
}
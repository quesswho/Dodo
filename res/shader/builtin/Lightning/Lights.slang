// Lights.slang
interface ILight
{
    float3 Irradiance(float3 pos);
}

struct PointLight : ILight
{
    float3 m_Color;
    float3 m_Pos;

    float3 Irradiance(float3 pos)
    {
        float3 dir = m_Pos - pos;
        return m_Color / dot(dir, dir);
    }
}

struct DirectionalLight : ILight
{
    float3 m_Color;
    float3 m_Direction;

    float3 Irradiance(float3 pos)
    {
        return m_Color;
    }
}
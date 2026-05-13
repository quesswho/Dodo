// Debug.slang
// [0,1] to colors: blue -> cyan -> green -> yellow -> red
float3 debugFloat(float t) {
    t = clamp(t, 0.0, 1.0);
    
    float3 c;
    c.r = smoothstep(0.5, 0.75, t);
    c.g = smoothstep(0.25, 0.5, t) - smoothstep(0.75, 1.0, t);
    c.b = 1.0 - smoothstep(0.0, 0.5, t);
    return c;
}
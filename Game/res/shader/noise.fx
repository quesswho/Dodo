#shader fragment

#version 330 core

out vec4 pixel;

in vec2 f_TexCoord;

uniform sampler2D u_Texture;

uniform float u_Gamma;

float simplex(float, float);
float snoise(vec2);
float sumSimplex(vec2 v, int num_iterations, float persistence, float scale);

void main()
{
    //vec3 color = texture(u_Texture, f_TexCoord).rgb;
    vec3 color = vec3(1.0f);
    //color *= simplex(gl_FragCoord.x * u_Gamma, gl_FragCoord.y * u_Gamma)/2.0+0.5;
    //color *= snoise(gl_FragCoord.xy * u_Gamma) / 2.0 + 0.5;
    color *= sumSimplex(gl_FragCoord.xy + vec2(1321314, -124141), 25, 0.25, u_Gamma*0.01) / 2.0 + 0.5;
    pixel = vec4(color, 1.0f);
}

vec3 permute(vec3 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439,
        -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
        + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy),
        dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float sumSimplex(vec2 v, int num_iterations, float persistence, float scale) {
    float maxAmp = 0;
    float amp = 1;
    float freq = scale;
    float noise = 0;

    // add successively smaller scale and higher frequency terms
    for (int i = 0; i < num_iterations; ++i) {
        noise += snoise(v * freq) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2;
    }
    noise /= maxAmp;

    return noise;
}

#shader vertex

#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 f_TexCoord;

void main()
{
    f_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0);
}
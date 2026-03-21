#shader fragment

#version 420 core

out vec4 pixel;

in vec2 f_TexCoord;

layout(binding = 0) uniform sampler2D u_TextureMap;

uniform float u_Gamma = 2.2f;

void main()
{
	vec3 color = texture(u_TextureMap, f_TexCoord).rgb;
	vec3 result = color;
	//vec3 result = pow(color / (color + vec3(1.0)), vec3(1.0 / u_Gamma));
    pixel = vec4(result, 1.0f);
}

#shader vertex

#version 420 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoord;

out vec2 f_TexCoord;

void main()
{
	f_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0);
}
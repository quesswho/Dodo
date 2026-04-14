#shader fragment

#version 330 core
in vec2 o_Texcoord;
out vec4 pixel;

float checker(vec2 uv)
{
	float cx = floor(20.0f * o_Texcoord.x);
	float cy = floor(20.0f * o_Texcoord.y);
	return sign(mod(cx + cy, 2.0f));
}

void main()
{
	float res = mix(1.0f, 0.0f, checker(o_Texcoord));
	pixel = vec4(res, res * 0.1f, res * 0.1f, 1.0);
}

#shader vertex

#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_Texcoord;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_Camera = mat4(1.0f);

out vec2 o_Texcoord;

void main()
{
   o_Texcoord = a_Texcoord;
   gl_Position = u_Camera * u_Model * vec4(a_Position, 1.0);
}
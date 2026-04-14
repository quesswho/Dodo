#shader fragment

#version 330 core

out vec4 pixel;

in vec3 o_Color;
in vec3 o_Normal;

void main()
{
    pixel = vec4(normalize(o_Normal * distance(o_Normal, o_Color)), 1.0f);
}

#shader vertex

#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_Texcoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_Camera = mat4(1.0f);

out vec3 o_Color;
out vec3 o_Normal;

void main()
{
    o_Normal = a_Normal;
    o_Color = vec3(a_Position.x, a_Position.y, a_Position.z);
    gl_Position = u_Camera * u_Model * vec4(a_Position.x, a_Position.y, a_Position.z, 1.0f);
}

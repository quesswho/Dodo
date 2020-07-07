#shader fragment

#version 330 core

out vec4 pixel;

in vec3 o_Color;

void main()
{
    pixel = vec4(o_Color.x, o_Color.y, o_Color.z, 1.0f);
}

#shader vertex

#version 330 core
layout (location = 0) in vec3 a_Position;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);

out vec3 o_Color;

void main()
{
    o_Color = vec3(a_Position.x, a_Position.y, a_Position.z);
    gl_Position = u_Projection * u_Model * vec4(a_Position.x, a_Position.y, a_Position.z, 1.0f);
}

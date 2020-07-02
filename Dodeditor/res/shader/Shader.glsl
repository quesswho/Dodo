#shader fragment

#version 330 core

out vec4 pixel;

void main()
{
    pixel = vec4(1.0f, 0.4f, 0.2f, 1.0f);
}

#shader vertex

#version 330 core
layout (location = 0) in vec3 a_Position;

uniform mat4 u_Model = mat4(1.0f);

void main()
{
    gl_Position = u_Model * vec4(a_Position.x, a_Position.y, a_Position.z, 1.0f);
}

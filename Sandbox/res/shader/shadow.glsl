#shader fragment

#version 330 core

//out vec4 pixel;

void main()
{
	gl_FragDepth = gl_FragCoord.z;
	//pixel = vec4(gl_FragCoord.z);
}

#shader vertex

#version 330 core
layout (location = 0) in vec3 a_Position;

uniform mat4 u_LightCamera;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightCamera * u_Model * vec4(a_Position, 1.0);
}
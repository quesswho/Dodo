#shader fragment

#version 330 core

out vec4 pixel;

uniform sampler2D tex;

in Vertex_Out {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
} frag_in;

void main()
{
    //if(frag_in.Normal.z < 0) discard;
    pixel = texture(tex, frag_in.TexCoord);
    //pixel = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#shader vertex

#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_Texcoord;
layout (location = 2) in vec3 a_Normal;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_Camera = mat4(1.0f);

out Vertex_Out {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 Normal;
} vertex_out;

void main()
{
    vertex_out.FragPos = vec3(u_Model * vec4(a_Position, 1.0f));
    vertex_out.TexCoord = a_Texcoord;
    vertex_out.Normal = a_Normal;
    gl_Position = u_Camera * u_Model * vec4(a_Position, 1.0f);
}

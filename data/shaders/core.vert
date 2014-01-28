#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

void main()
{
    gl_Position = vec4(position.x, position.z, position.y, 1.0);
}


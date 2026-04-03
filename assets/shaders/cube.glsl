#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 instancePos;

out vec2 TexCoord;

layout(std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

void main() {
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(aPos + instancePos, 1.0);
}

#version 330 core
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

layout(std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model; // per-object

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}

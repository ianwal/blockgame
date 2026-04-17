#version 330 core
in vec2 TexCoord;
uniform sampler2D tex;
out vec4 FragColor;

void main() {
    vec4 color = texture(tex, TexCoord);

    if (color.a < 0.5) {
        discard;
    }

    FragColor = color;
}

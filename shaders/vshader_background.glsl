#version 330

layout (location = 0) in vec2 vertexPosition;

out vec2 fragCoord;

void main() {
    gl_Position = vec4(vertexPosition, 0.f, 1.f);
    fragCoord = vertexPosition;
}
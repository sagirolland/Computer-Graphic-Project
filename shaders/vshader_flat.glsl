#version 330

layout (location = 0) in vec3 vertexPosition;
layout (location = 2) in vec3 vertexNormal;

uniform mat4 toScreenFromModel;
uniform mat4 normalTransform;

void main() {
    gl_Position = vec4(vertexPosition, 1.0);
}
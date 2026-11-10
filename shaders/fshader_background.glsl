#version 330

in vec2 fragCoord;
out vec4 fragColor;

uniform float time;

void main() {
    fragColor.r = sin(fragCoord.x + fragCoord.y + time)/2.f + .5f;
    fragColor.g = sin(fragCoord.y)/2.f + .5f;
    fragColor.b = sin(fragCoord.x + time)/2.f + .5f;
    fragColor.a = 1.f;
}

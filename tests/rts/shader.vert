#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 position;

void main() {
    gl_Position = vec4(inPosition,1.0);
}
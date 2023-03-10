#version 450

precision highp float;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexCoord;
layout(location = 3) in vec3 instancePos;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition + instancePos, 1.0);
    fragColor = vec3(1.0, 0.0, 0.0);
    if (instancePos.z < -1) {
        fragColor = vec3(0.0, 0.0, 1.0);
    }
    fragTexCoord = inTexCoord;
}

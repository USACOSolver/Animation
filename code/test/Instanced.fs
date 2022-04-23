#version 330 core

in vec3 Normal;
uniform vec3 lightDir;
uniform vec4 color;
out vec4 FragColor;
void main() {
    FragColor = color * max(dot(lightDir, Normal), 0.0);
}
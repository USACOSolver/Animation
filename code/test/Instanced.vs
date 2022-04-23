#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in mat4 aInstanceMatrix;
uniform mat4 projection;
uniform mat4 view;
out vec3 Normal;
void main() {
    Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
   	gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f);
}
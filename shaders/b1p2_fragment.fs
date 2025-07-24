#version 330 core
in vec3 ourColor;
out vec4 FragColor;

uniform float intensity;

void main()
{
    FragColor = vec4(ourColor * intensity, 1.0);
}
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float time;
uniform float offsetX;

void main()
{
    float offset = sin(time) * 0.1;
    vec3 pos = aPos;
    pos.y += offset;
    pos.x += offsetX; 

    gl_Position = vec4(pos, 1.0);
    TexCoord = (aPos.xy + vec2(1.0)) / 2.0;
}
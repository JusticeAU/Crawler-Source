#version 330 core

in vec2 _TexCoords;
out vec4 FragColor;

uniform sampler2D frame;

void main()
{             
    float depthValue = texture(frame, _TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}  
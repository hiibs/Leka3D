#version 330 core

uniform sampler2D colorMap;
uniform vec4 tint;

in vec2 texCoord;
out vec4 FragColor;

void main()
{
	FragColor = texture2D(colorMap, texCoord) * tint;
};
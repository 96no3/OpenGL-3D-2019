/**
* @file Simple.frag
*/
#version 430

layout(location=1) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D texColor;

/**
* Simple Fragment Shader.
*/
void main()
{
  fragColor = texture(texColor, inTexCoord);
}
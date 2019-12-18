/**
* @file Shadow.frag
*/
#version 430

layout(location=1) in vec2 inTexCoord;

uniform sampler2D texColor;

/**
* Shadow Fragment Shader.
*/
void main()
{
  // �s�����x��0.5�����̃s�N�Z���͔j������i�e�𗎂Ƃ��Ȃ��j.
  float a = texture(texColor, inTexCoord).a;
  if (a < 0.5) {
    discard; 
  }
}
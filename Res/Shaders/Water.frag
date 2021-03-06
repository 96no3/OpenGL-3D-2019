/**
* @file Water.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inTBN[3];
layout(location=5) in vec3 inRawPosition;

out vec4 fragColor;

uniform sampler2D texColorArray[4];
uniform sampler2D texNormalArray[3];
uniform isamplerBuffer texPointLightIndex;
uniform isamplerBuffer texSpotLightIndex;

uniform vec3 cameraPosition;
uniform samplerCube texCubeMap;
uniform float time;

const ivec2 mapSize = ivec2(200, 200);

struct AmbientLight
{
  vec4 color;
};

struct DirectionalLight
{
  vec4 color;
  vec4 direction;
};

struct PointLight
{
  vec4 color;
  vec4 position;
};

struct SpotLight
{
  vec4 color;
  vec4 dirAndCutOff;
  vec4 posAndInnerCutOff;
};

layout(std140) uniform LightUniformBlock
{
  AmbientLight ambientLight;
  DirectionalLight directionalLight;
  PointLight pointLight[100];
  SpotLight spotLight[100];
};

const float iorAir = 1.000293; // 空気の屈折率.
const float iorWater = 1.333;  // 水の屈折率.

const float eta = iorAir / iorWater;
const float f0 = (1.0 - eta) * (1.0 - eta) / ((1.0 + eta) * (1.0 + eta));

/**
* シュリック近似式によってフレネル係数を計算する.
*
* @param v カメラベクトル.
* @param n 法線ベクトル.
*
* @return  フレネル係数.
*/
float GetFresnelFactor(vec3 v, vec3 n)
{
  return f0 + (1.0 - f0) * pow(1.0 - dot(v,n), 5.0);
}

/**
* Water用フラグメントシェーダー.
*/
void main()
{
  vec4 waterColor = vec4(0.15, 0.20, 0.3, 0.6); // 水の色と不透明度.
  fragColor = waterColor;

  mat3 matTBN = mat3(normalize(inTBN[0]),normalize(inTBN[1]),normalize(inTBN[2]));
  vec4 uv = inTexCoord.xyxy * vec4(11.0, 11.0, 5.0, 5.0);
  vec4 scroll = vec4(-0.01, -0.01, 0.005, 0.005) * time;
  vec3 normalS = texture(texNormalArray[0], uv.xy + scroll.xy).rgb * 2.0 - 1.0;
  vec3 normalL = texture(texNormalArray[0], uv.zw + scroll.zw).rgb * 2.0 - 1.0;
  float roughness = 0.005; // 水面の荒れ具合.
  vec3 normal = (normalS * 0.5 + normalL) * vec3(roughness, roughness, 1.0);
  normal = normalize(matTBN * normal);

  vec3 lightColor = ambientLight.color.rgb;

  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power;

  int offset = int(inRawPosition.z)*mapSize.x + int(inRawPosition.x);
  ivec4 pointLightIndex = texelFetch(texPointLightIndex,offset);
  for (int i = 0; i < 4; ++i) {
    int id = pointLightIndex[i];
	if(id >= 0){
      vec3 lightVector = pointLight[id].position.xyz - inPosition;
      vec3 lightDir = normalize(lightVector);
      float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
      float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
      lightColor += pointLight[id].color.rgb * cosTheta * intensity;
	}
  }

  ivec4 spotLightIndex = texelFetch(texSpotLightIndex,offset);
  for (int i = 0; i < 4; ++i) {
    int id = spotLightIndex[i];
	if(id>=0){
      vec3 lightVector = spotLight[id].posAndInnerCutOff.xyz - inPosition;
      vec3 lightDir = normalize(lightVector);
      float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
      float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
      float spotCosTheta = dot(lightDir, -spotLight[id].dirAndCutOff.xyz);
      float cutOff = smoothstep(spotLight[id].dirAndCutOff.w, spotLight[id].posAndInnerCutOff.w, spotCosTheta);
      lightColor += spotLight[id].color.rgb * cosTheta * intensity * cutOff;
	}
  }

  fragColor.rgb *= lightColor;

  vec3 cameraVector = normalize(cameraPosition - inPosition);
  vec3 reflectionVector = 2.0 * max(dot(cameraVector,normal), 0.0) * normal - cameraVector;
  vec3 environmentColor = texture(texCubeMap, reflectionVector).rgb;
  //fragColor.rgb += environmentColor;

  // ITU-R BT.601の変換式に従ってYUV(実際にはYCbCr)に変換し、Yにフレネル係数を掛けてRGBに戻す.
  float brightness = 10.0; // 環境マップの明るさ補正値.
  //float opacity = 0.6;    // 水の不透明度.
  //float f = GetFresnelFactor(cameraVector, normal);
  //fragColor.rgb += environmentColor * f * brightness;
  //fragColor.a = clamp(opacity + f * (1.0 - opacity), 0.0, 1.0);
  vec3 yuv = mat3(
    0.299,-0.169, 0.500,
    0.587,-0.331,-0.419,
    0.114, 0.500,-0.081) * environmentColor;
  yuv.r = pow(yuv.r, 3.0); // 中間の明るさ対策
  yuv.r *= GetFresnelFactor(cameraVector, normal) * brightness;
  //fragColor.a = opacity + yuv.r;
  //yuv.r /= fragColor.a; // アルファを乗算した結果を色の加算と等しくする.

  // 散乱光 * 不透明度 + 反射光.
  fragColor.rgb *= fragColor.a;
  fragColor.rgb += mat3(
    1.000, 1.000, 1.000,
    0.000,-0.344, 1.772,
    1.402,-0.714, 0.000) * yuv;
  //fragColor.a = opacity + yuv.r;
  //fragColor.a += yuv.r;
  fragColor.a = clamp(fragColor.a + yuv.r, 0.0, 1.0);
}
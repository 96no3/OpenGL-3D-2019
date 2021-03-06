/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include <GL/glew.h>
#include "Shader.h"
#include "SkeletalMesh/UniformBuffer.h"
#include "Actor.h"
#include <glm/glm.hpp>
#include <vector>
#include <math.h>

/**
* 環境光.
*/
struct AmbientLight {
	glm::vec4 color = glm::vec4(0.1f, 0.05f, 0.15f, 1);
};

/**
* 指向性ライト.
*/
struct DirectionalLight {	
	glm::vec4 color = glm::vec4(1);
	glm::vec4 direction = glm::vec4(0, -1, 0, 1);
};

/**
* ポイントライト.
*/
struct PointLight {
	glm::vec4 color = glm::vec4(1);
	glm::vec4 position = glm::vec4(0);
};

/**
* スポットライト.
*/
struct SpotLight {
	glm::vec4 color = glm::vec4(1);
	glm::vec4 dirAndCutOff = glm::vec4(0, -1, 0, 0.9f);
	glm::vec4 posAndInnerCutOff = glm::vec4(0, 0, 0, 0.95f);
};

/**
* ライト用インターフェイス・ブロックに対応する構造体.
*/
struct LightUniformBlock {
	AmbientLight ambientLight;
	DirectionalLight directionalLight;
	PointLight pointLight[100];
	SpotLight spotLight[100];
};

/**
* ディレクショナルライトアクター.
*/
class DirectionalLightActor :public Actor 
{
public:
	DirectionalLightActor(const std::string& name,const glm::vec3& c,const glm::vec3& d)
		:Actor(name,1,glm::vec3(0)),color(c),direction(d)
	{}
	~DirectionalLightActor() = default;

	glm::vec3 color;
	glm::vec3 direction;
};
using DirectionalLightActorPtr = std::shared_ptr<DirectionalLightActor>;

/**
* ポイントライトアクター.
*/
class PointLightActor :public Actor
{
public:
	PointLightActor(const std::string& name, const glm::vec3& c, const glm::vec3& p)
		:Actor(name, 1, p), color(c)
	{}
	~PointLightActor() = default;

	glm::vec3 color;
	int index = -1;
};
using PointLightActorPtr = std::shared_ptr<PointLightActor>;

/**
* スポットライトアクター.
*/
class SpotLightActor :public Actor
{
public:
	SpotLightActor(const std::string& name, const glm::vec3& c, const glm::vec3& p,
		const glm::vec3& d, float cutOff, float innerCutOff)
		:Actor(name, 1, p), color(c),direction(d),
		cutOff(std::cos(cutOff)),innerCutOff(std::cos(innerCutOff))
	{}
	~SpotLightActor() = default;

	glm::vec3 color;
	glm::vec3 direction;
	float cutOff;
	float innerCutOff;
	int index = -1;
};
using SpotLightActorPtr = std::shared_ptr<SpotLightActor>;

/**
* UBOを利用してライトデータをGPUに転送するためのクラス.
*/
class LightBuffer
{
public:
	LightBuffer() = default;
	~LightBuffer() = default;

	bool Init(GLuint bindingPoint);
	bool BindToShader(const Shader::ProgramPtr& program);
	void Update(const ActorList& al, const glm::vec3& ambientColor);
	void Upload();
	void Bind();

private:
	LightUniformBlock data;
	UniformBufferPtr ubo[2];
	int currentUboIndex = 0; ///< UBOダブルバッファの書き込み側インデックス.
};

#endif // LIGHT_H_INCLUDED
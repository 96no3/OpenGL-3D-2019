/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ.
*
* @param name	アクターの名前.
* @param hp		耐久力.
* @param pos	位置.
* @param rot	回転.
* @param scale	拡大率.
*
* 指定された名前、耐久力、位置、回転、拡大率によってアクターを初期化する.
*/
Actor::Actor(const std::string& name, int hp, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
	: name(name), health(hp), position(pos), rotation(rot), scale(scale)
{
}

/**
* アクターの状態を更新する.
*
* @param deltaTime	経過時間.
*
* UpdateDrawData(float deltaTime)より前に実行すること.
*/
void Actor::Update(float deltaTime){

	position += velocity * deltaTime;
}

/**
* 描画情報の更新.
*
* @param deltaTime	経過時間.
*
* Update(float deltaTime)の後で実行すること.
*/
void Actor::UpdateDrawData(float deltaTime)
{
}

/**
* アクターの描画.
*/
void Actor::Draw()
{
}
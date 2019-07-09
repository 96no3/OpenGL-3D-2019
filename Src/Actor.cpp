/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �R���X�g���N�^.
*
* @param name	�A�N�^�[�̖��O.
* @param hp		�ϋv��.
* @param pos	�ʒu.
* @param rot	��].
* @param scale	�g�嗦.
*
* �w�肳�ꂽ���O�A�ϋv�́A�ʒu�A��]�A�g�嗦�ɂ���ăA�N�^�[������������.
*/
Actor::Actor(const std::string& name, int hp, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
	: name(name), health(hp), position(pos), rotation(rot), scale(scale)
{
}

/**
* �A�N�^�[�̏�Ԃ��X�V����.
*
* @param deltaTime	�o�ߎ���.
*
* UpdateDrawData(float deltaTime)���O�Ɏ��s���邱��.
*/
void Actor::Update(float deltaTime){

	position += velocity * deltaTime;
}

/**
* �`����̍X�V.
*
* @param deltaTime	�o�ߎ���.
*
* Update(float deltaTime)�̌�Ŏ��s���邱��.
*/
void Actor::UpdateDrawData(float deltaTime)
{
}

/**
* �A�N�^�[�̕`��.
*/
void Actor::Draw()
{
}
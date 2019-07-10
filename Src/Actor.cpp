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


/**
* �R���X�g���N�^.
*
* @param m		�\�����郁�b�V��.
* @param name	�A�N�^�[�̖��O.
* @param hp		�ϋv��.
* @param pos	�ʒu.
* @param rot	��].
* @param scale	�g�嗦.
*
* �w�肳�ꂽ���b�V���A���O�A�ϋv�́A�ʒu�A��]�A�g�嗦�ɂ���ăA�N�^�[������������.
*/
StaticMeshActor::StaticMeshActor(const Mesh::FilePtr& m, const std::string& name, int hp, const glm::vec3& pos,
								 const glm::vec3& rot, const glm::vec3& scale)
	:mesh(m),Actor(name, hp, pos, rot, scale)
{
}

/**
* �`��.
*/
void StaticMeshActor::Draw() 
{
	if (mesh) 
	{
		const glm::mat4 matT = glm::translate(glm::mat4(1), position);
		const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
		const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
		const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
		const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matModel = matT * matR_XZY * matS;
		Mesh::Draw(mesh, matModel);
	}
}


/**
* �i�[�\�ȃA�N�^�[�����m�ۂ���.
*
* @param reserveCount	�A�N�^�[�̔z��̊m�ې�.
*/
void ActorList::Reserve(size_t reserveCount)
{
	actors.reserve(reserveCount);
}

/**
* �A�N�^�[��ǉ�����.
*
* @param actor	�ǉ�����A�N�^�[.
*/
void ActorList::Add(const ActorPtr& actor)
{
	actors.push_back(actor);
}

/**
* �A�N�^�[���폜����.
*
* @param actor	�폜����A�N�^�[.
*
* @retval true	 �폜����.
* @retval false	 �폜���s.
*/
bool ActorList::Remove(const ActorPtr& actor)
{
	for (auto itr = actors.begin(); itr != actors.end(); ++itr) {
		if (*itr == actor) {
			actors.erase(itr);
			return true;
		}
	}
	return false;
}

/**
* �A�N�^�[�̏�Ԃ��X�V����.
*
* @param deltaTime	�O��̍X�V����̌o�ߎ���.
*
* UpdateDrawData(float deltaTime)���O�Ɏ��s���邱��.
*/
void ActorList::Update(float deltaTime)
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->Update(deltaTime);
		}
	}
}

/**
* �A�N�^�[�̕`��f�[�^���X�V����.
*
* @param deltaTime	�O��̍X�V����̌o�ߎ���.
*
* Update(float deltaTime)�̌�Ŏ��s���邱��.
*/
void ActorList::UpdateDrawData(float deltaTime)
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->UpdateDrawData(deltaTime);
		}
	}
}

/**
* �A�N�^�[��`�悷��.
*/
void ActorList::Draw()
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->Draw();
		}
	}
}
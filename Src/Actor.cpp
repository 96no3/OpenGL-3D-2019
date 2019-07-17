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
void Actor::Update(float deltaTime) {

	position += velocity * deltaTime;

	// �Փ˔���̍X�V.
	/*colWorld = colLocal;
	colWorld.center += position;*/
	const glm::mat4 matT = glm::translate(glm::mat4(1), position);
	const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
	const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
	const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
	const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
	const glm::mat4 matModel = matT * matR_XZY * matS;
	colWorld.type = colLocal.type;

	switch (colLocal.type)
	{
	case Collision::Shape::Type::sphere:
		colWorld.s.center = matModel * glm::vec4(colLocal.s.center, 1);
		colWorld.s.r = colLocal.s.r;
		break;

	case Collision::Shape::Type::capsule:
		colWorld.c.seg.a = matModel * glm::vec4(colLocal.c.seg.a, 1);
		colWorld.c.seg.b = matModel * glm::vec4(colLocal.c.seg.b, 1);
		colWorld.c.r = colLocal.c.r;
		break;
	}
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


/**
* �Փ˔�����s��.
*
* @param a			�Փ˔���̃A�N�^�[����1.
* @param b			�Փ˔���̃A�N�^�[����2.
* @param handler	�Փ˂����ꍇ�Ɏ��s�����֐�.
*/
void DetectCollision(const ActorPtr& a, const ActorPtr& b, CollisionHandlerType handler) 
{
	if (a->health <= 0 || b->health <= 0) {
		return;
	}
	/*if (Collision::TestSphereSphere(a->colWorld, b->colWorld)) {
		handler(a, b, b->colWorld.center);
	}*/
	glm::vec3 pa, pb;
	if (Collision::TestShapeShape(a->colWorld, b->colWorld, &pa, &pb)) {
		handler(a, b, pb);
	}
}

/**
* �Փ˔�����s��.
*
* @param a			�Փ˔���̃A�N�^�[.
* @param b			�Փ˔���̃A�N�^�[���X�g.
* @param handler	�Փ˂����ꍇ�Ɏ��s�����֐�.
*/
void DetectCollision(const ActorPtr& a, ActorList& b, CollisionHandlerType handler)
{
	if (a->health <= 0) {
		return;
	}
	for (const ActorPtr& actorB : b) {
		if (actorB->health <= 0) {
			continue;
		}
		/*if (Collision::TestSphereSphere(a->colWorld, actorB->colWorld)) {
			handler(a, actorB, actorB->colWorld.center);*/
		glm::vec3 pa, pb;
		if (Collision::TestShapeShape(a->colWorld, actorB->colWorld, &pa, &pb)) {
			handler(a, actorB, pb);
			if (a->health <= 0) {
				break;
			}
		}
	}
}

/**
* �Փ˔�����s��.
*
* @param a			�Փ˔���̃A�N�^�[���X�g����1.
* @param b			�Փ˔���̃A�N�^�[���X�g����2.
* @param handler	�Փ˂����ꍇ�Ɏ��s�����֐�.
*/
void DetectCollision(ActorList& a, ActorList& b, CollisionHandlerType handler)
{
	for (const ActorPtr& actorA : a) {
		if (actorA->health <= 0) {
			continue;
		}
		for (const ActorPtr& actorB : b) {
			if (actorB->health <= 0) {
				continue;
			}
			/*if (Collision::TestSphereSphere(actorA->colWorld, actorB->colWorld)) {
				handler(actorA, actorB, actorB->colWorld.center);*/
			glm::vec3 pa, pb;
			if (Collision::TestShapeShape(actorA->colWorld, actorB->colWorld, &pa, &pb)) {
				handler(actorA, actorB, pb);
				if (actorA->health <= 0) {
					break;
				}
			}
		}
	}
}
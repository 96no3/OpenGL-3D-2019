/**
* @file JizoActor.cpp
*/
#include "JizoActor.h"
#include "MainGameScene.h"

/**
* �R���X�g���N�^.
*/
JizoActor::JizoActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id, MainGameScene* p)
	:StaticMeshActor(m,"Jizo",100,pos,glm::vec3(0),glm::vec3(1)),id(id),parent(p)
{
	colLocal = Collision::CreateCapsule(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), 0.5f);
}

/**
* �Փ˔���.
*/
void JizoActor::OnHit(const ActorPtr& other, const glm::vec3& p) 
{
	// �M�~�b�N�����ς݂̏ꍇ�͉������Ȃ�.
	if (isActivated) {
		return;
	}
	// �N���ɐ���������N���t���O��true�ɂ���.
	if (parent->HandleJizoEffects(id, position)) {
		isActivated = true;
	}	
}
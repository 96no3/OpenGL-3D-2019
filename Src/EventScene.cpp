/**
* @file EventScene.cpp
*/
#include "EventScene.h"
#include "EventScript.h"

/**
* �R���X�g���N�^.
*
* @param filename �X�v���N�g�t�@�C����.
*/
EventScene::EventScene(const char* filename) : Scene("EventScene"), filename(filename)
{

}

/**
* �C�x���g�V�[��������������.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool EventScene::Initialize()
{
	return EventScriptEngine::Instance().RunScript(filename.c_str());
}

/**
* �C�x���g�V�[���̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void EventScene::Update(float deltaTime)
{
	if (EventScriptEngine::Instance().IsFinished()) {
		SceneStack::Instance().Pop();
	}
}
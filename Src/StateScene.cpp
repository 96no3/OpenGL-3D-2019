/**
* @file StateScene.cpp
*/
#include "StateScene.h"

///**
//* �R���X�g���N�^.
//*/
//StateScene::StateScene() :Scene("StateScene") {}

/**
* �v���C���[�̓��͂���������.
*/
void StateScene::ProcessInput() {
	SceneStack::Instance().Pop();
}

///**
//* �V�[�����X�V����.
//*
//* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
//*/
//void StateScene::Update(float deltaTime) {
//}
//
///**
//* �V�[����`�悷��.
//*/
//void StateScene::Render() {
//}
//
///**
//* �V�[����j������.
//*/
//void StateScene::Finalize() {
//}
/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "StateScene.h"
#include "GameOverScene.h"

///**
//* �R���X�g���N�^.
//*/
//MainGameScene::MainGameScene() :Scene("MainGameScene") {}

/**
* �v���C���[�̓��͂���������.
*/
void MainGameScene::ProcessInput() {
	if (!flag) {
		flag = true;
		SceneStack::Instance().Push(std::make_shared<StateScene>());
	}
	else {
		SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
	}
	
}

///**
//* �V�[�����X�V����.
//*
//* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
//*/
//void MainGameScene::Update(float deltaTime) {
//}
//
///**
//* �V�[����`�悷��.
//*/
//void MainGameScene::Render() {
//}
//
///**
//* �V�[����j������.
//*/
//void MainGameScene::Finalize() {
//}
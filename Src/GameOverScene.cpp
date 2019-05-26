/**
* @file GameOverScene.cpp
*/
#include "GameOverScene.h"
#include "TitleScene.h"
#include "GLFWEW.h"

///**
//* �R���X�g���N�^.
//*/
//GameOverScene::GameOverScene() :Scene("GameOverScene") {}

/**
* �v���C���[�̓��͂���������.
*/
void GameOverScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::START) {
		SceneStack::Instance().Replace(std::make_shared<TitleScene>());
	}
}

///**
//* �V�[�����X�V����.
//*
//* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
//*/
//void GameOverScene::Update(float deltaTime) {
//}
//
///**
//* �V�[����`�悷��.
//*/
//void GameOverScene::Render() {
//}
//
///**
//* �V�[����j������.
//*/
//void GameOverScene::Finalize() {
//}
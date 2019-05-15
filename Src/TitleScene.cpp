/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainGameScene.h"
#include "GLFWEW.h"

///**
//* �R���X�g���N�^.
//*/
//TitleScene::TitleScene() :Scene("TitleScene") {}

/**
* �v���C���[�̓��͂���������.
*/
void TitleScene::ProcessInput() {
	GLFWEW::Window & window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::START) {
		SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
	}
}

///**
//* �V�[�����X�V����.
//*
//* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
//*/
//void TitleScene::Update(float deltaTime) {
//}
//
///**
//* �V�[����`�悷��.
//*/
//void TitleScene::Render() {
//}
//
///**
//* �V�[����j������.
//*/
//void TitleScene::Finalize() {
//}
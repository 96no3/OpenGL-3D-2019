/**
* @file StatusScene.cpp
*/
#include "StatusScene.h"
#include "GLFWEW.h"

///**
//* �R���X�g���N�^.
//*/
//StatusScene::StatusScene() :Scene("StatusScene") {}

/**
* �v���C���[�̓��͂���������.
*/
void StatusScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::X) {
		SceneStack::Instance().Pop();
	}
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
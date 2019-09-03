/**
* @file StatusScene.cpp
*/
#include "StatusScene.h"
#include "GLFWEW.h"
#include "Audio/Audio.h"

/**
* �R���X�g���N�^.
*/
StatusScene::StatusScene() :Scene("StatusScene") {

}

/**
* �V�[��������������.
*
* @retval true  ����������.
* @retval false ���������s. �Q�[���i�s�s�ɂ��A�v���O�������I�����邱��.
*/
bool StatusScene::Initialize()
{
	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/Fonts/font.fnt");

	return true;
}

/**
* �v���C���[�̓��͂���������.
*/
void StatusScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::X) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/ClickNo.wav")->Play();
		SceneStack::Instance().Pop();
	}
}

/**
* �V�[�����X�V����.
*
* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
*/
void StatusScene::Update(float deltaTime) {
	fontRenderer.BeginUpdate();
	fontRenderer.AddString(glm::vec2(-600, 0), L"�X�e�[�^�X���");
	fontRenderer.EndUpdate();
}

/**
* �V�[����`�悷��.
*/
void StatusScene::Render() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	fontRenderer.Draw(screenSize);
}

///**
//* �V�[����j������.
//*/
//void StateScene::Finalize() {
//}
/**
* @file GameClearScene.cpp
*/
#include "GameClearScene.h"
#include "TitleScene.h"
#include "GLFWEW.h"

///**
//* �R���X�g���N�^.
//*/
//GameClearScene::GameClearScene() :Scene("GameClearScene") {}

/**
* �V�[��������������.
*
* @retval true  ����������.
* @retval false ���������s. �Q�[���i�s�s�ɂ��A�v���O�������I�����邱��.
*/
bool GameClearScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Shaders/Sprite.vert", "Res/Shaders/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/Images/clearBG.tga"));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/Fonts/font.fnt");

	// BGM���Đ�����.
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/BGM/clearBGM.wav");
	bgm->Play(Audio::Flag_Loop);

	return true;
}

/**
* �v���C���[�̓��͂���������.
*/
void GameClearScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (timer <= 0 && (window.GetGamePad().buttonDown & GamePad::START)) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/toTitle.wav")->Play();
		timer = 1.0f;
	}
}

/**
* �V�[�����X�V����.
*
* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
*/
void GameClearScene::Update(float deltaTime)
{
	spriteRenderer.BeginUpdate();
	for (const Sprite& e : sprites)
	{
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();
	fontRenderer.BeginUpdate();
	fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHeight), L"�Q�[���N���A���");
	fontRenderer.EndUpdate();

	// �V�[���؂�ւ��҂�.
	if (timer > 0) {
		timer -= deltaTime;
		if (timer <= 0) {
			timer = 0;
			bgm->Stop();
			SceneStack::Instance().Replace(std::make_shared<TitleScene>());
			return;
		}
	}
}

/**
* �V�[����`�悷��.
*/
void GameClearScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}

///**
//* �V�[����j������.
//*/
//void GameClearScene::Finalize() {
//}
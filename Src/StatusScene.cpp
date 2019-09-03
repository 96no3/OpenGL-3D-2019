/**
* @file StatusScene.cpp
*/
#include "StatusScene.h"
#include "GLFWEW.h"
#include "Audio/Audio.h"

/**
* コンストラクタ.
*/
StatusScene::StatusScene() :Scene("StatusScene") {

}

/**
* シーンを初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗. ゲーム進行不可につき、プログラムを終了すること.
*/
bool StatusScene::Initialize()
{
	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/Fonts/font.fnt");

	return true;
}

/**
* プレイヤーの入力を処理する.
*/
void StatusScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::X) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/ClickNo.wav")->Play();
		SceneStack::Instance().Pop();
	}
}

/**
* シーンを更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void StatusScene::Update(float deltaTime) {
	fontRenderer.BeginUpdate();
	fontRenderer.AddString(glm::vec2(-600, 0), L"ステータス画面");
	fontRenderer.EndUpdate();
}

/**
* シーンを描画する.
*/
void StatusScene::Render() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	fontRenderer.Draw(screenSize);
}

///**
//* シーンを破棄する.
//*/
//void StateScene::Finalize() {
//}
/**
* @file StatusScene.cpp
*/
#include "StatusScene.h"
#include "GLFWEW.h"

///**
//* コンストラクタ.
//*/
//StatusScene::StatusScene() :Scene("StatusScene") {}

/**
* プレイヤーの入力を処理する.
*/
void StatusScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (window.GetGamePad().buttonDown & GamePad::X) {
		SceneStack::Instance().Pop();
	}
}

///**
//* シーンを更新する.
//*
//* @param deltaTime  前回の更新からの経過時間(秒).
//*/
//void StateScene::Update(float deltaTime) {
//}
//
///**
//* シーンを描画する.
//*/
//void StateScene::Render() {
//}
//
///**
//* シーンを破棄する.
//*/
//void StateScene::Finalize() {
//}
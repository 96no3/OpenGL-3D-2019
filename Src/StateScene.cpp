/**
* @file StateScene.cpp
*/
#include "StateScene.h"

///**
//* コンストラクタ.
//*/
//StateScene::StateScene() :Scene("StateScene") {}

/**
* プレイヤーの入力を処理する.
*/
void StateScene::ProcessInput() {
	SceneStack::Instance().Pop();
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
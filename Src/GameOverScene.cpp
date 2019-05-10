/**
* @file GameOverScene.cpp
*/
#include "GameOverScene.h"
#include "TitleScene.h"

///**
//* コンストラクタ.
//*/
//GameOverScene::GameOverScene() :Scene("GameOverScene") {}

/**
* プレイヤーの入力を処理する.
*/
void GameOverScene::ProcessInput() {
	SceneStack::Instance().Replace(std::make_shared<TitleScene>());	
}

///**
//* シーンを更新する.
//*
//* @param deltaTime  前回の更新からの経過時間(秒).
//*/
//void GameOverScene::Update(float deltaTime) {
//}
//
///**
//* シーンを描画する.
//*/
//void GameOverScene::Render() {
//}
//
///**
//* シーンを破棄する.
//*/
//void GameOverScene::Finalize() {
//}
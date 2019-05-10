/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainGameScene.h"

///**
//* コンストラクタ.
//*/
//TitleScene::TitleScene() :Scene("TitleScene") {}

/**
* プレイヤーの入力を処理する.
*/
void TitleScene::ProcessInput() {
	SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
}

///**
//* シーンを更新する.
//*
//* @param deltaTime  前回の更新からの経過時間(秒).
//*/
//void TitleScene::Update(float deltaTime) {
//}
//
///**
//* シーンを描画する.
//*/
//void TitleScene::Render() {
//}
//
///**
//* シーンを破棄する.
//*/
//void TitleScene::Finalize() {
//}
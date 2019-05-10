/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "StateScene.h"
#include "GameOverScene.h"

///**
//* コンストラクタ.
//*/
//MainGameScene::MainGameScene() :Scene("MainGameScene") {}

/**
* プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput() {
	if (!flag) {
		flag = true;
		SceneStack::Instance().Push(std::make_shared<StateScene>());
	}
	else {
		SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
	}
	
}

///**
//* シーンを更新する.
//*
//* @param deltaTime  前回の更新からの経過時間(秒).
//*/
//void MainGameScene::Update(float deltaTime) {
//}
//
///**
//* シーンを描画する.
//*/
//void MainGameScene::Render() {
//}
//
///**
//* シーンを破棄する.
//*/
//void MainGameScene::Finalize() {
//}
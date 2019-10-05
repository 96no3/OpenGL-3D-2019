/**
* @file Main.cpp
*/
#include <Windows.h>
#include "GLFWEW.h"
#include "TitleScene.h"
#include "SkeletalMesh/SkeletalMesh.h"
#include "Audio/Audio.h"

int main() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1280, 720, u8"アクションゲーム");

	// 音声再生プログラムを初期化する.
	Audio::Engine& audioEngine = Audio::Engine::Instance();
	if (!audioEngine.Initialize()) {
		return 1;
	}

	// スケルタル・アニメーションを利用可能にする.
	Mesh::SkeletalAnimation::Initialize();

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	window.InitTimer();
	while (!window.ShouldClose()) {
		const float deltaTime = static_cast<float>(window.DeltaTime());
		window.UpdateTimer();

		// スケルタル・アニメーション用データの作成準備.
		Mesh::SkeletalAnimation::ResetUniformData();

		// ESCキーが押されたら終了ウィンドウを表示.
		if (window.GetGamePad().buttonDown & GamePad::GUIDE) {
			if (MessageBox(nullptr, "ゲームを終了しますか？", "終了", MB_OKCANCEL) == IDOK) {
				break;
			}
		}

		sceneStack.Update(deltaTime);

		// スケルタル・アニメーション用データをGPUメモリに転送.
		Mesh::SkeletalAnimation::UploadUniformData();

		// 音声再生プログラムを更新する.
		audioEngine.Update();

		// バックバッファを消去する.
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// GLコンテキストのパラメーターを設定.
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		sceneStack.Render();
		window.SwapBuffers();
	}

	// スケルタル・アニメーションの利用を終了する.
	Mesh::SkeletalAnimation::Finalize();

	// 音声再生プログラムを終了する.
	audioEngine.Finalize();
}

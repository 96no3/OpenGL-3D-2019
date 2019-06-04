/**
* @file Main.cpp
*/
#include "TitleScene.h"
#include "GLFWEW.h"
#include <Windows.h>

int main() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1280, 720, u8"�A�N�V�����Q�[��");

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	window.InitTimer();
	while (!window.ShouldClose()) {		
		window.UpdateTimer();

		// ESC�L�[�������ꂽ��I���E�B���h�E��\��.
		if (window.GetGamePad().buttonDown & GamePad::GUIDE) {
			if (MessageBox(nullptr, "�Q�[�����I�����܂����H", "�I��", MB_OKCANCEL) == IDOK) {
				break;
			}
		}

		const float deltaTime = window.DeltaTime();
		sceneStack.Update(deltaTime);

		// �o�b�N�o�b�t�@����������.
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// GL�R���e�L�X�g�̃p�����[�^�[��ݒ�.
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		sceneStack.Render();
		window.SwapBuffers();
	}
}

/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include <Windows.h>
#include "TitleScene.h"
#include "SkeletalMesh/SkeletalMesh.h"

int main() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1280, 720, u8"�A�N�V�����Q�[��");

	// �X�P���^���E�A�j���[�V�����𗘗p�\�ɂ���.
	Mesh::SkeletalAnimation::Initialize();

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	window.InitTimer();
	while (!window.ShouldClose()) {
		const float deltaTime = window.DeltaTime();
		window.UpdateTimer();

		// �X�P���^���E�A�j���[�V�����p�f�[�^�̍쐬����.
		Mesh::SkeletalAnimation::ResetUniformData();

		// ESC�L�[�������ꂽ��I���E�B���h�E��\��.
		if (window.GetGamePad().buttonDown & GamePad::GUIDE) {
			if (MessageBox(nullptr, "�Q�[�����I�����܂����H", "�I��", MB_OKCANCEL) == IDOK) {
				break;
			}
		}

		sceneStack.Update(deltaTime);

		// �X�P���^���E�A�j���[�V�����p�f�[�^��GPU�������ɓ]��.
		Mesh::SkeletalAnimation::UploadUniformData();

		// �o�b�N�o�b�t�@����������.
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// GL�R���e�L�X�g�̃p�����[�^�[��ݒ�.
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		sceneStack.Render();
		window.SwapBuffers();
	}

	// �X�P���^���E�A�j���[�V�����̗��p���I������.
	Mesh::SkeletalAnimation::Finalize();
}

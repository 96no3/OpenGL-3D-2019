/**
* @file Main.cpp
*/
#include "TitleScene.h"
#include "GLFWEW.h"

int main() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1280, 720, u8"アクションゲーム");

	SceneStack& sceneStack = SceneStack::Instance();
	sceneStack.Push(std::make_shared<TitleScene>());

	window.InitTimer();
	while (!window.ShouldClose()) {
		const float deltaTime = window.DeltaTime();
		window.UpdateTimer();
		
		sceneStack.Update(deltaTime);

		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		sceneStack.Render();
		window.SwapBuffers();
	}
}

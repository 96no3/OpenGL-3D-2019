/**
* @file MainGameScene.cpp
*/
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GameOverScene.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>

///**
//* コンストラクタ.
//*/
//MainGameScene::MainGameScene() :Scene("MainGameScene") {}

/**
* シーンを初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗. ゲーム進行不可につき、プログラムを終了すること.
*/
bool MainGameScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Shaders/Sprite.vert", "Res/Shaders/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/Images/TitleBack2.tga"));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/Fonts/font.fnt");

	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));
	meshBuffer.LoadMesh("Res/Models/red_pine_tree.gltf");

	// ハイトマップを作成する.
	if (!heightMap.LoadFromFile("Res/Images/Terrain00.tga", 20.0f, 0.5f)) {
		return false;
	}
	if (!heightMap.CreateMesh(meshBuffer,"Terrain01")) {
		return false;
	}

	return true;
}


/**
* プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	// カメラ操作.
	const GamePad gamepad = window.GetGamePad();
	glm::vec3 velocity(0);
	if (gamepad.buttons & GamePad::DPAD_LEFT) {
		velocity.x = -1;
	}
	else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
		velocity.x = 1;
	}
	if (gamepad.buttons & GamePad::DPAD_DOWN) {
		velocity.z = 1;
	}
	else if (gamepad.buttons & GamePad::DPAD_UP) {
		velocity.z = -1;
	}
	if (velocity.x || velocity.z) {
		velocity = normalize(velocity) * 20.0f;
	}
	camera.velocity = velocity;

	if (window.GetGamePad().buttonDown & GamePad::X) {
		SceneStack::Instance().Push(std::make_shared<StatusScene>());
	}
	else if (window.GetGamePad().buttonDown & GamePad::START) {
		SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
	}
	
	/*if (!flag) {
		flag = true;
		SceneStack::Instance().Push(std::make_shared<StateScene>());
	}
	else {
		SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
	}*/
	
}

/**
* シーンを更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void MainGameScene::Update(float deltaTime)
{
	// カメラの状態を更新.
	if (dot(camera.velocity, camera.velocity)) {
		camera.target += camera.velocity * deltaTime;
		camera.target.y = heightMap.Height(camera.target);
		camera.position = camera.target + glm::vec3(0, 50, 50);
	}

	spriteRenderer.BeginUpdate();
	for (const Sprite& e : sprites)
	{
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();
	fontRenderer.BeginUpdate();
	fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHeight), L"メインゲーム画面");
	fontRenderer.EndUpdate();
}

/**
* シーンを描画する.
*/
void MainGameScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);

	/*const glm::vec3 cameraPos(-5, 3, 5);
	const glm::vec3 targetPos(0, 0, 0);*/
	
	/*const glm::vec3 targetPos(100, 0, 100);
	const glm::vec3 cameraPos = targetPos + glm::vec3(0, 200, 200);
	const glm::mat4 matView = glm::lookAt(cameraPos, targetPos, glm::vec3(0, 1, 0));*/

	const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	const float aspectRatio = static_cast<float>(window.Width()) / static_cast<float>(window.Height());
	const glm::mat4 matProj = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 1000.0f);
	//const glm::mat4 matModel(1);
	glm::vec3 cubePos(0, 0, 0);
	cubePos.y = heightMap.Height(cubePos);
	const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);
	meshBuffer.SetViewProjectionMatrix(matProj * matView);
	//Mesh::Draw(meshBuffer.GetFile("Cube"), matProj * matView, matModel);
	Mesh::Draw(meshBuffer.GetFile("Cube"), matModel);
	Mesh::Draw(meshBuffer.GetFile("Terrain01"), glm::mat4(1));

	glm::vec3 treePos(110, 0, 110);
	treePos.y = heightMap.Height(treePos);
	const glm::mat4 matTreeModel = glm::translate(glm::mat4(1), treePos) * glm::scale(glm::mat4(1), glm::vec3(3));
	Mesh::Draw(meshBuffer.GetFile("Res/Models/red_pine_tree.gltf"), matTreeModel);
}
//
///**
//* シーンを破棄する.
//*/
//void MainGameScene::Finalize() {
//}
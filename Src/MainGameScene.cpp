/**
* @file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GameOverScene.h"
#include "SkeletalMesh/SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
//#include <random>

/**
* 衝突を解決する.
*
* @param a	衝突したアクターその1.
* @param b	衝突したアクターその2.
* @param p	衝突位置.
*/
void PlayerCollisionHandler(const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) 
{
	const glm::vec3 v = a->colWorld.s.center - p;
	// 衝突位置との距離が近すぎないか調べる.
	if (dot(v, v) > FLT_EPSILON) {
		// aをbに重ならない位置まで移動.
		const glm::vec3 vn = normalize(v);
		/*const float radiusSum = a->colWorld.r + b->colWorld.r;*/
		float radiusSum = a->colWorld.s.r;

		switch (b->colWorld.type)
		{
		case Collision::Shape::Type::sphere:
			radiusSum += b->colWorld.s.r;
			break;
		case Collision::Shape::Type::capsule:
			radiusSum += b->colWorld.c.r;
			break;
		}

		const float distance = radiusSum - glm::length(v) + 0.01f;
		a->position += vn * distance;
		a->colWorld.s.center += vn * distance;

		if (a->velocity.y < 0 && vn.y >= glm::cos(glm::radians(60.0f))) {
			a->velocity.y = 0;			
		}
	}
	else {
		// 移動を取り消す（距離が近すぎる場合の例外処理）.
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = a->velocity * deltaTime;
		a->position -= deltaVelocity;
		a->colWorld.s.center -= deltaVelocity;
	}
}

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

	// BGMを再生する.
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/BGM/gameBGM.wav");
	bgm->Play(Audio::Flag_Loop);

	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));
	meshBuffer.LoadMesh("Res/Models/red_pine_tree.gltf");
	meshBuffer.LoadMesh("Res/Models/jizo_statue.gltf");
	//meshBuffer.LoadMesh("Res/Models/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/Models/bikuni.gltf");
	//meshBuffer.LoadMesh("Res/Models/oni_small.gltf");
	meshBuffer.LoadSkeletalMesh("Res/Models/oni_small.gltf");
	meshBuffer.LoadMesh("Res/Models/wall_stone.gltf");

	// ハイトマップを作成する.
	if (!heightMap.LoadFromFile("Res/Images/Terrain00.tga", 20.0f, 0.5f)) {
		return false;
	}
	if (!heightMap.CreateMesh(meshBuffer,"Terrain01")) {
		return false;
	}

	glm::vec3 startPos(100, 0, 100);
	startPos.y = heightMap.Height(startPos);
	/*player = std::make_shared<StaticMeshActor>(meshBuffer.GetFile("Res/Models/bikuni.gltf"), "player", 20, startPos);*/
	//player = std::make_shared<PlayerActor>(meshBuffer.GetFile("Res/Models/bikuni.gltf"), startPos, glm::vec3(0), &heightMap);
	//player->colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
	player = std::make_shared<PlayerActor>(&heightMap, meshBuffer, startPos);

	//std::mt19937 rand;
	rand.seed(0);
	const float pi = 3.14f;

	// 敵を配置.
	{
		const size_t oniCount = 100;
		enemies.Reserve(oniCount);
		//const Mesh::FilePtr mesh = meshBuffer.GetFile("Res/Models/oni_small.gltf");
#if 0
		for (size_t i = 0; i < oniCount; ++i) {
			// 敵の位置を(50,50)-(150,150)の範囲からランダムに選択.
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(50, 150)(rand);
			position.z = std::uniform_real_distribution<float>(50, 150)(rand);
			position.y = heightMap.Height(position);
			// 敵の向きをランダムに選択.
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, 6.3f)(rand);
			//StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(mesh, "kooni", 13, position, rotation);
			const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(mesh, "kooni", 13, position, rotation);
			p->GetMesh()->Play("Run");
			//p->colLocal = Collision::Sphere{ glm::vec3(0), 1.0f };
			p->colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			enemies.Add(p);
		}
#endif
	}

	// 木を配置.
	{
		const size_t treeCount = 1000;
		trees.Reserve(treeCount);
		const Mesh::FilePtr mesh = meshBuffer.GetFile("Res/Models/red_pine_tree.gltf");
		for (size_t i = 0; i < treeCount; ++i) {
			// 木の位置を(0,0)-(200,200)の範囲からランダムに選択.
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(0, 200)(rand);
			position.z = std::uniform_real_distribution<float>(0, 200)(rand);
			position.y = heightMap.Height(position);
			// 木の向きをランダムに選択.
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, 6.3f)(rand);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(mesh, "tree", 1, position, rotation, glm::vec3(2));
			//p->colLocal = Collision::Sphere{ glm::vec3(0), 0.5f };
			p->colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 7.5f, 0), 0.5f);
			trees.Add(p);
		}
	}

	// お地蔵様を配置.
	{		
		const size_t objectCount = 4;
		//objects.Reserve(objectCount);
		const Mesh::FilePtr meshJizoStatue = meshBuffer.GetFile("Res/Models/jizo_statue.gltf");

		for (size_t i = 0; i < objectCount; ++i) {
			// お地蔵様の位置を(0,0)-(50,150)の範囲からランダムに選択.
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(50, 150)(rand);
			position.z = std::uniform_real_distribution<float>(50, 150)(rand);
			position.y = heightMap.Height(position);
			// お地蔵様の向きをランダムに選択.
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, pi*2.0f)(rand);
			JizoActorPtr p = std::make_shared<JizoActor>(meshJizoStatue, position, i, this);
			p->scale = glm::vec3(3); // 見つけやすいように拡大.
			objects.Add(p);
		}
	}

	// 石壁を配置.
	{
		//const float pi = 3.14f;
		/*const size_t objectCount = 1000;
		objects.Reserve(objectCount);*/
		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Models/wall_stone.gltf");

		glm::vec3 position = startPos + glm::vec3(0, 0, -65);
		position.y = heightMap.Height(position);		
		glm::vec3 rotation = glm::vec3(0, pi * 0.5f, 0);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(meshStoneWall, "StoneWall", 100, position, rotation, glm::vec3(4));
		glm::vec3 rectSize = glm::vec3(2, 2, 0.5f) * 4.0f;
		p->colLocal = Collision::CreateOBB(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), rectSize);
		objects.Add(p);

		//for (size_t i = 0; i < objectCount; ++i) {
		//	// 石壁の位置を(0,0)-(200,200)の範囲からランダムに選択.
		//	glm::vec3 position(0);
		//	position.x = std::uniform_real_distribution<float>(0, 200)(rand);
		//	position.z = std::uniform_real_distribution<float>(0, 200)(rand);
		//	position.y = heightMap.Height(position);
		//	// 石壁の向きをランダムに選択.
		//	glm::vec3 rotation(0);
		//	rotation.y = std::uniform_real_distribution<float>(0, 6.3f)(rand);
		//	StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(mesh, "tree", 1, position, rotation, glm::vec3(2));
		//	//p->colLocal = Collision::Sphere{ glm::vec3(0), 0.5f };
		//	p->colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 7.5f, 0), 0.5f);
		//	objects.Add(p);
		//}
	}

	return true;
}


/**
* プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	// プレイヤー操作.
	//const GamePad gamepad = window.GetGamePad();
	//glm::vec3 velocity(0);
	//if (gamepad.buttons & GamePad::DPAD_LEFT) {
	//	velocity.x = -1;
	//}
	//else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
	//	velocity.x = 1;
	//}
	//if (gamepad.buttons & GamePad::DPAD_DOWN) {
	//	velocity.z = 1;
	//}
	//else if (gamepad.buttons & GamePad::DPAD_UP) {
	//	velocity.z = -1;
	//}
	//if (velocity.x || velocity.z) {
	//	velocity = normalize(velocity);
	//	player->rotation.y = std::atan2(-velocity.z, velocity.x) + glm::radians(90.0f);
	//	velocity *= 6.0f;
	//}
	////player->velocity = velocity;
	//player->velocity.x = velocity.x;
	//player->velocity.z = velocity.z;

	//// ジャンプ.
	//if (gamepad.buttonDown && GamePad::B) {
	//	player->jump();
	//}
	player->ProcessInput();

	if (window.GetGamePad().buttonDown & GamePad::X) {
		SceneStack::Instance().Push(std::make_shared<StatusScene>());
		Audio::Engine::Instance().Prepare("Res/Audio/SE/CliclOk.wav")->Play();
	}
	else if (timer <= 0 && (window.GetGamePad().buttonDown & GamePad::START)) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/Click2.wav")->Play();
		timer = 1.0f;
		//SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
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
	/*if (dot(camera.velocity, camera.velocity)) {
		camera.target += camera.velocity * deltaTime;
		camera.target.y = heightMap.Height(camera.target);*/		
	{
		camera.target = player->position;
		//camera.position = camera.target + glm::vec3(0, 50, 50);
		camera.position = camera.target + glm::vec3(0, 10, 10);
	}

	// Actorの状態を更新.
	player->Update(deltaTime);
	enemies.Update(deltaTime);
	trees.Update(deltaTime);
	objects.Update(deltaTime);

	//player->position.y = heightMap.Height(player->position);
	/*DetectCollision(player, enemies, PlayerCollisionHandler);
	DetectCollision(player, trees, PlayerCollisionHandler);
	DetectCollision(player, objects, PlayerCollisionHandler);*/
	DetectCollision(player, enemies);
	DetectCollision(player, trees);
	DetectCollision(player, objects);
	//player->position.y = heightMap.Height(player->position);

	player->UpdateDrawData(deltaTime);
	enemies.UpdateDrawData(deltaTime);
	trees.UpdateDrawData(deltaTime);
	objects.UpdateDrawData(deltaTime);

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

	// シーン切り替え待ち.
	if (timer > 0) {
		timer -= deltaTime;
		if (timer <= 0) {
			timer = 0;
			bgm->Stop();
			SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
			return;
		}
	}
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

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

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

	/*glm::vec3 treePos(110, 0, 110);
	treePos.y = heightMap.Height(treePos);
	const glm::mat4 matTreeModel = glm::translate(glm::mat4(1), treePos) * glm::scale(glm::mat4(1), glm::vec3(3));
	Mesh::Draw(meshBuffer.GetFile("Res/Models/red_pine_tree.gltf"), matTreeModel);*/

	player->Draw();
	enemies.Draw();
	objects.Draw();
	glDisable(GL_CULL_FACE);
	trees.Draw();	
}

/**
* シーンを初期化する.
*
* @param id  お地蔵様の番号.
* @param pos お地蔵様の座標.
*
* @retval true  処理成功.
* @retval false すでに戦闘中なので処理しなかった.
*/
bool MainGameScene::HandleJizoEffects(int id, const glm::vec3& pos) 
{
	if (jizoId >= 0) {
		return false;
	}
	jizoId = id;

	const size_t oniCount = 8;
	const float pi = 3.14f;
	for (size_t i = 0; i < oniCount; ++i) 
	{
		glm::vec3 position(pos);
		// 敵の位置を地蔵の位置から(-15,-15)-(15,15)の範囲でランダムに選択.
		position.x += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.z += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.y = heightMap.Height(position);
		// 敵の向きをランダムに選択.
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, pi * 2.0f)(rand);
		const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
		SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(mesh, "kooni", 13, position, rotation);
		p->GetMesh()->Play("Wait");
		p->colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
		enemies.Add(p);
	}
	return true;
}

//
///**
//* シーンを破棄する.
//*/
//void MainGameScene::Finalize() {
//}
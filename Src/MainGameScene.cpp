/**
* @file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "SkeletalMesh/SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

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
	meshBuffer.LoadSkeletalMesh("Res/Models/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/Models/oni_small.gltf");
	meshBuffer.LoadMesh("Res/Models/wall_stone.gltf");
	
	// ハイトマップを作成する.
	if (!heightMap.LoadFromFile("Res/Images/Terrain.tga", 20.0f, 0.5f)) {
		return false;
	}
	if (!heightMap.CreateMesh(meshBuffer,"Terrain01")) {
		return false;
	}	// 水面の高さは要調整
	if (!heightMap.CreateWaterMesh(meshBuffer, "Water",-6)) {
		return false;
	}

	lightBuffer.Init(1);
	lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
	lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
	lightBuffer.BindToShader(meshBuffer.GetWaterShader());

	// プレイヤーを配置
	glm::vec3 startPos(100, 0, 100);
	startPos.y = heightMap.Height(startPos);
	player = std::make_shared<PlayerActor>(&heightMap, meshBuffer, startPos);

	rand.seed(0);

	// ライトを配置
	lights.Add(std::make_shared<DirectionalLightActor>("DirectonalLight",
		glm::vec3(0.15f, 0.25f, 0.2f) * 4.0f, glm::normalize(glm::vec3(1, -1, -1))));

	for (int i = 0; i < 50; ++i) {
		glm::vec3 color(1, 0.8f, 0.5f);
		glm::vec3 position(0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.y = heightMap.Height(position) + 1;
		lights.Add(std::make_shared<PointLightActor>("PointLight", color, position));
	}

	for (int i = 0; i < 50; ++i) {
		glm::vec3 color(0, 5, 0);
		glm::vec3 position(0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.y = heightMap.Height(position) + 2;
		glm::vec3 direction(glm::normalize(glm::vec3(1, -1, 1)));
		lights.Add(std::make_shared<SpotLightActor>("SpotLight", color, position, direction,
			glm::radians(30.0f), glm::radians(20.0f)));
	}

	lights.Update(0);
	lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
	heightMap.UpdateLightIndex(lights);

	// 敵を配置.
	{
		const size_t oniCount = 100;
		enemies.Reserve(oniCount);
#if 0
		for (size_t i = 0; i < oniCount; ++i) {
			// 敵の位置を(50,50)-(150,150)の範囲からランダムに選択.
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(50, 150)(rand);
			position.z = std::uniform_real_distribution<float>(50, 150)(rand);
			position.y = heightMap.Height(position);
			// 敵の向きをランダムに選択.
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2)(rand);
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
		const size_t treeCount = 500;
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
			p->colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 7.5f, 0), 0.5f);
			trees.Add(p);
		}
	}

	// お地蔵様を配置.
	{		
		const size_t objectCount = 4;
		const Mesh::FilePtr meshJizoStatue = meshBuffer.GetFile("Res/Models/jizo_statue.gltf");

		for (size_t i = 0; i < objectCount; ++i) {
			// お地蔵様の位置を(0,0)-(50,150)の範囲からランダムに選択.
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(50, 150)(rand);
			position.z = std::uniform_real_distribution<float>(50, 150)(rand);
			position.y = heightMap.Height(position);
			// お地蔵様の向きをランダムに選択.
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2.0f)(rand);
			JizoActorPtr p = std::make_shared<JizoActor>(meshJizoStatue, position, i, this);
			p->scale = glm::vec3(3); // 見つけやすいように拡大.
			objects.Add(p);
		}
	}

	// 石壁を配置.
	{
		const int maze[27][25] = {
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,2},
			{2,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		};

		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Models/wall_stone.gltf");
		const glm::vec3 baseXPos(4, 0, 1);
		const glm::vec3 baseYPos(1, 0, -4);
		glm::vec3 rectSize = glm::vec3(2, 2, 0.5f) * 2.0f;
		glm::vec3 position = baseXPos;
		for (int y = 0; y < 27; ++y) {
			for (int x = 0; x < 25; ++x) {
				if (maze[y][x] & 1) {					
					if (y == 26) {
						position = baseXPos + glm::vec3(x * 8, 0, y * 8 - 10);
					}
					else {
						position = baseXPos + glm::vec3(x * 8, 0, y * 8);
					}					
					position.y = heightMap.Height(position) + 4;
					StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
						meshStoneWall, "StoneWall", 100, position, glm::vec3(0, 0, 0), glm::vec3(2));
					p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
						glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), rectSize);
					objects.Add(p);
				}
				if (maze[y][x] & 2) {
					if (x == 24) {
						position = baseYPos + glm::vec3(x * 8 + 6, 0, y * 8);
					}
					else {
						position = baseYPos + glm::vec3(x * 8, 0, y * 8);
					}					
					position.y = heightMap.Height(position) + 4;
					StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
						meshStoneWall, "StoneWall", 100, position, glm::vec3(0, glm::pi<float>() * 0.5f, 0), glm::vec3(2));
					p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
						glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), rectSize);
					objects.Add(p);
				}
			}
		}

		/*glm::vec3 position = baseYPos;
		position.y = heightMap.Height(position) + 4;
		glm::vec3 rotation = glm::vec3(0, glm::pi<float>() * 0.5f, 0);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(meshStoneWall, "StoneWall", 100, position, rotation, glm::vec3(2));
		glm::vec3 rectSize = glm::vec3(2, 2, 0.5f) * 2.0f;
		p->colLocal = Collision::CreateOBB(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), rectSize);
		objects.Add(p);*/
	}

	return true;
}


/**
* プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	player->ProcessInput();

	if (window.GetGamePad().buttonDown & GamePad::X) {
		SceneStack::Instance().Push(std::make_shared<StatusScene>());
		Audio::Engine::Instance().Prepare("Res/Audio/SE/CliclOk.wav")->Play();
	}
	else if (timer <= 0 && (window.GetGamePad().buttonDown & GamePad::START)) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/Click2.wav")->Play();
		timer = 1.0f;
	}	
}

/**
* シーンを更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void MainGameScene::Update(float deltaTime)
{
	// カメラの状態を更新.		
	{
		camera.target = player->position;
		camera.position = camera.target + glm::vec3(0, 20, 20);
	}

	// Actorの状態を更新.
	player->Update(deltaTime);
	enemies.Update(deltaTime);
	trees.Update(deltaTime);
	objects.Update(deltaTime);
	lights.Update(deltaTime);

	DetectCollision(player, enemies);
	DetectCollision(player, trees);
	DetectCollision(player, objects);

	// プレイヤーの攻撃判定.
	ActorPtr attackCollision = player->GetAttackCollision();
	if (attackCollision) {
		bool hit = false;
		DetectCollision(attackCollision, enemies,
						[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
							SkeletalMeshActorPtr bb = std::static_pointer_cast<SkeletalMeshActor>(b);
							bb->health -= a->health;
							if (bb->health <= 0) {
								bb->colLocal = Collision::Shape{};
								bb->health = 1;
								bb->GetMesh()->Play("Down", false);
							}
							else {
								bb->GetMesh()->Play("Hit", false);
							}
							hit = true;
						}
		);
		if (hit) {
			attackCollision->health = 0;
		}
	}

	// 死亡アニメーションの終わった敵を消す.
	for (auto& e : enemies) {
		SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
		Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();
		if (mesh->IsFinished()) {
			if (mesh->GetAnimation() == "Down") {
				enemy->health = 0;
			}
			else {
				mesh->Play("Run");
			}
		}
	}

	// ライトの更新.
	glm::vec3 ambientColor(0.1f, 0.05f, 0.15f);
	lightBuffer.Update(lights, ambientColor);
	for (auto e : trees) {
		const std::vector<ActorPtr> neighborhood = lights.FindNearbyActors(e->position, 20);
		std::vector<int> pointLightIndex;
		std::vector<int> spotLightIndex;
		pointLightIndex.reserve(neighborhood.size());
		spotLightIndex.reserve(neighborhood.size());

		for (auto light : neighborhood) {
			if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light)) {
				if (pointLightIndex.size() < 8) {
					pointLightIndex.push_back(p->index);
				}
			}
			else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light)) {
				if (spotLightIndex.size() < 8) {
					spotLightIndex.push_back(p->index);
				}
			}
		}
		StaticMeshActorPtr p = std::dynamic_pointer_cast<StaticMeshActor>(e);
		p->SetPointLightList(pointLightIndex);
		p->SetSpotLightList(spotLightIndex);
	}

	// 敵御全滅させたら目的達成フラグをtrueにする.
	if (jizoId >= 0) {
		if (enemies.Empty()) {
			achivements[jizoId] = true;
			jizoId = -1;
		}
	}
	// 全ての目的達成フラグがtrueになっていたらメッセージを表示.
	int achivedCount = 0;
	for (auto e : achivements) {
		if (e) {
			++achivedCount;
		}
	}
	if (isClear) {
		clearTimer -= deltaTime;
		if (clearTimer <= 0) {
			SceneStack::Instance().Replace(std::make_shared<GameClearScene>());
			return;
		}
	}
	else if (achivedCount >= 4) {
		Audio::Engine::Instance().Prepare("Res/Audio/SE/Result.wav")->Play();
		isClear = true;
		clearTimer = 3;
	}

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
	fontRenderer.Color(glm::vec4(0,1,1,1));
	if (isClear) {
		fontRenderer.AddString(glm::vec2(-32 * 4, 0), L"ゲームクリア！！");
	}
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

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	lightBuffer.Upload();
	lightBuffer.Bind();

	const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	const float aspectRatio = static_cast<float>(window.Width()) / static_cast<float>(window.Height());
	const glm::mat4 matProj = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 1000.0f);
	glm::vec3 cubePos(100, 0, 100);
	cubePos.y = heightMap.Height(cubePos);
	const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);
	meshBuffer.SetViewProjectionMatrix(matProj * matView);

	meshBuffer.SetCameraPosition(camera.position);
	meshBuffer.SetTime(window.Time());

	Mesh::Draw(meshBuffer.GetFile("Cube"), matModel);
	Mesh::Draw(meshBuffer.GetFile("Terrain01"), glm::mat4(1));	

	player->Draw();
	enemies.Draw();
	objects.Draw();
	glDisable(GL_CULL_FACE);
	trees.Draw();

	Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1)); // 半透明メッシュはできるだけ最後に描画

	fontRenderer.Draw(screenSize);
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
	for (size_t i = 0; i < oniCount; ++i) 
	{
		glm::vec3 position(pos);
		// 敵の位置を地蔵の位置から(-15,-15)-(15,15)の範囲でランダムに選択.
		position.x += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.z += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.y = heightMap.Height(position);
		// 敵の向きをランダムに選択.
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2.0f)(rand);
		const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
		SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(mesh, "kooni", 13, position, rotation);
		p->GetMesh()->Play("Run");
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
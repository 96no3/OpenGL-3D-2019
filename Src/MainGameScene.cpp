/**
* @file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "EventScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "SkeletalMesh/SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

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

	textWindow.Init("Res/Images/TextWindow.tga",
		glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0));
	textWindow.Open(L"テキストウィンドウの実験\nこれは改行テスト。\n3行目。\n4\n5\n6");

	// BGMを再生する.
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/BGM/gameBGM.wav");
	bgm->Play(Audio::Flag_Loop);

	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));	
	meshBuffer.LoadMesh("Res/Models/red_pine_tree.gltf");
	meshBuffer.LoadMesh("Res/Models/jizo_statue.gltf");
	meshBuffer.LoadSkeletalMesh("Res/Models/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/Models/oni_small.gltf");
	meshBuffer.LoadMesh("Res/Models/wall_stone.gltf");

	// パーティクル・システムを初期化する.
	particleSystem.Init(1000);

	// FBOを作成する.
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	fboMain = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
	Mesh::FilePtr rt = meshBuffer.AddPlane("RenderTarget");
	if (rt) {
		rt->materials[0].program = Shader::Program::Create(
			"Res/Shaders/DepthOfField.vert", "Res/Shaders/DepthOfField.frag");
		rt->materials[0].texture[0] = fboMain->GetColorTexture();
		rt->materials[0].texture[1] = fboMain->GetDepthTexture();
	}
	if (!rt || !rt->materials[0].program) {
		return false;
	}
	
	// DoF描画用のFBOを作る.
	fboDepthOfField = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
	
	// 元解像度の縦横1/2（面積が1/4）の大きさのブルーム用FBOを作る.
	int w = window.Width();
	int h = window.Height();
	for (int j = 0; j < sizeof(fboBloom) / sizeof(fboBloom[0]); ++j) {
		w /= 2;
		h /= 2;
		for (int i = 0; i < sizeof(fboBloom[0]) / sizeof(fboBloom[0][0]); ++i) {
			fboBloom[j][i] = FramebufferObject::Create(w, h, GL_RGBA16F, FrameBufferType::colorOnly);
			if (!fboBloom[j][i]) {
				return false;
			}
		}
	}

	// ブルーム・エフェクト用の平面ポリゴンメッシュを作成する.
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("BrightPassFilter")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Shaders/Simple.vert", "Res/Shaders/BrightPassFilter.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("NormalBlur")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Shaders/Simple.vert", "Res/Shaders/NomalBlur.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("Simple")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Shaders/Simple.vert", "Res/Shaders/Simple.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (glGetError()) {
		std::cout << "[エラー]" << __func__ << ": ブルーム用メッシュの作成に失敗.\n";
		return false;
	}

	// デプスシャドウマッピング用のFBOを作成する.
	{
		fboShadow = FramebufferObject::Create(4096, 4096, GL_NONE, FrameBufferType::depthOnly);
		if (glGetError()) {
			std::cout << "[エラー]" << __func__ << ":シャドウ用FBOの作成に失敗.\n";
			return false;
		}	
	}
	// sampler2DShadowの比較モードを設定する.
	glBindTexture(GL_TEXTURE_2D, fboShadow->GetDepthTexture()->Get());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// ボーダーカラーを使って、シャドウテクスチャの範囲外の深度値を最大(1.0)にすることで、
    // 範囲外の領域に不自然な影が落ちないようにする.
    // 奥行方向の範囲不足には対応できないので、near、farプレーンの設定は適切にすること.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const glm::vec3 borderColor(1.0);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor.x);

	glBindTexture(GL_TEXTURE_2D, 0);

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
		glm::vec3(1.0f, 0.94f, 0.91f), glm::normalize(glm::vec3(1, -1, -1))));

	for (int i = 0; i < 30; ++i) {
		glm::vec3 color = glm::vec3(1, 0.8f, 0.5f) * 20.0f;
		glm::vec3 position(0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.y = heightMap.Height(position) + 5;
		lights.Add(std::make_shared<PointLightActor>("PointLight", color, position));
	}

	for (int i = 0; i < 30; ++i) {
		glm::vec3 color = glm::vec3(0, 5, 0) * 25.0f;
		glm::vec3 position(0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.y = heightMap.Height(position) + 5;
		glm::vec3 direction(glm::normalize(glm::vec3(0.25f, -1, 0.25f)));
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

	// パーティクル・システムのテスト用にエミッターを追加.
	{
		// エミッター1個目.
		ParticleEmitterParameter ep;
		//ep.imagePath = "Res/Images/DiskParticle.tga";
		ep.imagePath = "Res/Images/FireParticle.tga";
		ep.tiles = glm::ivec2(2, 2);
		ep.position = glm::vec3(96.5f, 0, 95.0f);
		ep.position.y = heightMap.Height(ep.position);
		ep.emissionsPerSecond = 20.0f;
		ep.dstFactor = GL_ONE; // 加算合成.
		ep.gravity = 0;
		ParticleParameter pp;
		pp.scale = glm::vec2(0.5f);
		pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
		particleSystem.Add(ep, pp);
	}
	{
		// エミッター2個目.
		ParticleEmitterParameter ep;
		ep.imagePath = "Res/Images/DiskParticle.tga";
		ep.position = glm::vec3(97, 0, 100);
		ep.position.y = heightMap.Height(ep.position);
		ep.angle = glm::radians(30.0f);
		ParticleParameter pp;
		pp.scale = glm::vec2(0.2f);
		pp.velocity = glm::vec3(0, 3, 0);
		pp.color = glm::vec4(0.1f, 0.3f, 0.8f, 1.0f);
		particleSystem.Add(ep, pp);
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

	// オープニングスクリプトを実行.
	SceneStack::Instance().Push(std::make_shared<EventScene>("Res/EventScripts/OpeningScript.txt"));

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
		//camera.position = camera.target + glm::vec3(0, 20, 20);
		camera.position = camera.target + glm::vec3(0, 8, 13);
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

	// パーティクルの更新
	particleSystem.Update(deltaTime);

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

	textWindow.Update(deltaTime);

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
	//spriteRenderer.Draw(screenSize);

	// 影用FBOに描画.
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboShadow->GetFramebuffer());
		auto tex = fboShadow->GetDepthTexture();
		glViewport(0, 0, tex->Width(), tex->Height());
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		// ディレクショナル・ライトの向きから影用のビュー行列を作成.
		// 視点は、カメラの注視点からライトの方向に100ｍ移動した位置に設置する.
		glm::vec3 direction(0, -1, 0);
		for (auto e : lights) {
			if (auto p = std::dynamic_pointer_cast<DirectionalLightActor>(e)) {
				direction = p->direction;
				break;
			}
		}
		const glm::vec3 position = camera.target - direction * 100.0f;
		const glm::mat4 matView = glm::lookAt(position, camera.target, glm::vec3(0, 1, 0));

		// 平行投影によるプロジェクション行列を作成.
		const float width = 100;  // 描画範囲の幅.
		const float height = 100; // 描画範囲の高さ.
		const float near = 10.0f; // 描画範囲の手前側の境界.
		const float far = 200.0f; // 描画範囲の奥側の境界.
		const glm::mat4 matProj = glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2, near, far);

		// ビュー・プロジェクション行列を設定してメッシュを描画.
		meshBuffer.SetShadowViewProjectionMatrix(matProj * matView);
		//meshBuffer.SetViewProjectionMatrix(matProj * matView);
		RenderMesh(Mesh::DrawType::shadow);
	}
	
	lightBuffer.Upload();
	lightBuffer.Bind();

	// FBOに描画.
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain->GetFramebuffer());
	const auto texMain = fboMain->GetColorTexture();
	glViewport(0, 0, texMain->Width(), texMain->Height());

	glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	const float aspectRatio = static_cast<float>(window.Width()) / static_cast<float>(window.Height());
	//const glm::mat4 matProj = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 1000.0f);
	const glm::mat4 matProj = glm::perspective(camera.fov * 0.5f, aspectRatio, camera.near, camera.far);
	/*glm::vec3 cubePos(100, 0, 100);
	cubePos.y = heightMap.Height(cubePos);
	const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);*/
	meshBuffer.SetViewProjectionMatrix(matProj * matView);

	meshBuffer.SetCameraPosition(camera.position);
	meshBuffer.SetTime(window.Time());
	meshBuffer.BindShadowTexture(fboShadow->GetDepthTexture());

	/*Mesh::Draw(meshBuffer.GetFile("Cube"), matModel);
	Mesh::Draw(meshBuffer.GetFile("Terrain01"), glm::mat4(1));*/	

	/*player->Draw();
	enemies.Draw();
	objects.Draw();
	glDisable(GL_CULL_FACE);
	trees.Draw();*/

	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1)); // 半透明メッシュはできるだけ最後に描画

	RenderMesh(Mesh::DrawType::color);
	particleSystem.Draw(matProj, matView);

	meshBuffer.UnbindShadowTexture();

	// 被写界深度エフェクト.
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboDepthOfField->GetFramebuffer());
		const auto tex = fboDepthOfField->GetColorTexture();
		glViewport(0, 0, tex->Width(), tex->Height());

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		camera.Update(matView);

		Mesh::FilePtr mesh = meshBuffer.GetFile("RenderTarget");

		Shader::ProgramPtr prog = mesh->materials[0].program;
		prog->Use();
		prog->SetViewInfo(static_cast<float>(window.Width()),
			static_cast<float>(window.Height()), camera.near, camera.far);
		prog->SetCameraInfo(camera.focalPlane, camera.focalLength,
			camera.aperture, camera.sensorSize);

		Mesh::Draw(mesh, glm::mat4(1));
	}

	// ブルーム・エフェクト
	{
		// 明るい部分を取り出す.
		{
			auto tex = fboBloom[0][0]->GetColorTexture();
			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[0][0]->GetFramebuffer());
			glViewport(0, 0, tex->Width(), tex->Height());
			Mesh::FilePtr mesh = meshBuffer.GetFile("BrightPassFilter");
			mesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
			Mesh::Draw(mesh, glm::mat4(1));
		}

		// 縮小コピー.
		Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
		for (int i = 0; i < sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; ++i) {
			auto tex = fboBloom[i + 1][0]->GetColorTexture();
			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i + 1][0]->GetFramebuffer());
			glViewport(0, 0, tex->Width(), tex->Height());
			glClear(GL_COLOR_BUFFER_BIT);
			simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
			Mesh::Draw(simpleMesh, glm::mat4(1));
		}

		// ガウスぼかし.
		Mesh::FilePtr blurMesh = meshBuffer.GetFile("NormalBlur");
		Shader::ProgramPtr progBlur = blurMesh->materials[0].program;
		for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i >= 0; --i) {
			auto tex = fboBloom[i][0]->GetColorTexture();
			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][1]->GetFramebuffer());
			glViewport(0, 0, tex->Width(), tex->Height());
			glClear(GL_COLOR_BUFFER_BIT);
			progBlur->Use();
			progBlur->SetBlurDirection(1.0f / static_cast<float>(tex->Width()), 0.0f);
			blurMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
			Mesh::Draw(blurMesh, glm::mat4(1));

			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][0]->GetFramebuffer());
			glClear(GL_COLOR_BUFFER_BIT);
			progBlur->Use();
			progBlur->SetBlurDirection(0.0f, 1.0f / static_cast<float>(tex->Height()));
			blurMesh->materials[0].texture[0] = fboBloom[i][1]->GetColorTexture();
			Mesh::Draw(blurMesh, glm::mat4(1));
		}

		// 拡大＆加算合成.
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i > 0; --i) {
			auto tex = fboBloom[i - 1][0]->GetColorTexture();
			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i - 1][0]->GetFramebuffer());
			glViewport(0, 0, tex->Width(), tex->Height());
			simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
			Mesh::Draw(simpleMesh, glm::mat4(1));
		}
	}

	// 全てをデフォルト・フレームバッファに合成描画.
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.Width(), window.Height());

		const glm::vec2 screenSize(window.Width(), window.Height());
		spriteRenderer.Draw(screenSize);

		// 被写界深度エフェクト適用後の画像を描画.
		glDisable(GL_BLEND);
		Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
		simpleMesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
		Mesh::Draw(simpleMesh, glm::mat4(1));

		// 拡散光を描画.
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		simpleMesh->materials[0].texture[0] = fboBloom[0][0]->GetColorTexture();
		Mesh::Draw(simpleMesh, glm::mat4(1));

		textWindow.Draw();
		fontRenderer.Draw(screenSize);
	}
#if 0
	// デバッグのために、影用の深度テクスチャを表示する.
	{
		glDisable(GL_BLEND);
		Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
		simpleMesh->materials[0].texture[0] = fboShadow->GetDepthTexture();
		glm::mat4 m = glm::scale(
			glm::translate(glm::mat4(1), glm::vec3(-0.45f, 0, 0)),
			glm::vec3(0.5f, 0.89f, 1));
		Mesh::Draw(simpleMesh, m);
	}
#endif
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

/**
* カメラのパラメータを更新する.
*
* @param matView  更新に利用するビュー行列.
*/
void MainGameScene::Camera::Update(const glm::mat4& matView) 
{
	const glm::vec4 pos = matView * glm::vec4(target, 1);
	focalPlane = pos.z * -1000.0f;

	const float imageDistance = sensorSize * 0.5f / glm::tan(fov * 0.5f);
	focalLength = 1.0f / ((1.0f / focalPlane) + (1.0f / imageDistance));

	aperture = focalLength / fNumber;
}

/**
* メッシュを描画する.
*
* @param drawType 描画するデータの種類.
*/
void MainGameScene::RenderMesh(Mesh::DrawType drawType)
{
	glm::vec3 cubePos(100, 0, 100);
	cubePos.y = heightMap.Height(cubePos);
	const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);
	Mesh::Draw(meshBuffer.GetFile("Cube"), matModel, drawType);
	Mesh::Draw(meshBuffer.GetFile("Terrain01"), glm::mat4(1), drawType);

	player->Draw(drawType);
	enemies.Draw(drawType);
	objects.Draw(drawType);
	//glDisable(GL_CULL_FACE);
	trees.Draw(drawType);

	if (drawType == Mesh::DrawType::color) {
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1), drawType); // 半透明メッシュはできるだけ最後に描画
	}
}

//
///**
//* シーンを破棄する.
//*/
//void MainGameScene::Finalize() {
//}
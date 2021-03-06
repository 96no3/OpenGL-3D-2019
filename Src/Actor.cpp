/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

/**
* コンストラクタ.
*
* @param name	アクターの名前.
* @param hp		耐久力.
* @param pos	位置.
* @param rot	回転.
* @param scale	拡大率.
*
* 指定された名前、耐久力、位置、回転、拡大率によってアクターを初期化する.
*/
Actor::Actor(const std::string& name, int hp, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
	: name(name), health(hp), position(pos), rotation(rot), scale(scale)
{
}

/**
* アクターの状態を更新する.
*
* @param deltaTime	経過時間.
*
* UpdateDrawData(float deltaTime)より前に実行すること.
*/
void Actor::Update(float deltaTime) {

	position += velocity * deltaTime;

	// 衝突判定の更新.
	const glm::mat4 matT = glm::translate(glm::mat4(1), position);
	const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
	const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
	const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
	const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
	const glm::mat4 matModel = matT * matR_XZY * matS;
	colWorld.type = colLocal.type;

	switch (colLocal.type)
	{
	case Collision::Shape::Type::sphere:
		colWorld.s.center = matModel * glm::vec4(colLocal.s.center, 1);
		colWorld.s.r = colLocal.s.r;
		break;

	case Collision::Shape::Type::capsule:
		colWorld.c.seg.a = matModel * glm::vec4(colLocal.c.seg.a, 1);
		colWorld.c.seg.b = matModel * glm::vec4(colLocal.c.seg.b, 1);
		colWorld.c.r = colLocal.c.r;
		break;

	case Collision::Shape::Type::obb:
		colWorld.obb.center = matModel * glm::vec4(colLocal.obb.center, 1);
		for (int i = 0; i < 3; ++i) {
			colWorld.obb.axis[i] = matR_XZY * glm::vec4(colLocal.obb.axis[i], 1);
		}
		colWorld.obb.e = colLocal.obb.e;
		break;
	}
}

/**
* 描画情報の更新.
*
* @param deltaTime	経過時間.
*
* Update(float deltaTime)の後で実行すること.
*/
void Actor::UpdateDrawData(float deltaTime)
{
}

/**
* アクターの描画.
*
* @param drawType 描画するデータの種類.
*/
void Actor::Draw(Mesh::DrawType drawType)
{
}


/**
* コンストラクタ.
*
* @param m		表示するメッシュ.
* @param name	アクターの名前.
* @param hp		耐久力.
* @param pos	位置.
* @param rot	回転.
* @param scale	拡大率.
*
* 指定されたメッシュ、名前、耐久力、位置、回転、拡大率によってアクターを初期化する.
*/
StaticMeshActor::StaticMeshActor(const Mesh::FilePtr& m, const std::string& name, int hp, const glm::vec3& pos,
								 const glm::vec3& rot, const glm::vec3& scale)
	:mesh(m),Actor(name, hp, pos, rot, scale)
{
}

/**
* 描画.
*
* @param drawType 描画するデータの種類.
*/
void StaticMeshActor::Draw(Mesh::DrawType drawType)
{
	if (mesh) 
	{
		const glm::mat4 matT = glm::translate(glm::mat4(1), position);
		const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
		const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
		const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
		const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matModel = matT * matR_XZY * matS;
		
		if (!mesh->materials.empty()) {
			if (drawType == Mesh::DrawType::color) {
				const Shader::ProgramPtr p = mesh->materials[0].program;
				if (p) {
					p->Use();
					p->SetPointLightIndex(pointLightCount, pointLightIndex);
					p->SetSpotLightIndex(spotLightCount, spotLightIndex);
				}
			}
			Mesh::Draw(mesh, matModel, drawType);
		}
	}
}

/**
* アクターに影響するポイントライトのインデックスを設定する.
*
* @param v	ポイントライトのインデックス配列.
*/
void StaticMeshActor::SetPointLightList(const std::vector<int>& v) 
{
	pointLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		pointLightIndex[i] = v[i];
	}
}

/**
* アクターに影響するスポットライトのインデックスを設定する.
*
* @param v	スポットライトのインデックス配列.
*/
void StaticMeshActor::SetSpotLightList(const std::vector<int>& v) 
{
	spotLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		spotLightIndex[i] = v[i];
	}
}

/**
* 格納可能なアクター数を確保する.
*
* @param reserveCount	アクターの配列の確保数.
*/
void ActorList::Reserve(size_t reserveCount)
{
	actors.reserve(reserveCount);
}

/**
* アクターを追加する.
*
* @param actor	追加するアクター.
*/
void ActorList::Add(const ActorPtr& actor)
{
	actors.push_back(actor);
}

/**
* アクターを削除する.
*
* @param actor	削除するアクター.
*
* @retval true	 削除成功.
* @retval false	 削除失敗.
*/
bool ActorList::Remove(const ActorPtr& actor)
{
	for (auto itr = actors.begin(); itr != actors.end(); ++itr) {
		if (*itr == actor) {
			actors.erase(itr);
			return true;
		}
	}
	return false;
}

/**
* 指定された座標に対応する格子のインデックスを取得する.
*
* @param pos  インデックスの元になる位置.
*
* @return	  posに対応する格子のインデックス.
*/
glm::ivec2 ActorList::CalcMapIndex(const glm::vec3& pos) const 
{
	const int x = std::max(0,
		std::min(sepalationSizeX - 1, static_cast<int>(pos.x / mapGridSizeX)));
	const int y = std::max(0,
		std::min(sepalationSizeY - 1, static_cast<int>(pos.z / mapGridSizeY)));
	return glm::ivec2(x, y);
}

/**
* アクターの状態を更新する.
*
* @param deltaTime	前回の更新からの経過時間.
*
* UpdateDrawData(float deltaTime)より前に実行すること.
*/
void ActorList::Update(float deltaTime)
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->Update(deltaTime);
		}
	}

	// 死亡したアクターを削除する.
	for (auto i = actors.begin(); i != actors.end();) {
		const ActorPtr& e = *i;
		if (!e || e->health <= 0) {
			i = actors.erase(i);
		}
		else {
			++i;
		}
	}

	// 格子状空間にアクターを割り当てる.
	for (int y = 0; y < sepalationSizeY; ++y) {
		for (int x = 0; x < sepalationSizeX; ++x) {
			grid[y][x].clear();
		}
	}
	for (auto i = actors.begin(); i != actors.end(); ++i) {
		const glm::ivec2 mapIndex = CalcMapIndex((*i)->position);
		grid[mapIndex.y][mapIndex.x].push_back(*i);
	}
}

/**
* アクターの描画データを更新する.
*
* @param deltaTime	前回の更新からの経過時間.
*
* Update(float deltaTime)の後で実行すること.
*/
void ActorList::UpdateDrawData(float deltaTime)
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->UpdateDrawData(deltaTime);
		}
	}
}

/**
* アクターを描画する.
*
* @param drawType 描画するデータの種類.
*/
void ActorList::Draw(Mesh::DrawType drawType)
{
	for (const ActorPtr& e : actors) {
		if (e && e->health > 0) {
			e->Draw(drawType);
		}
	}
}

/**
* 指定された座標の近傍にあるアクターのリストを取得する.
*
* @param pos		  検索の基点となる座標.
* @param maxDistance  近傍とみなす最大距離(m).
*
* @return	Actor::positionがposから半径maxDistance以内にあるアクターの配列.
*/
std::vector<ActorPtr> ActorList::FindNearbyActors(
	const glm::vec3& pos, float maxDistance) const 
{
	std::vector<std::pair<float, ActorPtr>> buffer;
	buffer.reserve(1000);

	const glm::ivec2 mapIndex = CalcMapIndex(pos);
	const glm::ivec2 min = glm::max(mapIndex - 1, 0);
	const glm::ivec2 max = glm::min(mapIndex + 1,
		glm::ivec2(sepalationSizeX - 1, sepalationSizeY - 1));

	for (int y = min.y; y <= max.y; ++y) {
		for (int x = min.x; x <= max.x; ++x) {
			const std::vector<ActorPtr>& list = grid[y][x];
			for (auto actor : list) {
				const float distance = glm::distance(glm::vec3(actor->position), pos);
				buffer.push_back(std::make_pair(distance, actor));
			}
		}
	}

	std::vector<ActorPtr> result;
	result.reserve(100);

	for (const auto& e : buffer) {
		if (e.first <= maxDistance) {
			result.push_back(e.second);
		}
	}

	return result;
}

/**
* 衝突判定を行う.
*
* @param a			衝突判定のアクターその1.
* @param b			衝突判定のアクターその2.
* @param handler	衝突した場合に実行される関数.
*/
void DetectCollision(const ActorPtr& a, const ActorPtr& b, CollisionHandlerType handler) 
{
	if (a->health <= 0 || b->health <= 0) {
		return;
	}
	glm::vec3 pa, pb;
	if (Collision::TestShapeShape(a->colWorld, b->colWorld, &pa, &pb)) {
		if (handler) {
			handler(a, b, pb);
		}
		else {
			a->OnHit(b, pb);
			b->OnHit(a, pa);
		}
	}
}

/**
* 衝突判定を行う.
*
* @param a			衝突判定のアクター.
* @param b			衝突判定のアクターリスト.
* @param handler	衝突した場合に実行される関数.
*/
void DetectCollision(const ActorPtr& a, ActorList& b, CollisionHandlerType handler)
{
	if (a->health <= 0) {
		return;
	}
	for (const ActorPtr& actorB : b) {
		if (actorB->health <= 0) {
			continue;
		}
		glm::vec3 pa, pb;
		if (Collision::TestShapeShape(a->colWorld, actorB->colWorld, &pa, &pb)) {
			if (handler) {
				handler(a, actorB, pb);
			}
			else {
				a->OnHit(actorB, pb);
				actorB->OnHit(a, pa);
			}
			
			if (a->health <= 0) {
				break;
			}
		}
	}
}

/**
* 衝突判定を行う.
*
* @param a			衝突判定のアクターリストその1.
* @param b			衝突判定のアクターリストその2.
* @param handler	衝突した場合に実行される関数.
*/
void DetectCollision(ActorList& a, ActorList& b, CollisionHandlerType handler)
{
	for (const ActorPtr& actorA : a) {
		if (actorA->health <= 0) {
			continue;
		}
		for (const ActorPtr& actorB : b) {
			if (actorB->health <= 0) {
				continue;
			}
			glm::vec3 pa, pb;
			if (Collision::TestShapeShape(actorA->colWorld, actorB->colWorld, &pa, &pb)) {
				if (handler) {
					handler(actorA, actorB, pb);
				}
				else {
					actorA->OnHit(actorB, pb);
					actorB->OnHit(actorA, pa);
				}
								
				if (actorA->health <= 0) {
					break;
				}
			}
		}
	}
}
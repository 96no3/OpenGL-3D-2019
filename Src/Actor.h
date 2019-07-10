/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <GL/glew.h>
#include "Mesh.h"
#include "Collision.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

/**
* シーンに配置するオブジェクト.
*/
class Actor {

public:
	Actor(const std::string& name, int hp, const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0), const glm::vec3& scale = glm::vec3(1));
	virtual ~Actor() = default;

	virtual void Update(float deltaTime);
	virtual void UpdateDrawData(float deltaTime);
	virtual void Draw();


	std::string name;                  ///< アクターの名前.
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);
	glm::vec3 velocity = glm::vec3(0); ///< 速度.
	int health = 0;                    ///< 体力.
	Collision::Sphere colLocal;        ///< 衝突判定形状.
	Collision::Sphere colWorld;        ///< colLocalをワールド空間に移した値.
};
using ActorPtr = std::shared_ptr<Actor>;

/**
* メッシュ表示機能付きのアクター.
*/
class StaticMeshActor :public Actor
{
public:
	StaticMeshActor(const Mesh::FilePtr& m, const std::string& name, int hp, const glm::vec3& pos, 
					const glm::vec3& rot = glm::vec3(0), const glm::vec3& scale = glm::vec3(1));
	virtual ~StaticMeshActor() = default;

	virtual void Draw() override;

	const Mesh::FilePtr& GetMesh() const { return mesh; }

private:
	Mesh::FilePtr mesh;
};
using StaticMeshActorPtr = std::shared_ptr<StaticMeshActor>;

/**
* アクターをまとめて操作するクラス.
*/
class ActorList
{
public:
	// イテレーターを定義する.
	using iterator = std::vector<ActorPtr>::iterator;
	using const_iterator = std::vector<ActorPtr>::const_iterator;

	ActorList() = default;
	~ActorList() = default;

	void Reserve(size_t);
	void Add(const ActorPtr&);
	bool Remove(const ActorPtr&);
	void Update(float deltaTime);
	void UpdateDrawData(float deltaTime);
	void Draw();

	// イテレーターを取得する関数.
	iterator begin() { return actors.begin(); }
	iterator end() { return actors.end(); }
	const_iterator begin() const { return actors.begin(); }
	const_iterator end() const { return actors.end(); }

private:
	std::vector<ActorPtr> actors;
};

using CollisionHandlerType = std::function<void(const ActorPtr&, const ActorPtr&, const glm::vec3&)>;
void DetectCollision(const ActorPtr& a, const ActorPtr& b, CollisionHandlerType handler);
void DetectCollision(const ActorPtr& a, ActorList& b, CollisionHandlerType handler);
void DetectCollision(ActorList& a, ActorList& b, CollisionHandlerType handler);

#endif	// ACTOR_H_INCLUDED
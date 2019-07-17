/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Collision {

	/**
	* 球.
	*/
	struct Sphere {
		glm::vec3 center = glm::vec3(0);  ///< 中心座標.
		float r = 0;  ///< 半径.
	};

	/**
	* 線分.
	*/
	struct Segment {
		glm::vec3 a = glm::vec3(0);  ///< 線分の始点.
		glm::vec3 b = glm::vec3(0);  ///< 線分の終点.
	};

	/**
	* カプセル.
	*/
	struct Capsule {
		Segment seg;  ///< 円柱部の中心の線分.
		float r = 0;  ///< カプセルの半径.
	};

	/**
	* 汎用衝突形状.
	*/
	struct Shape 
	{
		enum class Type {
			none,    ///< 形状無し.
			sphere,  ///< 球.
			capsule, ///< カプセル.
		};
		Type type = Type::none; ///< 実際の形状.

		Sphere s;  ///< 球の形状データ.
		Capsule c; ///< カプセルの形状データ.
	};

	// 形状作成関数.
	Shape CreateSphere(const glm::vec3&, float);
	Shape CreateCapsule(const glm::vec3&, const glm::vec3&, float);

	bool TestSphereSphere(const Sphere&, const Sphere&);
	bool TestSphereCapsule(const Sphere&, const Capsule&, glm::vec3*);
	bool TestShapeShape(const Shape&, const Shape&, glm::vec3*, glm::vec3*);

} // namespace Collision

#endif // COLLISION_H_INCLUDED
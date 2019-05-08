/**
* @file Scene.cpp
*/
#include "Scene.h"
#include <iostream>

/**
* コンストラクタ
*
* @param name	シーン名.
*/
Scene::Scene(const char* name) :name(name)
{
	std::cout << "Scene コンストラクタ" << name << std::endl;
}

/**
* デストラクタ
*/
Scene::~Scene()
{
	Finalize();
	std::cout << "Scene デストラクタ" << name << std::endl;
}

/**
* シーンを活性状態にする
*/
void Scene::Play()
{
	isActive = true;
	std::cout << "Scene Play:" << name << std::endl;
}

/**
* シーンを停止状態にする
*/
void Scene::Stop()
{
	isActive = false;
	std::cout << "Scene Stop:" << name << std::endl;
}

/**
* シーンを表示する
*/
void Scene::Show()
{
	isVisible = true;
	std::cout << "Scene Show:" << name << std::endl;
}

/**
* シーンを非表示する
*/
void Scene::Hide()
{
	isVisible = false;
	std::cout << "Scene Hide:" << name << std::endl;
}

/**
* シーン名を取得する
*
* @return  シーン名.
*/
const std::string& Scene::Name() const
{
	return name;
}

/**
* シーン名を取得する
*
* @retval true	活動している.
* @retval false 活動していない.
*/
bool Scene::IsVisible() const
{
	return isVisible;
}

/**
* シーン名を取得する
*
* @retval true	活動している.
* @retval false 活動していない.
*/
bool Scene::IsActive() const
{
	return isActive;
}
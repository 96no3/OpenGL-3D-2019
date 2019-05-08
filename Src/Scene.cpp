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
}

/**
* デストラクタ
*/
Scene::~Scene()
{
}

/**
* シーンを活性状態にする
*/
void Scene::Play()
{
}

/**
* シーンを停止状態にする
*/
void Scene::Stop()
{
}

/**
* シーンを表示する
*/
void Scene::Show()
{
}

/**
* シーンを非表示する
*/
void Scene::Hide()
{
}

/**
* シーン名を取得する
*
* @return  シーン名.
*/
const std::string& Scene::Name() const
{
}

/**
* シーン名を取得する
*
* @retval true	活動している.
* @retval false 活動していない.
*/
bool Scene::IsVisible() const
{
}

/**
* シーン名を取得する
*
* @retval true	活動している.
* @retval false 活動していない.
*/
bool Scene::IsActive() const
{
}
/**
* @file Scene.cpp
*/
#include "Scene.h"
#include <iostream>

/**
* �R���X�g���N�^
*
* @param name	�V�[����.
*/
Scene::Scene(const char* name) :name(name)
{
}

/**
* �f�X�g���N�^
*/
Scene::~Scene()
{
}

/**
* �V�[����������Ԃɂ���
*/
void Scene::Play()
{
}

/**
* �V�[�����~��Ԃɂ���
*/
void Scene::Stop()
{
}

/**
* �V�[����\������
*/
void Scene::Show()
{
}

/**
* �V�[�����\������
*/
void Scene::Hide()
{
}

/**
* �V�[�������擾����
*
* @return  �V�[����.
*/
const std::string& Scene::Name() const
{
}

/**
* �V�[�������擾����
*
* @retval true	�������Ă���.
* @retval false �������Ă��Ȃ�.
*/
bool Scene::IsVisible() const
{
}

/**
* �V�[�������擾����
*
* @retval true	�������Ă���.
* @retval false �������Ă��Ȃ�.
*/
bool Scene::IsActive() const
{
}
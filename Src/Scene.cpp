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
	std::cout << "Scene �R���X�g���N�^" << name << std::endl;
}

/**
* �f�X�g���N�^
*/
Scene::~Scene()
{
	Finalize();
	std::cout << "Scene �f�X�g���N�^" << name << std::endl;
}

/**
* �V�[����������Ԃɂ���
*/
void Scene::Play()
{
	isActive = true;
	std::cout << "Scene Play:" << name << std::endl;
}

/**
* �V�[�����~��Ԃɂ���
*/
void Scene::Stop()
{
	isActive = false;
	std::cout << "Scene Stop:" << name << std::endl;
}

/**
* �V�[����\������
*/
void Scene::Show()
{
	isVisible = true;
	std::cout << "Scene Show:" << name << std::endl;
}

/**
* �V�[�����\������
*/
void Scene::Hide()
{
	isVisible = false;
	std::cout << "Scene Hide:" << name << std::endl;
}

/**
* �V�[�������擾����
*
* @return  �V�[����.
*/
const std::string& Scene::Name() const
{
	return name;
}

/**
* �V�[�������擾����
*
* @retval true	�������Ă���.
* @retval false �������Ă��Ȃ�.
*/
bool Scene::IsVisible() const
{
	return isVisible;
}

/**
* �V�[�������擾����
*
* @retval true	�������Ă���.
* @retval false �������Ă��Ȃ�.
*/
bool Scene::IsActive() const
{
	return isActive;
}

/**
* �V�[���Ǘ��N���X
*
* @return �V�[���X�^�b�N.
*/
SceneStack& SceneStack::Instance() {
	static SceneStack instance;
	return instance;
}

/**
* �R���X�g���N�^
*/
SceneStack::SceneStack() {
	stack.reserve(16);
}

/**
* �V�[�����v�b�V������.
*
* @param p �V�����V�[��.
*/
void SceneStack::Push(ScenePtr p){
	if (!stack.empty()) {
		Current().Stop();
	}
	stack.push_back(p);
	std::cout << "�u�V�[���@�v�b�V���v" << p->Name() << "\n";
	Current().Initialize();
	Current().Play();
}

/**
* �V�[�����|�b�v����.
*/
void SceneStack::Pop(){
	if (stack.empty()) {
		std::cout << "�u�V�[���@�|�b�v�v[�x��]�V�[���X�^�b�N����ł�." << "\n";
		return;
	}
	Current().Stop();
	Current().Finalize();
	const std::string sceneName = Current().Name();
	stack.pop_back();
	std::cout << "�u�V�[���@�|�b�v�v" << sceneName << "\n";
	if (!stack.empty()) {
		Current().Play();
	}
}

/**
* �V�[����u��������.
*
* @param p �V�����V�[��.
*/
void SceneStack::Replace(ScenePtr p){
	std::string sceneName = "(Empty)";
	if (stack.empty()) {
		std::cout << "�u�V�[���@���v���[�X�v[�x��]�V�[���X�^�b�N����ł�." << "\n";
	}
	else {
		sceneName = Current().Name();
		Current().Stop();
		Current().Finalize();
		stack.pop_back();
	}
	stack.push_back(p);
	std::cout << "�u�V�[���@���v���[�X�v" << sceneName << "->" << p->Name() << "\n";
	Current().Initialize();
	Current().Play();
}

/**
* ���݂̃V�[�����擾����
*
* @return ���݂̃V�[��.
*/
Scene& SceneStack::Current(){
	return *stack.back();
}

/**
* ���݂̃V�[�����擾����
*
* @return ���݂̃V�[��.
*/
const Scene& SceneStack::Current() const {
	return *stack.back();
}

/**
* �V�[���̐����擾����
*
* @return �X�^�b�N�ɐς܂�Ă���V�[���̐�.
*/
size_t SceneStack::Size() const{
	return stack.size();
}

/**
* �V�[�����󂩂ǂ����𒲂ׂ�.
*
* @retval true  �X�^�b�N�͋�.
* @retval false �X�^�b�N��1�ȏ�̃V�[�����ς܂�Ă���.
*/
bool SceneStack::Empty() const{
	return stack.empty();
}

/**
* �V�[�����X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ��ԁi�b�j.
*/
void SceneStack::Update(float deltaTime){
	
	if (!Empty()) {
		Current().ProcessInput();
	}

	for (ScenePtr& e : stack) {
		if (e->IsActive()) {
			e->Update(deltaTime);
		}		
	}
}

/**
* �V�[����`�悷��.
*/
void SceneStack::Render(){
	for (ScenePtr& e : stack) {
		if (e->IsVisible()) {
			e->Render();
		}
	}
}
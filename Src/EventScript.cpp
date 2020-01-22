/**
* @file EventScript.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "EventScript.h"
#include "GLFWEW.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <locale.h>
#include <stdlib.h>

/**
* �X�N���v�g�G���W���̃V���O���g���E�C���X�^���X���擾����.
*
* @return �X�N���v�g�G���W���̃V���O���g���E�C���X�^���X.
*/
EventScriptEngine& EventScriptEngine::Instance() 
{
	static EventScriptEngine instance;
	return instance;
}

/**
* �X�N���v�g�G���W��������������.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool EventScriptEngine::Init()
{
	if (isInitialized) {
		std::cerr << "[�G���[] EventScriptEngine�͊��ɏ���������Ă��܂�.\n";
		return false;
	}

	filename.reserve(256);
	script.reserve(2048);
	if (!textWindow.Init("Res/Images/TextWindow.tga",
		glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0))) {
		std::cerr << "[�G���[]" << __func__ << "�F�X�N���v�g�G���W���̏������Ɏ��s.\n";
		return false;
	}
	isInitialized = true;
	return true;
}

/**
* �C�x���g�E�X�N���v�g�����s����.
*
* @param filename �X�N���v�g�E�t�@�C����.
*
* @retval true    ���s�ɐ���.
* @retval false   ���s�Ɏ��s.
*/
bool EventScriptEngine::RunScript(const char* filename)
{
	if (!isInitialized) {
		return false;
	}

	// �X�N���v�g�t�@�C����ǂݍ��݁A�e�L�X�g�E�B���h�E�\���̂��߂�wchar_t�^�ɕϊ�����.
	std::ifstream ifs(filename);
	if (!ifs) {
		std::cerr << "[�G���[]" << __func__ << "�F�X�N���v�g�t�@�C��" << filename << "��ǂݍ��߂܂���.\n";
		return false;
	}
	std::stringstream ss;
	// �N���X�o�b�t�@�I�u�W�F�N�g�o�R�Ńt�@�C���S�̂�ǂݍ���.
	ss << ifs.rdbuf();
	std::string tmp = ss.str();
	// �ϊ����̌�����w��.
	setlocale(LC_CTYPE, "ja-JP");
	// �e�L�X�g�E�B���h�E�ɓn����`�ɕϊ�.
	const size_t size = mbstowcs(nullptr, tmp.c_str(), 0);
	script.resize(size);
	mbstowcs(&script[0], tmp.c_str(), size);

	// �p�����[�^��ݒ�.
	isFinished = false;
	this->filename = filename;

	// �e�L�X�g�E�B���h�E�ɃX�N���v�g��\��.
	textWindow.Open(script.c_str());

	std::cout << "[INFO]" << __func__ << "�F�X�v���N�g�t�@�C��" << filename << "�����s.\n";
	return true;
}

/**
* �X�N���v�g�G���W���̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V��������̌o�ߎ���(�b).
*/
void EventScriptEngine::Update(float deltaTime) 
{
	if (!isInitialized) {
		return;
	}

	if (textWindow.IsFinished()) {
		const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
		if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
			textWindow.Close();
			isFinished = true;
		}
	}
	textWindow.Update(deltaTime);
}

/**
* �X�N���v�g�G���W����`�悷��.
*/
void EventScriptEngine::Draw()
{
	if (!isInitialized) {
		return;
	}
	textWindow.Draw();
}

/**
* �X�N���v�g�̎��s�������������ǂ������ׂ�.
*
* @retval true    ���s����.
* @retval false   ���s���A�܂��̓X�N���v�g���ǂݍ��܂�Ă��Ȃ�.
*/
bool EventScriptEngine::IsFinished() const 
{
	return isFinished;
}
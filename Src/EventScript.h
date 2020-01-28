/**
* @file EventScript.h
*/
#ifndef EVENTSCRIPT_H_INCLUDED
#define EVENTSCRIPT_H_INCLUDED
#include "TextWindow.h"
#include <string>
#include <vector>

/**
* �C�x���g�X�N���v�g����N���X.
*/
class EventScriptEngine 
{
public:
	static EventScriptEngine& Instance();

	bool Init();
	bool RunScript(const char* filename);
	void Update(float deltaTime);
	void Draw();

	bool IsFinished() const;

private:
	EventScriptEngine() = default;
	~EventScriptEngine() = default;
	EventScriptEngine(const EventScriptEngine&) = delete;
	EventScriptEngine& operator=(const EventScriptEngine&) = delete;

	// ���߂̎��.
	enum class InstructionType {
		nop,      // �������Ȃ��_�~�[����.
		print,    // ���͂�\��.
		assign,   // �ϐ��ɒl��ݒ�.
		beginif,  // if���߂̊J�n.
		endif,    // if���߂̏I�[.
	};

	// �����̌^.
	using Argument = std::wstring;

	// �X�N���v�g���ߌ^.
	struct Instruction {
		InstructionType type = InstructionType::nop;
		Argument arguments[4];
	};

	std::string filename;
	//std::wstring script;
	std::vector<Instruction> script;
	size_t programCounter = 0;  // ���Ɏ��s���閽�߂̈ʒu.

	bool isInitialized = false; // �G���W��������������Ă����true.
	bool isFinished = false;    // �X�N���v�g�̎��s���I��������true.

	TextWindow textWindow;
};

#endif // EVENTSCRIPT_H_INCLUDED
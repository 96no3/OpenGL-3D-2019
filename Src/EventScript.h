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

	std::string filename;
	std::wstring script;

	bool isInitialized = false; // �G���W��������������Ă����true.
	bool isFinished = false;    // �X�N���v�g�̎��s���I��������true.

	TextWindow textWindow;
};

#endif // EVENTSCRIPT_H_INCLUDED
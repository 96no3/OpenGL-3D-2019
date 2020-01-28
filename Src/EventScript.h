/**
* @file EventScript.h
*/
#ifndef EVENTSCRIPT_H_INCLUDED
#define EVENTSCRIPT_H_INCLUDED
#include "TextWindow.h"
#include <string>
#include <vector>

/**
* イベントスクリプト制御クラス.
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

	// 命令の種類.
	enum class InstructionType {
		nop,      // 何もしないダミー命令.
		print,    // 文章を表示.
		assign,   // 変数に値を設定.
		beginif,  // if命令の開始.
		endif,    // if命令の終端.
	};

	// 引数の型.
	using Argument = std::wstring;

	// スクリプト命令型.
	struct Instruction {
		InstructionType type = InstructionType::nop;
		Argument arguments[4];
	};

	std::string filename;
	//std::wstring script;
	std::vector<Instruction> script;
	size_t programCounter = 0;  // 次に実行する命令の位置.

	bool isInitialized = false; // エンジンが初期化されていればtrue.
	bool isFinished = false;    // スクリプトの実行が終了したらtrue.

	TextWindow textWindow;
};

#endif // EVENTSCRIPT_H_INCLUDED
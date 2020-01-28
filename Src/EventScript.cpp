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
* スクリプトエンジンのシングルトン・インスタンスを取得する.
*
* @return スクリプトエンジンのシングルトン・インスタンス.
*/
EventScriptEngine& EventScriptEngine::Instance() 
{
	static EventScriptEngine instance;
	return instance;
}

/**
* スクリプトエンジンを初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool EventScriptEngine::Init()
{
	if (isInitialized) {
		std::cerr << "[エラー] EventScriptEngineは既に初期化されています.\n";
		return false;
	}

	filename.reserve(256);
	script.reserve(2048);
	if (!textWindow.Init("Res/Images/TextWindow.tga",
		glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0))) {
		std::cerr << "[エラー]" << __func__ << "：スクリプトエンジンの初期化に失敗.\n";
		return false;
	}

	// スプリクト変数の初期化.
	variables.resize(100, 0.0);

	isInitialized = true;
	return true;
}

/**
* イベント・スクリプトを実行する.
*
* @param filename スクリプト・ファイル名.
*
* @retval true    実行に成功.
* @retval false   実行に失敗.
*/
bool EventScriptEngine::RunScript(const char* filename)
{
	if (!isInitialized) {
		return false;
	}

	// スクリプトファイルを読み込み、テキストウィンドウ表示のためにwchar_t型に変換する.
	std::ifstream ifs(filename);
	if (!ifs) {
		std::cerr << "[エラー]" << __func__ << "：スクリプトファイル" << filename << "を読み込めません.\n";
		return false;
	}
	//std::stringstream ss;
	//// クラスバッファオブジェクト経由でファイル全体を読み込む.
	//ss << ifs.rdbuf();
	//std::string tmp = ss.str();

	// 変換元の言語を指定.
	setlocale(LC_CTYPE, "ja-JP");

	//// テキストウィンドウに渡せる形に変換.
	//const size_t size = mbstowcs(nullptr, tmp.c_str(), 0);
	//script.resize(size);
	//mbstowcs(&script[0], tmp.c_str(), size);

	size_t lineCount = 0;  // 読み込んだ行数.
	std::string line;
	char buf[1000];
	while (std::getline(ifs, line)) {
		// 先頭の空白を除去する.
		line.erase(0, line.find_first_not_of(" \t\n"));
		++lineCount;

		Instruction inst;
		// print命令を読み込む.
		int n = sscanf(line.c_str(), "print %999[^\n]", buf);
		if (n >= 1) {
			// テキストウィンドウに渡せる形に変換.
			const size_t size = mbstowcs(nullptr, buf, 0);
			std::wstring text(size, L'\0');
			mbstowcs(&text[0], buf, size);
			inst.type = InstructionType::print;
			inst.arguments[0] = text;
			script.push_back(inst);
			continue;
		}
	}

	programCounter = 0;

	// パラメータを設定.
	isFinished = false;
	this->filename = filename;

	//// テキストウィンドウにスクリプトを表示.
	//textWindow.Open(script.c_str());

	std::cout << "[INFO]" << __func__ << "：スプリクトファイル" << filename << "を実行.\n";
	return true;
}

/**
* スクリプトエンジンの状態を更新する.
*
* @param deltaTime 前回の更新処理からの経過時間(秒).
*/
void EventScriptEngine::Update(float deltaTime) 
{
	if (!isInitialized) {
		return;
	}

	// スクリプト未設定、または実行終了なら何もしない.
	if (script.empty() || isFinished) {
		return;
	}

	// 命令を実行する.
	// 実行を中断する場合はyield変換にtrueを設定.
	for (bool yield = false; !yield;) {
		// 実行位置(programCounter)がスクリプトの命令数以上なら実行完了.
		if (programCounter >= script.size()) {
			isFinished = true;
			break;
		}

		const auto& inst = script[programCounter];
		switch (inst.type)
		{
		case InstructionType::print:
			if (!textWindow.IsOpen()) {
				// ウィンドウが閉じているので文章を設定.
				const auto p = std::get_if<Text>(&inst.arguments[0]);
				if (!p) {
					std::cerr << "[エラー]" << __func__ << "print命令の引数はText型でなくてはなりません.\n";
					++programCounter;
					break;
				}
				//textWindow.Open(inst.arguments[0].c_str());
				textWindow.Open(p->c_str());
			}
			else {
				// ウィンドウが開いていたら表示終了を待つ.表示が終了したら、キー入力を待つ.
				// キー入力があったら、ウィンドウを閉じて、次の命令の処理へ進む.
				if (textWindow.IsFinished()) {
					const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
					if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
						textWindow.Close();
						++programCounter;
						continue;
					}
				}
			}
			yield = true;
			break;

		default:
			++programCounter;
			break;
		}
	}

	/*if (textWindow.IsFinished()) {
		const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
		if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
			textWindow.Close();
			isFinished = true;
		}
	}*/
	textWindow.Update(deltaTime);
}

/**
* スクリプトエンジンを描画する.
*/
void EventScriptEngine::Draw()
{
	if (!isInitialized) {
		return;
	}
	textWindow.Draw();
}

/**
* スクリプトの実行が完了したかどうか調べる.
*
* @retval true    実行完了.
* @retval false   実行中、またはスクリプトが読み込まれていない.
*/
bool EventScriptEngine::IsFinished() const 
{
	return isFinished;
}
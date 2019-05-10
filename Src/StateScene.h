/**
* @file StateScene.h
*/
#ifndef STATESCENE_H_INCLUDED
#define STATESCENE_H_INCLUDED
#include "Scene.h"

/**
* ステータス画面.
*/
class StateScene : public Scene {
public:
	StateScene() :Scene("StateScene") {}
	virtual ~StateScene() = default;

	virtual bool Initialize() override { return true; }
	virtual void ProcessInput() override;
	virtual void Update(float) override {}
	virtual void Render() override {}
	virtual void Finalize() override {}
};

#endif // STATESCENE_H_INCLUDED
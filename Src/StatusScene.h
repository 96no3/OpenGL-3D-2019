/**
* @file StatusScene.h
*/
#ifndef STATUSSCENE_H_INCLUDED
#define STATUSSCENE_H_INCLUDED
#include "Scene.h"
#include "Font.h"

/**
* ステータス画面.
*/
class StatusScene : public Scene {
public:
	StatusScene();
	virtual ~StatusScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override {}
private:
	FontRenderer fontRenderer;
};

#endif // STATUSSCENE_H_INCLUDED
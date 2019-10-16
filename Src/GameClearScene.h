/**
* @file GameClearScene.h
*/
#ifndef GAMECLEARSCENE_H_INCLUDED
#define GAMECLEARSCENE_H_INCLUDED
#include "Scene.h"
#include "Sprite.h"
#include "Font.h"
#include "Audio/Audio.h"

/**
* ÉQÅ[ÉÄÉNÉäÉAâÊñ .
*/
class GameClearScene : public Scene {
public:
	GameClearScene() :Scene("GameClearScene") {}
	virtual ~GameClearScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override {}

private:
	std::vector<Sprite> sprites;
	SpriteRenderer spriteRenderer;
	FontRenderer fontRenderer;

	float timer = 0;
	Audio::SoundPtr bgm;
};

#endif // GAMECLEARSCENE_H_INCLUDED
#pragma once

class SpriteScreenMask;

class SceneManager
{
public:
	~SceneManager(void) {};
	static SceneManager& sharedInstance() {return instance;};

	bool loadMainMenu();
	bool loadLevel(int levelNumber);

	void initFirstScene();

	bool isCurrentSceneLevel(){return this->currentSceneLevel;};
	bool isNextSceneLevel(){return this->nextSceneLevel;};

private:
	SceneManager() : currentSceneLevel(false), nextSceneLevel(false), blackVeil(NULL){};
	static SceneManager instance;

	bool currentSceneLevel;
	bool nextSceneLevel;

	SpriteScreenMask* blackVeil;
};


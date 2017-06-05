#include "OrbsPluginPCH.h"
#include "SceneManager.h"

SceneManager SceneManager::instance;

bool SceneManager::loadMainMenu()
{
	string relativePath = string("Scenes\\");
	string mainMenuName = relativePath + string("IntroMenu") + string(".vscene");

	VSceneLoader sceneLoader;
	this->nextSceneLevel = false;
	if (!sceneLoader.LoadScene(mainMenuName.c_str()))
	{
		hkvLog::Error(sceneLoader.GetLastError());
		return false;
	}
	else
	{
		while (!sceneLoader.IsFinished())
		{
			hkvLog::Error("Tick");
			sceneLoader.Tick();
		}		
	}
	this->currentSceneLevel = false;

	return true;
}

bool SceneManager::loadLevel(int levelNumber)
{
	LevelManager::sharedInstance().setCurrentLevelByNumber(levelNumber);

	string relativePath = string("Scenes\\");
	string levelName = relativePath + string("level_") + stringFromInt(levelNumber) + string(".vscene");

	VSceneLoader sceneLoader;
	this->nextSceneLevel = true;
	if (!sceneLoader.LoadScene(levelName.c_str()))
	{
		hkvLog::Error(sceneLoader.GetLastError());
		return false;
	}
	else
	{
		while (!sceneLoader.IsFinished())
		{
			sceneLoader.Tick();
		}
	}
	this->currentSceneLevel = true;
	return true;
}

void SceneManager::initFirstScene()
{
	this->currentSceneLevel = true;
	this->nextSceneLevel = true;
}
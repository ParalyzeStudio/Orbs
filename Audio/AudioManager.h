#pragma once

class MusicTrack;
class SoundCallbackHandler;

#define MUSIC_TIME_INTERVAL 0.0f

class AudioManager : public ResourcePreloader, public IVisCallbackHandler_cl
{
public:
	~AudioManager(void) {};

	void OneTimeInit();
	void OneTimeDeInit();

	static AudioManager& sharedInstance() {return instance;};

	void preloadStringsFiles();
	void preloadMusic();
	void preloadMenuSounds();
	void preloadGameSounds();

	//musics
	void queueRandomMusic(float delay);
	void playQueuedMusic();

	//sounds
	void playMidPanelOpeningSound();
	void playMenuStandardButtonSound();
	void playOptionsPanelButtonSound();
	void playPageSwipeSound();
	void playLevelsMenuIntroSound();
	void playLevelsListOpeningSound();
	void playWindowOpeningSound();

	void toggleMusic();
	void toggleSound();

	vector<string> getMusicNamesList();

	void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	void setCurrentPlayedMusic(VFmodSoundObject* currentPlayedMusic){this->currentPlayedMusic = currentPlayedMusic;};

private:
	AudioManager() : playedMusicIndex(0), 
		currentPlayedMusic(NULL),
		midPanelOpeningSoundRes(NULL),
		menuStandardButtonSoundRes(NULL),
		optionsPanelButtonSoundRes(NULL),
		achievementsPageSwipeSoundRes(NULL),
		levelsMenuIntroSoundRes(NULL),
		levelsListOpeningSoundRes(NULL),
		windowOpeningSoundRes(NULL)
	{};

	static AudioManager instance;

	vector<MusicTrack*> menuMusics;
	vector<MusicTrack*> gameMusics;
	int playedMusicIndex;

	//music (1 at a time)
	VFmodSoundObject* currentPlayedMusic;

	//sounds
	//menu sounds
	VFmodSoundResource* midPanelOpeningSoundRes;
	VFmodSoundResource* menuStandardButtonSoundRes;
	VFmodSoundResource* optionsPanelButtonSoundRes;
	VFmodSoundResource* achievementsPageSwipeSoundRes;
	VFmodSoundResource* levelsMenuIntroSoundRes;
	VFmodSoundResource* levelsListOpeningSoundRes;
	VFmodSoundResource* windowOpeningSoundRes;

	//game sounds
};

class MusicTrack : public VRefCounter
{
public:
	void init(string name, string filename);

	string getName(){return this->name;};
	string getSoundFilename(){return this->filename;};

private:
	string name;
	string filename;
};
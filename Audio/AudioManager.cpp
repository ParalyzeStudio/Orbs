#include "OrbsPluginPCH.h"
#include "AudioManager.h"

AudioManager AudioManager::instance;

void AudioManager::OneTimeInit()
{
	VFmodManager::GlobalManager().OnSoundCallback += this;
}

void AudioManager::OneTimeDeInit()
{
	hkvLog::Error("AudioManager OneTimeDeInit");
	for (int menuMusicIndex = 0; menuMusicIndex != menuMusics.size(); menuMusicIndex++)
	{
		menuMusics[menuMusicIndex]->Release();
	}

	for (int gameMusicIndex = 0; gameMusicIndex != gameMusics.size(); gameMusicIndex++)
	{
		gameMusics[gameMusicIndex]->Release();
	}

	currentPlayedMusic = NULL;

	//sounds
	//menu sounds
	midPanelOpeningSoundRes = NULL;
	menuStandardButtonSoundRes = NULL;
	optionsPanelButtonSoundRes = NULL;
	achievementsPageSwipeSoundRes = NULL;
	levelsMenuIntroSoundRes = NULL;
	levelsListOpeningSoundRes = NULL;
	windowOpeningSoundRes = NULL;

	hkvLog::Error("-= onSoundCallback");
	VFmodManager::GlobalManager().OnSoundCallback -= this;
}

void AudioManager::preloadStringsFiles()
{
	if (!stringsFilesLoaded)
	{
		preloadMusic();
		ResourcePreloader::preloadStringsFiles();
	}
}

void AudioManager::preloadMusic()
{
	//path to Music folder
	const char* musicPath = "Audio\\Music\\";

	//load the playlist
	TiXmlDocument playlist;
	string playlistPath = string(musicPath) + string("playlist.xml");
	bool playlistLoaded = playlist.LoadFile(playlistPath.c_str());

	if (!playlistLoaded)
		return;

	//loop through all music xml lines and populate the array of MusicObject
	for (TiXmlElement *pNode = playlist.RootElement()->FirstChildElement("music"); pNode; pNode=pNode->NextSiblingElement("music"))
	{
		const char *tag = XMLHelper::Exchange_String(pNode,"tag",NULL,false);
		bool isMenuMusic = false;
		XMLHelper::Exchange_Bool(pNode,"menu",isMenuMusic,false);
		const char* name = XMLHelper::Exchange_String(pNode,"name",NULL,false);
		const char* filename = XMLHelper::Exchange_String(pNode,"filename",NULL,false);
		string filenamePath = string(musicPath) + string(filename);

		MusicTrack* music = new MusicTrack();
		music->AddRef();
		music->init(name, filenamePath);
		if (isMenuMusic)
			menuMusics.push_back(music);
		else
			gameMusics.push_back(music);
	}
}

void AudioManager::preloadMenuSounds()
{
	midPanelOpeningSoundRes = VFmodManager::GlobalManager().LoadSoundResource("Audio\\Sounds\\levels_list_opening.mp3", VFMOD_RESOURCEFLAG_2D);
	menuStandardButtonSoundRes = VFmodManager::GlobalManager().LoadSoundResource("", VFMOD_RESOURCEFLAG_2D);
	optionsPanelButtonSoundRes = VFmodManager::GlobalManager().LoadSoundResource("", VFMOD_RESOURCEFLAG_2D);
	achievementsPageSwipeSoundRes = VFmodManager::GlobalManager().LoadSoundResource("Audio\\Sounds\\item_list_slide.aiff", VFMOD_RESOURCEFLAG_2D);
	levelsMenuIntroSoundRes = VFmodManager::GlobalManager().LoadSoundResource("Audio\\Sounds\\levels_menu_intro.wav", VFMOD_RESOURCEFLAG_2D);
	levelsListOpeningSoundRes = VFmodManager::GlobalManager().LoadSoundResource("Audio\\Sounds\\levels_list_opening.mp3", VFMOD_RESOURCEFLAG_2D);
	windowOpeningSoundRes = VFmodManager::GlobalManager().LoadSoundResource("Audio\\Sounds\\window_opening.mp3", VFMOD_RESOURCEFLAG_2D);
}

void AudioManager::preloadGameSounds()
{

}

void AudioManager::queueRandomMusic(float delay)
{
	vector<MusicTrack*> musics;
	if (SceneManager::sharedInstance().isCurrentSceneLevel())
	{
		musics = gameMusics;
	}
	else
	{
		musics = menuMusics;
	}

	if (musics.size() == 0)
		return;
	
	int randomIndex = -1;
	int numMusics = (int) musics.size();
	if (numMusics > 1)
	{
		if (numMusics == 2) //if only 2 musics, only one choice
		{
			randomIndex = (playedMusicIndex == 0) ? 1 : 0;
		}
		else
		{
			do
			{
				randomIndex = rand() % numMusics;
			}
			while (randomIndex == playedMusicIndex);
		}
	}
	else
		randomIndex = 0;

	string soundFilename = musics[randomIndex]->getSoundFilename();
	VFmodSoundObject* randomMusic = VFmodManager::GlobalManager().CreateSoundInstance(soundFilename.c_str(), VFMOD_RESOURCEFLAG_2D | VFMOD_RESOURCEFLAG_STREAM, VFMOD_FLAG_PAUSED);
	CallFuncObject* playMusicAction = new CallFuncObject(this, (CallFunc)(&AudioManager::playQueuedMusic), delay);
	CallFuncHandler::sharedInstance().addCallFuncInstance(playMusicAction);
	playMusicAction->executeFunc();
	this->currentPlayedMusic = randomMusic;
	this->playedMusicIndex = randomIndex;
}

void AudioManager::playQueuedMusic()
{
	this->currentPlayedMusic->Play();
	//this->currentPlayedMusic->SetVolume(0); //tmp volume 0
}

void AudioManager::playMidPanelOpeningSound()
{
	midPanelOpeningSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playMenuStandardButtonSound()
{
	menuStandardButtonSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playOptionsPanelButtonSound()
{
	optionsPanelButtonSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playPageSwipeSound()
{
	achievementsPageSwipeSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playLevelsMenuIntroSound()
{
	levelsMenuIntroSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playLevelsListOpeningSound()
{
	levelsListOpeningSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::playWindowOpeningSound()
{
	windowOpeningSoundRes->CreateInstance(hkvVec3(0,0,0), VFMOD_FLAG_NONE);
}

void AudioManager::toggleMusic()
{
	if (currentPlayedMusic)
		currentPlayedMusic->SetPaused(currentPlayedMusic->IsPlaying());
}

void AudioManager::toggleSound()
{

}

vector<string> AudioManager::getMusicNamesList()
{
	vector<string> list;
	for (int menuMusicIndex = 0; menuMusicIndex != menuMusics.size(); menuMusicIndex++)
	{
		list.push_back(menuMusics[menuMusicIndex]->getName());
	}
	
	for (int gameMusicIndex = 0; gameMusicIndex != gameMusics.size(); gameMusicIndex++)
	{
		list.push_back(gameMusics[gameMusicIndex]->getName());
	}

	return list;
}

void AudioManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &VFmodManager::GlobalManager().OnSoundCallback)
	{
		hkvLog::Error("onSoundCallback");
		VFmodSoundDataObject *pSoundData = (VFmodSoundDataObject *)pData;

		// called, when sound object has ended
		if (pSoundData->m_eType == FMOD_CHANNEL_CALLBACKTYPE_END)
		{
			// get sound object
			VFmodSoundObject *pSoundObject = pSoundData->m_pSoundObject;
			if (pSoundObject == this->currentPlayedMusic) //filter only the current playback music
			{
				this->currentPlayedMusic = NULL;
				queueRandomMusic(MUSIC_TIME_INTERVAL);
			}
		}
	}
}

////////////////////////MusicTrack////////////////////////
void MusicTrack::init(string name, string filename)
{
	this->name = name;
	this->filename = filename;
}
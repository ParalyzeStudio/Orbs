/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include "OrbsApplicationPCH.h"
#include <Vision/Runtime/Framework/VisionApp/VAppImpl.hpp>

#include <Vision/Runtime/Framework/VisionApp/Modules/VHelp.hpp>

#if defined(WIN32) 
#include <Vision/Runtime/EnginePlugins/RemoteInputEnginePlugin/IVRemoteInput.hpp>
#endif

#include <Vision/Runtime/Framework/VisionApp/Modules/VDefaultMenu.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VLoadingScreen.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VRestoreScreen.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VDebugOptions.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VDebugShadingModes.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VDebugProfiling.hpp>
#include <Vision/Runtime/Framework/VisionApp/Modules/VLogoOverlay.hpp>


//#include <Vision/Runtime/Framework/VisionApp/Modules/VFileServe.hpp>
//REGISTER_STARTUP_MODULE(VFileServeStartupModule, 0);


// Use the following line to initialize a plugin that is statically linked.
// Note that only Windows platform links plugins dynamically (on Windows you can comment out this line).
VIMPORT IVisPlugin_cl* GetEnginePlugin_OrbsPlugin();

class OrbsApplicationClass : public VAppImpl
{
public:
  OrbsApplicationClass()
#ifdef WIN_32
	  : m_pRemoteInput(NULL) 
#endif
	  {}
  virtual ~OrbsApplicationClass() {}

  virtual void SetupAppConfig(VisAppConfig_cl& config) HKV_OVERRIDE;
  virtual void PreloadPlugins() HKV_OVERRIDE;

  virtual void Init() HKV_OVERRIDE;
  virtual void AfterSceneLoaded(bool bLoadingSuccessful) HKV_OVERRIDE;
  virtual void AfterEngineInit();
  virtual bool Run() HKV_OVERRIDE;
  virtual void DeInit() HKV_OVERRIDE;

private:
#if defined(WIN32) 
	IVRemoteInput* m_pRemoteInput;
#endif
};

VAPP_IMPLEMENT_SAMPLE(OrbsApplicationClass);

void OrbsApplicationClass::SetupAppConfig(VisAppConfig_cl& config)
{
  // Set custom file system root name ("havok_sdk" by default)
  config.m_sFileSystemRootName = "template_root";

#if defined(WIN32)
  // Set the initial starting position of our game window and other properties
  // if not in fullscreen. This is only relevant on windows
  config.m_videoConfig.m_iXRes = 1024; // Set the Window size X if not in fullscreen.
  config.m_videoConfig.m_iYRes = 768;  // Set the Window size Y if not in fullscreen.
  config.m_videoConfig.m_iXPos = 0;   // Set the Window position X if not in fullscreen.
  config.m_videoConfig.m_iYPos = 0;   // Set the Window position Y if not in fullscreen.
#endif

  // Name to be displayed in the windows title bar.
  config.m_videoConfig.m_szWindowTitle = "StandAlone Project Template";

  config.m_videoConfig.m_bWaitVRetrace = true;

  // Fullscreen mode with current desktop resolution
  
#if defined(WIN32)
  /*
  DEVMODEA deviceMode;
  deviceMode = Vision::Video.GetAdapterMode(config.m_videoConfig.m_iAdapter);
  config.m_videoConfig.m_iXRes = deviceMode.dmPelsWidth;
  config.m_videoConfig.m_iYRes = deviceMode.dmPelsHeight;
  config.m_videoConfig.m_bFullScreen = true;
  */
#endif
  
}

void OrbsApplicationClass::PreloadPlugins()
{
  // Use the following line to load a plugin. Remember that, except on Windows platform, in addition
  // you still need to statically link your plugin library (e.g. on mobile platforms) through project
  // Properties, Linker, Additional Dependencies.
#if defined(WIN32) 
	VISION_PLUGIN_ENSURE_LOADED(vRemoteInput);
#endif
	VISION_PLUGIN_ENSURE_LOADED(OrbsPlugin);
}

//---------------------------------------------------------------------------------------------------------
// Init function. Here we trigger loading our scene
//---------------------------------------------------------------------------------------------------------
void OrbsApplicationClass::Init()
{
  // Set filename and paths to our stand alone version.
  // Note: "/Data/Vision/Base" is always added by the sample framework
  VisAppLoadSettings settings("Scenes\\level_4.vscene");
  settings.m_customSearchPaths.Append(":template_root/Assets");
  LoadScene(settings);
}

//---------------------------------------------------------------------------------------------------------
// Gets called after the scene has been loaded
//---------------------------------------------------------------------------------------------------------
void OrbsApplicationClass::AfterSceneLoaded(bool bLoadingSuccessful)
{
  // Define some help text
  //VArray<const char*> help;
  //help.Append("How to use this demo...");
  //help.Append("");
  //RegisterAppModule(new VHelp(help));

  // Create a mouse controlled camera (set above the ground so that we can see the ground)
  //Vision::Game.CreateEntity("VisMouseCamera_cl", hkvVec3(0.0f, 0.0f, 170.0f));

  // Add other initial game code here
  // [...]

#if defined(WIN32)
	m_pRemoteInput = (IVRemoteInput*)Vision::Plugins.GetRegisteredPlugin("vRemoteInput");
	m_pRemoteInput->StartServer("RemoteGui");
#endif
}



void OrbsApplicationClass::AfterEngineInit()
{
  // Default help menu (press F1)
	RegisterAppModule(new VDefaultMenu());
	RegisterAppModule(new VDebugOptions());
	RegisterAppModule(new VDebugShadingModes());
	RegisterAppModule(new VDebugProfiling());

  //RegisterAppModule(new VLoadingScreen());

  VLoadingScreen* loadingScreen = new VLoadingScreen();

  const VLoadingScreenBase::Settings &settings = loadingScreen->GetSettings();
  VString m_sImagePath = settings.m_sImagePath;
  float fadeOutTime = settings.m_fFadeOutTime;

  VLoadingScreenBase::Settings settings2;
  settings2.m_fFadeOutTime = 2.0f;

  VColorRef m_backgroundColor = V_RGBA_BLACK;                  ///< Color used to clear the background.
  VColorRef m_progressBarColor = V_RGBA_WHITE;                 ///< The color of the progress bar.
  VColorRef m_progressBarBackgroundColor = V_RGBA_GREY;    
  settings2.m_backgroundColor = m_backgroundColor;
  settings2.m_progressBarColor = m_progressBarColor;
  settings2.m_progressBarBackgroundColor = m_progressBarBackgroundColor;

  int m_uiFlags = (VLoadingScreenBase::LSF_SHOW_PROGRESSBAR | VLoadingScreenBase::LSF_FADEOUT_SMOOTH);
  settings2.m_uiFlags = m_uiFlags;

  loadingScreen->SetSettings(settings2);

  RegisterAppModule(loadingScreen);

  //RegisterAppModule(new VRestoreScreen());

  //RegisterAppModule(new VLogoOverlay());
}

//---------------------------------------------------------------------------------------------------------
// Main Loop of the application until we quit
//---------------------------------------------------------------------------------------------------------
bool OrbsApplicationClass::Run()
{
#if defined(WIN32)
	m_pRemoteInput->DebugDrawTouchPoints(VColorRef(255,0,0));
#endif
  return true;
}

void OrbsApplicationClass::DeInit()
{
  // De-Initialization
  // [...]

#if defined(WIN32) 
	m_pRemoteInput = NULL;
#endif
}

/*
 * Havok SDK - Base file, BUILD(#20131218)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2013
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */

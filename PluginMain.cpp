/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2013 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */


//  Basic Plugin Framework to house your own components
//

#include "OrbsPluginPCH.h"
#include "GameManager.h"

#include <Common/Base/KeyCode.h>

// use plugins if supported
VIMPORT IVisPlugin_cl* GetEnginePlugin_vFmodEnginePlugin();
//#ifdef WIN_32
//#if defined( HAVOK_PHYSICS_2012_KEYCODE )
//VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavok();
//#endif
//#if defined( HAVOK_AI_KEYCODE )
//VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokAi();
//#endif
//#if defined( HAVOK_BEHAVIOR_KEYCODE )
//VIMPORT IVisPlugin_cl* GetEnginePlugin_vHavokBehavior();
//#endif
//#endif

//============================================================================================================
//  Set up the Plugin Class
//============================================================================================================
class OrbsPluginClass : public IVisPlugin_cl
{
public:

  void OnInitEnginePlugin();    
  void OnDeInitEnginePlugin();  

  const char *GetPluginName()
  {
    return "OrbsPlugin";  // must match DLL name
  }
};

//  global plugin instance
OrbsPluginClass g_myComponents;

//--------------------------------------------------------------------------------------------
//  create a global instance of a VModule class
//  note: g_myComponentModule is defined in stdfx.h
//--------------------------------------------------------------------------------------------
DECLARE_THIS_MODULE(g_myComponentModule, MAKE_VERSION(1,0),
                    "Sample Plugin", 
                    "Havok",
                    "A sample plugin for entities", &g_myComponents);


//--------------------------------------------------------------------------------------------
//  Use this to get and initialize the plugin when you link statically
//--------------------------------------------------------------------------------------------
VEXPORT IVisPlugin_cl* GetEnginePlugin_OrbsPlugin(){  return &g_myComponents; }


#if (defined _DLL) || (defined _WINDLL)

  //  The engine uses this to get and initialize the plugin dynamically
  VEXPORT IVisPlugin_cl* GetEnginePlugin(){return GetEnginePlugin_OrbsPlugin();}

#endif // _DLL or _WINDLL


//============================================================================================================
//  Initialize our plugin.
//============================================================================================================
//  Called when the plugin is loaded
//  We add our component initialize data here
void OrbsPluginClass::OnInitEnginePlugin()
{
  hkvLog::Info("OrbsPluginClass:OnInitEnginePlugin()");
  Vision::RegisterModule(&g_myComponentModule);

  
// load plugins if supported
//#ifdef WIN_32
//#if defined( HAVOK_PHYSICS_2012_KEYCODE )
//  VISION_PLUGIN_ENSURE_LOADED(vHavok);
//#endif
//#if defined( HAVOK_AI_KEYCODE )
//  VISION_PLUGIN_ENSURE_LOADED(vHavokAi);
//#endif
//#if defined( HAVOK_BEHAVIOR_KEYCODE )
//  VISION_PLUGIN_ENSURE_LOADED(vHavokBehavior);
//#endif
//#endif
  
  VISION_PLUGIN_ENSURE_LOADED(vFmodEnginePlugin);

  // Start our game managers here....
  GameManager::GlobalManager().OneTimeInit();
  MeshRenderer::sharedInstance().OneTimeInit();
  CallFuncHandler::sharedInstance().OneTimeInit();
  AudioManager::sharedInstance().OneTimeInit();
  SceneManager::sharedInstance().initFirstScene();
}

// Called before the plugin is unloaded
void OrbsPluginClass::OnDeInitEnginePlugin()
{
  hkvLog::Info("OrbsPluginClass:OnDeInitEnginePlugin()");

  // Close our game managers here....
  AudioManager::sharedInstance().OneTimeDeInit();
  MeshRenderer::sharedInstance().OneTimeDeInit();
  CallFuncHandler::sharedInstance().OneTimeDeInit();
  GameManager::GlobalManager().OneTimeDeInit();

  // de-register our module when the plugin is de-initialized
  Vision::UnregisterModule(&g_myComponentModule);
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

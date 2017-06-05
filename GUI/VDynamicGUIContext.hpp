/// \file VGUIContext.hpp

#ifndef VDYNAMIC_GUI_CONTEXT_HPP_INCLUDED
#define VDYNAMIC_GUI_CONTEXT_HPP_INCLUDED

/// \brief
///   Resolution aware GUI context
class VDynamicGUIContext : public VGUIMainContext
{
protected:
  VRectanglef rReferenceResolution;

public:

  /// \brief
  ///   Constructor of the GUI context instance
  VDynamicGUIContext(VGUIManager *pManager, float fReferenceResolutionX = 2048.0f, float fReferenceResolutionY = 1536.0f) :
  VGUIMainContext(pManager), rReferenceResolution(0, 0, fReferenceResolutionX, fReferenceResolutionY)
  {
  };

  /// \brief
  ///   Destructor
  virtual ~VDynamicGUIContext()
  {
  };


  virtual void UpdateCursorPos(VGUIUserInfo_t &userState)
  {
    #if defined(SUPPORTS_MULTITOUCH)
      IVMultiTouchInput* pMultiTouch = static_cast<IVMultiTouchInput*>(&VInputDeviceManager::GetInputDevice(INPUT_DEVICE_TOUCHSCREEN));

      if (pMultiTouch->GetNumberOfTouchPoints()>0)
      {
        userState.m_vMousePosAccum.x = pMultiTouch->GetControlValue(CT_TOUCH_POINT_0_X, 0) * rReferenceResolution.GetSizeX();
        userState.m_vMousePosAccum.y = pMultiTouch->GetControlValue(CT_TOUCH_POINT_0_Y, 0) * rReferenceResolution.GetSizeY();
        
        // cursor positions should always be integer because of texture filtering
        userState.m_vMousePos.x = floorf(userState.m_vMousePosAccum.x);
        userState.m_vMousePos.y = floorf(userState.m_vMousePosAccum.y);
        
        rReferenceResolution.Clamp(userState.m_vMousePos);
        //Vision::Error.SystemMessage("Set cursor to: x: %f y: %f", userState.m_vMousePos.x, userState.m_vMousePos.y);
      }
    #else
      VGUIMainContext::UpdateCursorPos(userState);
    
      float fScaleX = rReferenceResolution.GetSizeX() / m_ClientRect.GetSizeX();
      float fScaleY = rReferenceResolution.GetSizeY() / m_ClientRect.GetSizeY();
    
      //convert to int
      userState.m_vMousePos.x = floorf(userState.m_vMousePosAccum.x * fScaleX);
      userState.m_vMousePos.y = floorf(userState.m_vMousePosAccum.y * fScaleY);
    
      rReferenceResolution.Clamp(userState.m_vMousePos);
    #endif
  }

  virtual void OnRender()
  {
    //VISION_PROFILE_FUNCTION(VGUIManager::PROFILING_RENDERING);
    IVRender2DInterface *pRenderer = Vision::RenderLoopHelper.BeginOverlayRendering();
    hkvVec4 transformation;
	//scale
	//float scale = min(m_ClientRect.GetSizeX() / rReferenceResolution.GetSizeX(), m_ClientRect.GetSizeY() / rReferenceResolution.GetSizeY());
	float scale, offsetX, offsetY;
	float clientRectRatio = m_ClientRect.GetSizeX() / m_ClientRect.GetSizeY();
	float referenceRatio = rReferenceResolution.GetSizeX() / rReferenceResolution.GetSizeY();
	if (clientRectRatio > referenceRatio)
	{
		scale = m_ClientRect.GetSizeY() / rReferenceResolution.GetSizeY();
		offsetX = 0.5f * (m_ClientRect.GetSizeX() - rReferenceResolution.GetSizeX() * scale);
		offsetY = 0;
	}
	else
	{
		scale = 1.0f;
		offsetX = 0;
		offsetY = 0;
	}
    transformation.x = scale;
    transformation.y = scale;
	//offset
	transformation.z = offsetX;
	transformation.w = offsetY;
    pRenderer->SetTransformation(&transformation);
    VItemRenderInfo defaultState(this, NULL);
    VGraphicsInfo Graphics(*pRenderer, *this);
    OnPaint(Graphics,defaultState);
    Vision::RenderLoopHelper.EndOverlayRendering();
  }

  virtual void OnPaint(VGraphicsInfo &Graphics, const VItemRenderInfo &parentState)
  {
	  VSimpleRenderState_t state = VGUIManager::DefaultGUIRenderState();
	  state.SetFlag(RENDERSTATEFLAG_FILTERING);
	  VGUIMainContext::OnPaint(Graphics, parentState);
  }

};

#endif


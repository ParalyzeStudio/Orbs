#include "OrbsPluginPCH.h"
#include "SceneGUIManager.h"
#include "GameManager.h"

void SceneGUIManager::initGUIContext()
{
	m_GUIContext = new VGUIMainContext(NULL);
	m_GUIContext->SetActivate(true);
	defineScale();

#ifdef _VISION_ANDROID
	VGUIManager::GlobalManager().LoadResourceFile("GUI\\MenuSystem_mobile.xml");
#else
	VGUIManager::GlobalManager().LoadResourceFile("GUI\\MenuSystem.xml");
#endif

	Vision::Callbacks.OnUpdateSceneBegin += this;
}

void SceneGUIManager::deInitGUIContext()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void SceneGUIManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference();
		update(dt);
	}
}

//////////////////////////////////Anchored2DObject//////////////////////////////////
V_IMPLEMENT_DYNCREATE(Anchored2DObject, VTypedObject, Vision::GetEngineModule());

Anchored2DObject::Anchored2DObject() : anchorPoint(hkvVec2(0,0)),
	scaling(false),
	translating(false),
	rotating(false),
	fading(false),
	currentScale(hkvVec2(0,0)),
	currentPosition(hkvVec2(0,0)),
	currentAngle(0),
	currentOpacity(1.0f),
	startScale(hkvVec2(0,0)),
	startPosition(hkvVec2(0,0)),
	startAngle(0),
	startOpacity(0),
	toScale(hkvVec2(0,0)),
	toPosition(hkvVec2(0,0)),
	toAngle(0),
	toOpacity(0),
	scaleDuration(0),
	rotationDuration(0),
	translationDuration(0),
	fadeDuration(0),
	scaleDelay(0),
	rotationDelay(0),
	translationDelay(0),
	fadeDelay(0),
	scaleElapsedTime(0),
	rotationElapsedTime(0),
	translationElapsedTime(0),
	fadeElapsedTime(0),
	parentPanel(NULL),
	m_iID(-1)
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

Anchored2DObject::~Anchored2DObject()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void Anchored2DObject::init(hkvVec2 position, float angle, hkvVec2 scale, float opacity, hkvVec2 anchorPoint)
{
	//this->SetScale(scale);
	//this->SetPosition(position);
	//this->SetAngle(angle);
	//this->SetOpacity(opacity);
	this->currentPosition = position;
	this->currentAngle = angle;
	this->currentScale = scale;
	this->currentOpacity = opacity;
	this->anchorPoint = anchorPoint;
}

void Anchored2DObject::SetScale(hkvVec2 scale)
{
	hkvVec2 deltaScale = scale - this->currentScale;
	this->IncScale(deltaScale);
}

void Anchored2DObject::SetPosition(hkvVec2 position)
{
	hkvVec2 deltaPosition = position - this->currentPosition;
	this->IncPosition(deltaPosition);
}
	
void Anchored2DObject::SetAngle(float angle)
{
	float deltaAngle = angle - this->currentAngle;
	this->IncRotation(deltaAngle);
}

void Anchored2DObject::SetOpacity(float opacity)
{
	float deltaOpacity = opacity - this->currentOpacity;
	this->IncOpacity(deltaOpacity);
}

void Anchored2DObject::IncScale(hkvVec2 &deltaScale)
{
	hkvVec2 newScale = currentScale + deltaScale;
	if (newScale.x < 0)
	{
		deltaScale.x = -currentScale.x;
		currentScale.x = 0;
	}
	else
		currentScale.x = newScale.x;
	if (newScale.y < 0)
	{
		deltaScale.y = -currentScale.y;
		currentScale.y = 0;
	}
	else
		currentScale.y = newScale.y;
}

void Anchored2DObject::IncPosition(hkvVec2 deltaPosition)
{
	currentPosition += deltaPosition;
}

void Anchored2DObject::IncRotation(float deltaAngle)
{
	currentAngle += deltaAngle; 
	GeometryUtils::normalizeAngle(this->currentAngle);
}

void Anchored2DObject::IncOpacity(float deltaOpacity)
{
	float newOpacity = currentOpacity + deltaOpacity;
	if (newOpacity > 1) 
	{
		deltaOpacity = 1 - currentOpacity;
		currentOpacity = 1;
		return;
	}
	if (newOpacity < 0) 
	{
		deltaOpacity = -currentOpacity;
		currentOpacity = 0;
		return;
	}
	
	//else
	currentOpacity = newOpacity;
}

void Anchored2DObject::updatePosScaleAndOpacityFromParent()
{
	//pos
	this->IncPosition(hkvVec2(0,0));

	//scale
	this->SetScale(parentPanel->GetScale());

	//opacity
	this->SetOpacity(parentPanel->GetOpacity());
}

hkvVec2 Anchored2DObject::GetAbsPosition()
{
	return this->parentPanel ? parentPanel->GetAbsPosition() + currentPosition : currentPosition;
}

hkvVec2 Anchored2DObject::scaleLinearVariation(float dt)
{
	return hkvVec2((toScale.x - startScale.x) / scaleDuration * dt, (toScale.y - startScale.y) / scaleDuration * dt);
}

float Anchored2DObject::angleLinearVariation(float dt)
{
	return (toAngle - startAngle) / rotationDuration * dt;
}

hkvVec2 Anchored2DObject::positionLinearVariation(float dt)
{
	return hkvVec2((toPosition.x - startPosition.x) / translationDuration * dt, (toPosition.y - startPosition.y) / translationDuration * dt);
}

float Anchored2DObject::opacityLinearVariation(float dt)
{
	return (toOpacity - startOpacity) / fadeDuration * dt;
}

void Anchored2DObject::scaleTo(hkvVec2 toScale, float duration, float delay)
{
	if (duration > 0)
	{
		this->scaling = true;
		this->startScale = currentScale;
		this->toScale = toScale;
		this->scaleDuration = duration;
		this->scaleElapsedTime = 0;
		this->scaleDelay = delay;
	}
}

void Anchored2DObject::rotateTo(float toAngle, float duration, float delay)
{
	if (duration > 0)
	{
		this->rotating = true;
		this->startAngle = currentAngle;
		this->toAngle = toAngle;
		this->rotationDuration = duration;
		this->rotationElapsedTime = 0;
		this->rotationDelay = delay;
	}
}

void Anchored2DObject::rotateBy(float byAngle)
{
	this->IncRotation(byAngle);
}

void Anchored2DObject::translateTo(hkvVec2 toPosition, float duration, float delay)
{
	if (duration > 0)
	{
		this->translating = true;
		this->startPosition = this->GetAbsPosition();
		this->toPosition = this->parentPanel ? this->parentPanel->GetAbsPosition() + toPosition: toPosition;
		this->translationDuration = duration;
		this->translationElapsedTime = 0;
		this->translationDelay = delay;
	}
}

void Anchored2DObject::fadeTo(float toOpacity, float duration, float delay)
{
	if (duration > 0)
	{
		this->fading = true;
		this->toOpacity = toOpacity;
		this->fadeDuration = duration;
		this->fadeElapsedTime = 0;
		this->fadeDelay = delay;
		this->startOpacity = currentOpacity;
	}
}

void Anchored2DObject::update(float dt)
{
	////SCALE
	if (scaling)
	{
		float scaleDt = dt;
		if (scaleElapsedTime < scaleDelay)
		{
			if ((scaleElapsedTime + scaleDt) <= scaleDelay)
				scaleElapsedTime += scaleDt;
			else
			{
				scaleDt = scaleDt - (scaleDelay - scaleElapsedTime);
				scaleElapsedTime = scaleDelay;
			}
		}
		if (scaleElapsedTime >= scaleDelay)
		{
			if (scaleElapsedTime + scaleDt > (scaleDuration + scaleDelay)) //last frame
			{
				scaleDt = scaleDuration + scaleDelay - scaleElapsedTime;
				scaling = false;
				scaleElapsedTime = 0;
			}
			else
				scaleElapsedTime += scaleDt;

			hkvVec2 variation = scaleLinearVariation(scaleDt);

			IncScale(variation);
		}
	}

	////TRANSLATION
	if (translating)
	{
		float translateDt = dt;
		if (translationElapsedTime < translationDelay)
		{
			if ((translationElapsedTime + translateDt) <= translationDelay)
				translationElapsedTime += translateDt;
			else
			{
				translateDt = translateDt - (translationDelay - translationElapsedTime);
				translationElapsedTime = translationDelay;
			}
		}

		//if (!inTranslateDelay)
		if (translationElapsedTime >= translationDelay)
		{
			if (translationElapsedTime + translateDt > (translationDuration + translationDelay)) //last frame
			{
				translateDt = translationDuration + translationDelay - translationElapsedTime;
				translating = false;
				translationElapsedTime = 0;
			}
			else
				translationElapsedTime += translateDt;

			hkvVec2 variation = positionLinearVariation(translateDt);
			IncPosition(variation);
		}
	}

	////ROTATION
	if (rotating)
	{
		//bool inRotateDelay = false;
		float rotateDt = dt;
		if (rotationElapsedTime < rotationDelay)
		{
			if ((rotationElapsedTime + rotateDt) <= rotationDelay)
				rotationElapsedTime += rotateDt;
			else
			{
				rotateDt = rotateDt - (rotationDelay - rotationElapsedTime);
				rotationElapsedTime = rotationDelay;
			}
		}

		if (rotationElapsedTime >= rotationDelay)
		{
			if (rotationElapsedTime + rotateDt > (rotationDuration + rotationDelay)) //last frame
			{
				rotateDt = rotationDuration + rotationDelay - rotationElapsedTime;
				rotating = false;
				rotationElapsedTime = 0;
			}
			else
				rotationElapsedTime += rotateDt;

			float variation = angleLinearVariation(rotateDt);

			IncRotation(variation);
		}
	}

	////OPACITY
	if (fading)
	{
		float fadeDt = dt;
		if (fadeElapsedTime < fadeDelay)
		{
			if ((fadeElapsedTime + fadeDt) <= fadeDelay)
				fadeElapsedTime += fadeDt;
			else
			{
				fadeDt = fadeDt - (fadeDelay - fadeElapsedTime);
				fadeElapsedTime = fadeDelay;
			}
		}
		if (fadeElapsedTime >= fadeDelay)
		{
			if (fadeElapsedTime + fadeDt > (fadeDuration + fadeDelay)) //last frame
			{
				fadeDt = fadeDuration + fadeDelay - fadeElapsedTime;
				fading = false;
				fadeElapsedTime = 0;
			}
			else
				fadeElapsedTime += fadeDt;

			float variation = opacityLinearVariation(fadeDt);

			IncOpacity(variation);
		}
	}
}

void Anchored2DObject::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference();
		update(dt);
	}
}

//////////////////////////////////ScreenMaskPanel//////////////////////////////////
V_IMPLEMENT_DYNCREATE(ScreenMaskPanel, Anchored2DObject, Vision::GetEngineModule());

ScreenMaskPanel::~ScreenMaskPanel()
{
	removeAllChildren();
}

void ScreenMaskPanel::init(hkvVec2 position, hkvVec2 scale, float opacity)
{
	Anchored2DObject::init(position, 0, scale, opacity);
}

void ScreenMaskPanel::addScreenMask(SpriteScreenMask* screenMask)
{
	screenMask->setParentPanel(this);
	this->childScreenMasks.push_back(screenMask);
}
void ScreenMaskPanel::addTextLabel(PrintTextLabel* textLabel)
{
	textLabel->setParentPanel(this); 
	this->textLabels.push_back(textLabel);
}

void ScreenMaskPanel::addPanel(ScreenMaskPanel* panel)
{
	panel->setParentPanel(this);
	this->panels.push_back(panel);
}

void ScreenMaskPanel::addList(ScrollingList* list)
{
	list->setParentPanel(this);
	this->lists.push_back(list);
}

void ScreenMaskPanel::addFountain(OrbFountain* fountain)
{
	fountain->setParentPanel(this);
	this->fountains.push_back(fountain);
}

void ScreenMaskPanel::removeAllChildren()
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		V_SAFE_RELEASE(childScreenMasks[childScreenMaskIndex]);
	}
	childScreenMasks.clear();

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		V_SAFE_RELEASE(textLabels[textLabelIndex]);
	}
	textLabels.clear();

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		V_SAFE_RELEASE(panels[panelIndex]);
	}
	panels.clear();

	for (int listIndex = 0; listIndex != lists.size(); listIndex++)
	{
		V_SAFE_RELEASE(lists[listIndex]);
	}
	lists.clear();

	for (int fountainIndex = 0; fountainIndex != fountains.size(); fountainIndex++)
	{
		V_SAFE_RELEASE(fountains[fountainIndex]);
	}
	fountains.clear();
}

void ScreenMaskPanel::removeScreenMask(SpriteScreenMask* screenMask)
{
	vector<SpriteScreenMask*>::iterator masksIt;
	for (masksIt = childScreenMasks.begin(); masksIt != childScreenMasks.end(); masksIt++)
	{
		if (*masksIt == screenMask)
		{
			childScreenMasks.erase(masksIt);
			screenMask->Release();
			return;
		}
	}
}

void ScreenMaskPanel::removeTextLabel(PrintTextLabel* textLabel)
{
	vector<PrintTextLabel*>::iterator textsIt;
	for (textsIt = textLabels.begin(); textsIt != textLabels.end(); textsIt++)
	{
		if (*textsIt == textLabel)
		{
			textLabels.erase(textsIt);
			textLabel->Release();
			return;
		}
	}
}

void ScreenMaskPanel::removePanel(ScreenMaskPanel* panel)
{
	vector<ScreenMaskPanel*>::iterator panelsIt;
	for (panelsIt = panels.begin(); panelsIt != panels.end(); panelsIt++)
	{
		if (*panelsIt == panel)
		{
			panels.erase(panelsIt);
			panel->Release();
			return;
		}
	}
}

void ScreenMaskPanel::removeList(ScrollingList* list)
{
	vector<ScrollingList*>::iterator listsIt;
	for (listsIt = lists.begin(); listsIt != lists.end(); listsIt++)
	{
		if (*listsIt == list)
		{
			lists.erase(listsIt);
			list->Release();
			return;
		}
	}
}

void ScreenMaskPanel::removeFountain(OrbFountain* fountain)
{
	vector<OrbFountain*>::iterator fountainsIt;
	for (fountainsIt = fountains.begin(); fountainsIt != fountains.end(); fountainsIt++)
	{
		if (*fountainsIt == fountain)
		{
			(*fountainsIt)->setParentPanel(NULL);
			fountains.erase(fountainsIt);
			return;
		}
	}
}

Anchored2DObject* ScreenMaskPanel::findElementById(const char* id)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* screenMask = childScreenMasks[childScreenMaskIndex];
		if (screenMask->m_iID == VGUIManager::GetID(id))
			return screenMask;
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = textLabels[textLabelIndex];
		if (textLabel->m_iID == VGUIManager::GetID(id))
			return textLabel;
	}

	for (int listIndex = 0; listIndex != lists.size(); listIndex++)
	{
		ScrollingList* list = lists[listIndex];
		if (list->m_iID == VGUIManager::GetID(id))
			return list;
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		if (panel->m_iID == VGUIManager::GetID(id))
			return panel;
		Anchored2DObject* foundElement = panel->findElementById(id);
		if (foundElement)
			return foundElement;
	}

	return NULL;
}
//
//void ScreenMaskPanel::SetScale(hkvVec2 scale)
//{
//	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
//	{
//		childScreenMasks[childScreenMaskIndex]->SetScale(scale);
//	}
//
//	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
//	{
//		textLabels[textLabelIndex]->SetScale(scale);
//	}
//
//	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
//	{
//		panels[panelIndex]->SetScale(scale);
//	}
//}
//
//void ScreenMaskPanel::SetOpacity(float opacity)
//{
//	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
//	{
//		childScreenMasks[childScreenMaskIndex]->SetOpacity(opacity);
//	}
//
//	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
//	{
//		textLabels[textLabelIndex]->SetOpacity(opacity);
//	}
//
//	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
//	{
//		panels[panelIndex]->SetOpacity(opacity);
//	}
//}
	
void ScreenMaskPanel::IncScale(hkvVec2 &deltaScale)
{
	Anchored2DObject::IncScale(deltaScale);
	this->IncChildsScale(deltaScale);
}

void ScreenMaskPanel::IncPosition(hkvVec2 deltaPosition)
{
	Anchored2DObject::IncPosition(deltaPosition);
	this->IncChildsPosition(deltaPosition);
}

void ScreenMaskPanel::IncRotation(float deltaAngle)
{
	Anchored2DObject::IncRotation(deltaAngle);
	this->IncChildsRotation(deltaAngle);
}

void ScreenMaskPanel::IncOpacity(float deltaOpacity)
{
	Anchored2DObject::IncOpacity(deltaOpacity);
	this->IncChildsOpacity(deltaOpacity);
}


void ScreenMaskPanel::IncChildsScale(hkvVec2 deltaScale)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];
		childScreenMask->IncScale(deltaScale);
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = textLabels[textLabelIndex];
		textLabel->IncScale(deltaScale);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->IncScale(deltaScale);
	}
}

void ScreenMaskPanel::IncChildsPosition(hkvVec2 deltaPosition)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];
		float posX, posY;
		childScreenMask->GetPos(posX, posY);
		posX += deltaPosition.x;
		posY += deltaPosition.y;
		childScreenMask->SetPos(posX, posY);
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		textLabels[textLabelIndex]->invalidatePos();
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->IncChildsPosition(deltaPosition);
	}

	for (int listIndex = 0; listIndex != lists.size(); listIndex++)
	{
		ScrollingList* list = lists[listIndex];
		list->IncPosition(deltaPosition);
	}
}

void ScreenMaskPanel::IncChildsRotation(float deltaAngle)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];

		hkvVec2 maskPosition = childScreenMask->GetPosition();
		GeometryUtils::rotate2DPoint(maskPosition, hkvVec2(0,0), deltaAngle);
		childScreenMask->SetPosition(maskPosition);
		childScreenMask->IncRotation(deltaAngle);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		hkvVec2 panelPosition = panel->GetPosition();
		GeometryUtils::rotate2DPoint(panelPosition, hkvVec2(0,0), deltaAngle);
		panel->SetPosition(panelPosition);
		panel->IncRotation(deltaAngle);
	}
}

void ScreenMaskPanel::IncChildsOpacity(float deltaOpacity)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];
		childScreenMask->IncOpacity(deltaOpacity);
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = textLabels[textLabelIndex];
		textLabel->IncOpacity(deltaOpacity);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->IncOpacity(deltaOpacity);
	}
}

void ScreenMaskPanel::SetOrder(int order)
{
	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		childScreenMasks[childScreenMaskIndex]->SetOrder(order);
	}
}

void ScreenMaskPanel::disable()
{
	float disableOpacity = 0.5f;
	this->currentOpacity = 0.5f;

	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];
		childScreenMask->SetOpacity(disableOpacity);
		if (vdynamic_cast<ButtonScreenMask*>(childScreenMask))
		//if (childScreenMask->isClickable())
			((ButtonScreenMask*) childScreenMask)->disable();
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = textLabels[textLabelIndex];
		textLabel->SetOpacity(disableOpacity);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->disable();
	}
}

void ScreenMaskPanel::enable()
{
	float enableOpacity = 1.0f;
	this->currentOpacity = 1.0f;

	for (int childScreenMaskIndex = 0; childScreenMaskIndex != childScreenMasks.size(); childScreenMaskIndex++)
	{
		SpriteScreenMask* childScreenMask = childScreenMasks[childScreenMaskIndex];
		childScreenMask->SetOpacity(enableOpacity);
		if (vdynamic_cast<ButtonScreenMask*>(childScreenMask))
		//if (childScreenMask->isClickable())
			((ButtonScreenMask*) childScreenMask)->enable();
	}

	for (int textLabelIndex = 0; textLabelIndex != textLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = textLabels[textLabelIndex];
		textLabel->SetOpacity(enableOpacity);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->enable();
	}
}

vector<PrintTextLabel*> ScreenMaskPanel::retrieveAllTextLabels()
{
	vector<PrintTextLabel*> allTextLabels;
	for (int textEntryIndex = 0; textEntryIndex != textLabels.size(); textEntryIndex++)
	{
		allTextLabels.push_back(textLabels[textEntryIndex]);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		vector<PrintTextLabel*> recursiveTextLabels = panels[panelIndex]->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), recursiveTextLabels.begin(), recursiveTextLabels.end());
	}

	for (int listIndex = 0; listIndex != lists.size(); listIndex++)
	{
		vector<ScreenMaskPanel*> listItems = lists[listIndex]->getPanels();
		for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
		{
			vector<PrintTextLabel*> recursiveTextLabels = listItems[panelIndex]->retrieveAllTextLabels();
			allTextLabels.insert(allTextLabels.end(), recursiveTextLabels.begin(), recursiveTextLabels.end());
		}
	}

	return allTextLabels;
}

////////////////////////////////////SpriteScreenMask////////////////////////////////////
V_IMPLEMENT_DYNCREATE(SpriteScreenMask, Anchored2DObject, Vision::GetEngineModule());

SpriteScreenMask::SpriteScreenMask()
	: VisScreenMask_cl(),
	Anchored2DObject(),
	usedAsButton(false),
	fixedRotation(false)
{	

}

SpriteScreenMask::~SpriteScreenMask()
{

}

void SpriteScreenMask::init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale, float angle, float opacity, hkvVec2 anchorPoint)
{
	Anchored2DObject::init(position, angle, scale, opacity, anchorPoint);
	this->unscaledTargetSize = unscaledTargetSize;
	this->SetTextureObject(textureObj);
	this->SetFiltering(true);
	this->SetTransparency(VIS_TRANSP_ALPHA);
	this->SetWrapping(false, false);	
	hkvVec2 targetSize = hkvVec2(scale.x * unscaledTargetSize.x, scale.y * unscaledTargetSize.y);
	this->SetTargetSize(targetSize.x, targetSize.y);
	hkvVec2 pos = hkvVec2(this->GetAbsPosition().x - anchorPoint.x * targetSize.x, this->GetAbsPosition().y - anchorPoint.y * targetSize.y);
	this->SetPos(pos.x, pos.y);
	this->SetOrder(DEFAULT_ORDER);
	this->SetRotationCenter(anchorPoint.x * targetSize.x, anchorPoint.y * targetSize.y);
	this->SetRotationAngle(angle);
	
	//alpha
	this->SetOpacity(opacity);
}

void SpriteScreenMask::init(hkvVec2 position, hkvVec2 unscaledTargetSize, hkvVec4 textureRange, VTextureObject* textureObj, bool repeatTexture, hkvVec2 scale, float angle, float opacity, hkvVec2 anchorPoint)
{
	SpriteScreenMask::init(position, unscaledTargetSize, textureObj, scale, angle, opacity, anchorPoint);
	this->SetWrapping(repeatTexture, repeatTexture);
	this->SetTextureRange(textureRange.x, textureRange.y, textureRange.z, textureRange.w);
}

void SpriteScreenMask::SetUnscaledTargetSize(hkvVec2 size)
{
	this->unscaledTargetSize = size;
	float targetSizeX = size.x * currentScale.x;
	float targetSizeY = size.y * currentScale.y;
	this->SetTargetSize(targetSizeX, targetSizeY);

	hkvVec2 pos = hkvVec2(this->GetAbsPosition().x - anchorPoint.x * targetSizeX, this->GetAbsPosition().y - anchorPoint.y * targetSizeY);
	this->SetPos(pos.x, pos.y);
}

void SpriteScreenMask::IncScale(hkvVec2 &deltaScale)
{
	Anchored2DObject::IncScale(deltaScale);
	hkvVec2 targetSize = hkvVec2(this->currentScale.x * unscaledTargetSize.x, this->currentScale.y * unscaledTargetSize.y);
	this->SetTargetSize(targetSize.x, targetSize.y);
	hkvVec2 pos = this->GetAbsPosition() - hkvVec2(anchorPoint.x * targetSize.x, anchorPoint.y * targetSize.y);
	this->SetPos(pos.x, pos.y);
	this->SetRotationCenter(0.5f * targetSize.x, 0.5f * targetSize.y);
}

void SpriteScreenMask::IncPosition(hkvVec2 deltaPosition)
{
	Anchored2DObject::IncPosition(deltaPosition);
	hkvVec2 absPosition = this->GetAbsPosition();
	hkvVec2 pos = hkvVec2(absPosition.x - anchorPoint.x * currentScale.x * unscaledTargetSize.x, absPosition.y - anchorPoint.y * currentScale.y * unscaledTargetSize.y); //update the position
	this->SetPos(pos.x, pos.y);
}

void SpriteScreenMask::IncRotation(float deltaAngle)
{
	if (!fixedRotation)
	{
		Anchored2DObject::IncRotation(deltaAngle);
		this->SetRotationCenter(anchorPoint.x * currentScale.x * unscaledTargetSize.x, anchorPoint.y * currentScale.y * unscaledTargetSize.y);
		this->SetRotationAngle(currentAngle);
	}
}

void SpriteScreenMask::IncOpacity(float deltaOpacity)
{
	Anchored2DObject::IncOpacity(deltaOpacity);

	hkvVec4 color = this->GetColor().getAsVec4();
	color.w = currentOpacity;

	VColorRef newColorRef = VColorRef::Float_To_RGBA(color);

	this->SetColor(newColorRef);
}

void SpriteScreenMask::updateTexture(VTextureObject* newTexture)
{
	hkvVec2 scale = this->GetScale();
	hkvVec2 position = this->GetPosition();
	float angle = this->GetAngle();
	hkvVec2 unscaledTargetSize = this->GetUnscaledTargetSize();
	int order = this->GetOrder();

	this->init(position, unscaledTargetSize, newTexture, scale, angle);
	this->SetOrder(order);
}

bool SpriteScreenMask::containsTouch(hkvVec2 touchLocation)
{
	hkvVec2 transformedTouchLocation = touchLocation;
	GeometryUtils::rotate2DPoint(transformedTouchLocation, this->GetAbsPosition(), -this->currentAngle);

	hkvVec2 currentSize = hkvVec2(currentScale.x * unscaledTargetSize.x, currentScale.y * unscaledTargetSize.y);
	hkvVec2 absPosition = this->GetAbsPosition();
	hkvVec2 bboxMin = hkvVec2(this->GetAbsPosition().x - 0.5f * currentSize.x, this->GetAbsPosition().y - 0.5f * currentSize.y);
	hkvVec2 bboxMax = hkvVec2(this->GetAbsPosition().x + 0.5f * currentSize.x, this->GetAbsPosition().y + 0.5f * currentSize.y);
	VRectanglef bbox = VRectanglef(bboxMin, bboxMax);

	return bbox.IsInside(transformedTouchLocation);
}

////////////////////////////////////RotatingSpriteScreenMask////////////////////////////////////
void RotatingSpriteScreenMask::init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, float rotationSpeed, hkvVec2 scale, float angle)
{
	SpriteScreenMask::init(position, unscaledTargetSize, textureObj, scale, angle);
	this->rotationSpeed = rotationSpeed;
	this->SetRotationCenter(anchorPoint.x * scale.x * unscaledTargetSize.x, anchorPoint.y * scale.y * unscaledTargetSize.y); //circles rotate around their center
}

void RotatingSpriteScreenMask::update(float dt)
{
	SpriteScreenMask::update(dt);
	hkvVec2 currentSize = hkvVec2(this->currentScale.x * this->unscaledTargetSize.x, this->currentScale.y * this->unscaledTargetSize.y);
	this->SetRotationCenter(anchorPoint.x * currentSize.x, anchorPoint.y * currentSize.y); //circles rotate around their center
	this->rotateBy(rotationSpeed * dt);
}

////////////////////////////////////Touchable////////////////////////////////////
bool Touchable::trigger(hkvVec2 touchLocation, int eventType)
{
	if (eventType == POINTER_DOWN)
	{
		if (this->containsTouch(touchLocation))
		{
			this->select();
			this->onPointerDown(touchLocation);
			return true;
		}
		else
			return false;
	}
	else if (eventType == POINTER_MOVE)
	{
		this->onPointerMove(touchLocation);
		return this->selected;
	}
	else if (eventType == POINTER_UP)
	{
		if (this->selected)
		{
			bool touchContained = this->containsTouch(touchLocation);
			this->onPointerUp(touchLocation);
			return touchContained;
			//if (this->containsTouch(touchLocation))
			//{
			//	this->onPointerUp(touchLocation);
			//	return true;
			//}
			//else
			//{
			//	this->deselect();
			//	return false;
			//}
		}
	}
	else if (eventType == POINTER_CANCEL)
	{
		this->deselect();
		return true;
	}
	return false;
}

void Touchable::onPointerDown(hkvVec2 touchLocation)
{
	this->lastTouchLocation = touchLocation;
}

bool Touchable::onPointerMove(hkvVec2 touchLocation)
{
	if (touchLocation.isEqual(lastTouchLocation, 0.001f))
		return false;

	this->deltaTouchLocation = touchLocation - this->lastTouchLocation;
	this->lastTouchLocation = touchLocation;
	return true;
}

void Touchable::onPointerUp(hkvVec2 touchLocation)
{
	this->lastTouchLocation = touchLocation;
	this->deselect();
}

void Touchable::select()
{
	this->selected = true;
}

void Touchable::deselect()
{
	this->selected = false;
}

////////////////////////////////////Clickable////////////////////////////////////
bool Clickable::trigger(hkvVec2 touchLocation, int eventType)
{
	if (!enabled || eventType == POINTER_MOVE) //don't handle disable items or move event
		return false;

	return Touchable::trigger(touchLocation, eventType);
}

void Clickable::disable()
{
	this->enabled = false;
}

void Clickable::enable()
{
	this->enabled = true;
}

void Clickable::onPointerUp(hkvVec2 touchLocation)
{
	Touchable::onPointerUp(touchLocation);
	if (this->containsTouch(touchLocation))
		this->onClick();
}

////////////////////////////////////ButtonScreenMask////////////////////////////////////
V_IMPLEMENT_DYNCREATE(ButtonScreenMask, SpriteScreenMask, Vision::GetEngineModule());

ButtonScreenMask::ButtonScreenMask() : SpriteScreenMask()
{
	this->usedAsButton = true;
}

void ButtonScreenMask::init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale, float angle, float opacity, VColorRef enabledColor, VColorRef disabledColor, VColorRef selectedColor)
{
	SpriteScreenMask::init(position, unscaledTargetSize, textureObj, scale, angle, opacity);
	this->enabledColor = enabledColor;
	this->disabledColor = disabledColor;
	this->selectedColor = selectedColor;
	this->enabled = true;
	this->SetColor(enabledColor);
}

void ButtonScreenMask::disable()
{
	//this->SetColor(disabledColor);
	this->enabled = false;
	//this->SetOpacity(0.5f);
}

void ButtonScreenMask::enable()
{
	//this->SetColor(enabledColor);
	this->enabled = true;
	//this->SetOpacity(1.0f);
}

void ButtonScreenMask::select()
{
	Clickable::select();
	this->SetColor(selectedColor);
}

void ButtonScreenMask::deselect()
{
	Clickable::deselect();
	this->SetColor(enabledColor);
}

bool ButtonScreenMask::trigger(hkvVec2 touchLocation, int eventType)
{
	return Clickable::trigger(touchLocation, eventType);
}

bool ButtonScreenMask::containsTouch(hkvVec2 touchLocation)
{
	return SpriteScreenMask::containsTouch(touchLocation);
}

////////////////////////////////////ClickableScreenMaskPanel////////////////////////////////////
V_IMPLEMENT_DYNCREATE(ClickableScreenMaskPanel, ScreenMaskPanel, Vision::GetEngineModule());

void ClickableScreenMaskPanel::init(hkvVec2 position, hkvVec2 scale, float opacity)
{
	ClickableScreenMaskPanel::init(position, hkvVec2(0,0), scale, opacity);
}

void ClickableScreenMaskPanel::init(hkvVec2 position, hkvVec2 touchArea, hkvVec2 scale, float opacity)
{
	ScreenMaskPanel::init(position, scale, opacity);
	this->touchArea = touchArea;
	this->enabled = true;
}

bool ClickableScreenMaskPanel::containsTouch(hkvVec2 touchLocation)
{
	hkvVec2 transformedTouchLocation = touchLocation;
	GeometryUtils::rotate2DPoint(transformedTouchLocation, this->GetAbsPosition(), -this->currentAngle);

	hkvVec2 absPosition = this->GetAbsPosition();
	hkvVec2 bboxMin = absPosition - anchorPoint.compMul(touchArea);
	hkvVec2 bboxMax = absPosition + anchorPoint.compMul(touchArea);
	VRectanglef bbox = VRectanglef(bboxMin, bboxMax);

	return bbox.IsInside(transformedTouchLocation);
}

void ClickableScreenMaskPanel::disable()
{
	Clickable::disable();
	this->SetOpacity(0.5f);
}

void ClickableScreenMaskPanel::enable()
{
	Clickable::enable();
	this->SetOpacity(1.0f);
}

void ClickableScreenMaskPanel::onClick()
{
	return;
}

////////////////////////////////////ScrollingList////////////////////////////////////
ScrollingList::~ScrollingList()
{
	for (int textIndex = 0; textIndex != texts.size(); textIndex++)
	{
		texts[textIndex]->Release();
	}

	for (int maskIndex = 0; maskIndex != masks.size(); maskIndex++)
	{
		masks[maskIndex]->Release();
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		panels[panelIndex]->Release();
	}
}

void ScrollingList::init(hkvVec2 position, float width, float height, bool autoscroll, float autoscrollSpeed)
{
	Anchored2DObject::init(position);
	this->width = width;
	this->height = height;
	this->autoscroll = autoscroll;
	this->autoscrollSpeed = autoscrollSpeed;
	this->pos = this->GetAbsPosition() - hkvVec2(anchorPoint.x * width, anchorPoint.y * height);
	this->lastPage = -1;
	this->currentPage = 0;
}

void ScrollingList::addTextEntry(PrintTextLabel* textEntry, hkvVec2 offset)
{
	textEntry->AddRef();
	texts.push_back(textEntry);
	float posX = this->GetAbsPosition().x - 0.5f * textEntry->getSize().x + offset.x;
	float posY = this->getPos().y + this->totalLength + offset.y;
	textEntry->setAbsPos(hkvVec2(posX, posY));

	this->totalLength += (textEntry->getSize().y + offset.y);
}

void ScrollingList::addMaskEntry(SpriteScreenMask* maskEntry, hkvVec2 offset)
{
	maskEntry->AddRef();
	masks.push_back(maskEntry);

	float maskWidth, maskHeight;
	maskEntry->GetTargetSize(maskWidth, maskHeight);
	hkvVec2 maskPosition = hkvVec2(this->GetAbsPosition().x + offset.x, this->getPos().y + this->totalLength + 0.5f * maskHeight + offset.y);
	maskEntry->SetPosition(maskPosition);

	this->totalLength += (maskHeight + offset.y);
}

void ScrollingList::addPanelEntry(ScreenMaskPanel* panelEntry, hkvVec2 panelSize, int page, hkvVec2 offset)
{
	if (page > lastPage)
	{
		this->totalLength = 0;
		lastPage++;
	}

	offset += hkvVec2(page * this->getSize().x, 0);
	panelEntry->AddRef();
	panels.push_back(panelEntry);
	hkvVec2 panelPosition;
	panelPosition = hkvVec2(this->GetAbsPosition().x + offset.x, this->getPos().y + this->totalLength + 0.5f * panelSize.y + offset.y);
	panelEntry->SetPosition(panelPosition);

	this->totalLength += (panelSize.y + offset.y);
}

void ScrollingList::pageSwipe(bool toNextPage)
{
	hkvLog::Error("swiping:%s", swiping ? "TRUE" : "FALSE");
	if (this->swiping)
		return;

	hkvVec2 toPosition;
	if (toNextPage && currentPage < lastPage)
	{
		this->currentPage++;
		toPosition = this->GetPosition() - hkvVec2(this->getSize().x, 0);
		this->swiping = true;
	}
	else if (!toNextPage && currentPage > 0)
	{
		this->currentPage--;
		toPosition = this->GetPosition() + hkvVec2(this->getSize().x, 0);
		this->swiping = true;
	}

	if (swiping)
	{
		this->onPageSwipe(this->currentPage);
		this->translateTo(toPosition, 0.5f, 0.0f);
		CallFuncObject* stopSwipingAction = new CallFuncObject(this, (CallFunc)(&ScrollingList::stopSwiping), translationDuration);
		CallFuncHandler::sharedInstance().addCallFuncInstance(stopSwipingAction);
		stopSwipingAction->executeFunc();
	}
}

void ScrollingList::stopSwiping()
{
	this->swiping = false;
}

void ScrollingList::IncChildsPosition(hkvVec2 deltaPos)
{
	//inc the position of text entries
	for (int textEntryIndex = 0; textEntryIndex != texts.size(); textEntryIndex++)
	{
		PrintTextLabel* textEntry = texts[textEntryIndex];
		textEntry->IncPosition(deltaPos);
		adjustOpacity(textEntry);
	}

	for (int maskEntryIndex = 0; maskEntryIndex != masks.size(); maskEntryIndex++)
	{
		SpriteScreenMask* maskEntry = masks[maskEntryIndex];
		maskEntry->IncPosition(deltaPos);
		adjustOpacity(maskEntry);
	}

	for (int panelEntryIndex = 0; panelEntryIndex != panels.size(); panelEntryIndex++)
	{
		ScreenMaskPanel* panelEntry = panels[panelEntryIndex];
		panelEntry->IncPosition(deltaPos);
		adjustOpacity(panelEntry);
	}
}

void ScrollingList::renderPrintTextLabels()
{
	/*IVRender2DInterface* pRI = Vision::RenderLoopHelper.BeginOverlayRendering();
	for (int textEntryIndex = 0; textEntryIndex != texts.size(); textEntryIndex++)
	{
		PrintTextLabel* textEntry = texts[textEntryIndex];
		
		string text = textEntry->getText();
		VisFont_cl* font = textEntry->getFont();
		float fontHeight = textEntry->getFontHeight();
		float fontScaling = fontHeight / font->GetFontHeight();
		VColorRef fontColor = textEntry->getFontColor();
		hkvVec2 pos = textEntry->getPos();
		VSimpleRenderState_t renderState = VisFont_cl::DEFAULT_STATE;
		renderState.SetFlag(RENDERSTATEFLAG_FILTERING);
		font->PrintText(pRI, pos, text.c_str(), fontColor, renderState, fontScaling);
	}
	Vision::RenderLoopHelper.EndOverlayRendering();*/
}

void ScrollingList::IncPosition(hkvVec2 deltaPosition)
{
	Anchored2DObject::IncPosition(deltaPosition);
	hkvVec2 absPosition = this->GetAbsPosition();
	this->pos = absPosition - anchorPoint.compMul(this->getSize());

	IncChildsPosition(deltaPosition);
}

void ScrollingList::IncOpacity(float deltaOpacity)
{
	Anchored2DObject::IncOpacity(deltaOpacity);
	for (int maskIndex = 0; maskIndex != masks.size(); maskIndex++)
	{
		SpriteScreenMask* mask = masks[maskIndex];
		mask->IncOpacity(deltaOpacity);
	}

	for (int textLabelIndex = 0; textLabelIndex != texts.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = texts[textLabelIndex];
		textLabel->IncOpacity(deltaOpacity);
	}

	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* panel = panels[panelIndex];
		panel->IncOpacity(deltaOpacity);
	}
}

bool ScrollingList::containsTouch(hkvVec2 touchLocation)
{
	VRectanglef bbox = VRectanglef(this->GetAbsPosition().x - 0.5f * this->getSize().x,
								   this->GetAbsPosition().y - 0.5f * this->getSize().y,
								   this->GetAbsPosition().x + 0.5f * this->getSize().x,
								   this->GetAbsPosition().y + 0.5f * this->getSize().y);

	return bbox.IsInside(touchLocation);
}

bool ScrollingList::onPointerMove(hkvVec2 touchLocation)
{
	hkvVec2 tmpLastTouchLocation = this->lastTouchLocation;
	if (!Touchable::onPointerMove(touchLocation))
		return false;

	if (this->selected)
	{
		hkvVec2 deltaPosition = tmpLastTouchLocation - touchLocation;

		float dx = deltaPosition.y;
		scroll(-dx);
		return true;
	}
	return false;
}

void ScrollingList::update(float dt)
{
	Anchored2DObject::update(dt);

	if (autoscroll && !selected)
	{
		float dx = autoscrollSpeed * dt;
		scroll(-dx); //down to top
	}
}

////////////////////////////////////PrintTextLabel////////////////////////////////////
PrintTextLabel::PrintTextLabel() : Anchored2DObject(),
	text(""),
	absPos(hkvVec2(0,0)),
	size(hkvVec2(0,0)),
	font(NULL),
	unscaledFontHeight(0),
	fontHeight(0),
	iColor(V_RGBA_WHITE),
	renderedBehindScreenMasks(false),
	renderedBehindVeils(false)
{
	Vision::Callbacks.OnRenderHook += this;
}

PrintTextLabel::~PrintTextLabel()
{
	Vision::Callbacks.OnRenderHook -= this;
}

void PrintTextLabel::init(hkvVec2 position, string text, VisFont_cl* font, float unscaledFontHeight, VColorRef iColor)
{
	Anchored2DObject::init(position);

	this->text = text;
	this->font = font;
	this->iColor = iColor;
	this->unscaledFontHeight = unscaledFontHeight;
	this->fontHeight = unscaledFontHeight;
	this->renderedBehindScreenMasks = false;
	this->renderedBehindVeils = false;

	this->setText(text);
}

void PrintTextLabel::IncScale(hkvVec2 &deltaScale)
{
	Anchored2DObject::IncScale(deltaScale);
	this->fontHeight = this->unscaledFontHeight * this->currentScale.x; //uniform scaling take the x-coords
	if (text.length() == 0)
		this->size = hkvVec2(0,0);
	else
	{
		VRectanglef textDim;
		font->GetTextDimension(text.c_str(), textDim);
		float fontHeightRatio = fontHeight / font->GetFontHeight();
		textDim.operator*=(fontHeightRatio);
		this->size = hkvVec2(textDim.GetSizeX(), textDim.GetSizeY());
	}
	
	invalidatePos();
}

void PrintTextLabel::IncPosition(hkvVec2 deltaPosition)
{
	Anchored2DObject::IncPosition(deltaPosition);
	//this->absPos += deltaPosition;
	invalidatePos();
}

void PrintTextLabel::IncOpacity(float deltaOpacity)
{
	Anchored2DObject::IncOpacity(deltaOpacity);
	hkvVec4 colorVec4 = this->iColor.getAsVec4();
	colorVec4.w = this->currentOpacity;
	this->iColor = VColorRef::Float_To_RGBA(colorVec4);
}

void PrintTextLabel::renderItself()
{
	IVRender2DInterface* pRI = Vision::RenderLoopHelper.BeginOverlayRendering();

	float fontScaling = fontHeight / font->GetFontHeight();
	VSimpleRenderState_t renderState = VisFont_cl::DEFAULT_STATE;
	renderState.SetFlag(RENDERSTATEFLAG_FILTERING);
	font->PrintText(pRI, absPos, text.c_str(), iColor, renderState, fontScaling);

	Vision::RenderLoopHelper.EndOverlayRendering();
}

void PrintTextLabel::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	Anchored2DObject::OnHandleCallback(pData);
	if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
		if (!renderedBehindScreenMasks && pRHDO->m_iEntryConst == VRH_GUI)
		{		
			renderItself();
		}
		else if (renderedBehindScreenMasks && pRHDO->m_iEntryConst == VRH_PRE_SCREENMASKS)
		{
			renderItself();
		}
	}
}

void PrintTextLabel::setText(string text)
{
	this->text = text;
	if (text.length() == 0)
		this->size = hkvVec2(0,0);
	else
	{
		VRectanglef textDim;
		font->GetTextDimension(text.c_str(), textDim);
		float fontHeightRatio = this->fontHeight / font->GetFontHeight();
		textDim.operator*=(fontHeightRatio);
		this->size = hkvVec2(textDim.GetSizeX(), textDim.GetSizeY());
	}
	invalidatePos();
}

void PrintTextLabel::invalidatePos()
{
	this->absPos = this->GetAbsPosition() - anchorPoint.compMul(size);
}

void PrintTextLabel::setAbsPos(hkvVec2 pos)
{
	hkvVec2 deltaPos = pos - this->absPos;
	this->absPos = pos;
	this->currentPosition += deltaPos;
}

////////////////////////////////////IncrementalNumberTextLabel////////////////////////////////////
IncrementalNumberTextLabel::IncrementalNumberTextLabel() : PrintTextLabel(), 
	incrementing(false),
	startNumber(0), 
	endNumber(0), 
	speed(0), 
	incrementingElapsedTime(0), 
	incrementingDuration(0)
{

}

void IncrementalNumberTextLabel::init(hkvVec2 position, VisFont_cl* font, float unscaledFontHeight, VColorRef iColor, int startNumber, int endNumber, float speed)
{
	PrintTextLabel::init(position, "", font, unscaledFontHeight, iColor);
	this->startNumber = startNumber;
	this->endNumber = endNumber;
	this->speed = speed;
}

void IncrementalNumberTextLabel::startIncrementing()
{
	this->incrementing = true;
	this->incrementingDuration = (endNumber - startNumber) / speed;
	this->incrementingElapsedTime = 0;
}

int IncrementalNumberTextLabel::getNumberAt(float time)
{
	float timeRatio = incrementingElapsedTime / incrementingDuration;
	return GeometryUtils::round(timeRatio * (endNumber - startNumber) + startNumber);
}

void IncrementalNumberTextLabel::update(float dt)
{
	PrintTextLabel::update(dt);

	if (incrementing)
	{
		incrementingElapsedTime += dt;
		int number;
		if (incrementingElapsedTime > incrementingDuration)
			number = endNumber;
		else
			number = getNumberAt(incrementingElapsedTime);
		this->setText(stringFromInt(number).c_str());
	}
}

////////////////////////////////////ItemList////////////////////////////////////
void ItemList::init(hkvVec2 position, float width, float height, hkvVec2 itemSize)
{
	ScrollingList::init(position, width, height, false, 0);
	this->itemSize = itemSize;
	this->stabilizingSpeed = DEFAULT_STABILIZING_SPEED;


	if (this->panels.size() > 0)
	{
		this->currentItem = panels[0];
		this->currentItemIndex = 0;
	}
}

void ItemList::scroll(float dx)
{
	for (int itemIndex = 0; itemIndex != panels.size(); itemIndex++)
	{
		ScreenMaskPanel* item = panels[itemIndex];
		hkvVec2 scrollVec = hkvVec2(0, dx);
		item->IncPosition(scrollVec);
		this->adjustOpacity(item);
		this->adjustScale(item);
	}
}

void ItemList::scrollUpOneElement()
{
	if (stabilizing)
		return;

	if (currentItemIndex > 0)
	{
		this->currentItemIndex--;
		this->currentItem = panels[currentItemIndex];
		stabilizeList(false);
		this->onItemSelected();
	}
}

void ItemList::scrollDownOneElement()
{
	if (stabilizing)
		return;

	if (currentItemIndex != panels.size() - 1)
	{
		this->currentItemIndex++;
		this->currentItem = panels[currentItemIndex];
		stabilizeList(false);
		this->onItemSelected();
	}
}

void ItemList::findSelectedItem()
{
	int oldCurrentItemIndex = this->currentItemIndex;

	float minDistance = INT_MAX;
	ScreenMaskPanel* selectedItem = NULL;
	for (int itemIndex = 0; itemIndex != panels.size(); itemIndex++)
	{
		ScreenMaskPanel* item = panels[itemIndex];
		hkvVec2 distanceToCenter = item->GetPosition() - this->GetPosition();
		float distance = abs(distanceToCenter.y);
		if (distance < minDistance)
		{
			this->currentItem = item;
			this->currentItemIndex = itemIndex;
			minDistance = distance;
		}
	}	

	if (oldCurrentItemIndex != this->currentItemIndex)
		this->onItemSelected();
}

void ItemList::stabilizeList(bool performFindSelectedItem)
{
	this->stabilizing = true;
	if (performFindSelectedItem)
		findSelectedItem();
	float stabilizingDistance = abs(this->currentItem->GetPosition().y - this->GetPosition().y);
	stabilizeStartPosition = currentItem->GetPosition();
	stabilizingTimer = stabilizingDistance / stabilizingSpeed;
}

void ItemList::onPointerDown(hkvVec2 touchLocation)
{
	if (!stabilizing)
	{
		ScrollingList::onPointerDown(touchLocation);
		hkvLog::Error("ItemList OnPointerDown SELECTED:%s", selected ? "TRUE" : "FALSE");
	}
}

void ItemList::onPointerUp(hkvVec2 touchLocation)
{
	if (!stabilizing)
	{
		ScrollingList::onPointerUp(touchLocation);
		stabilizeList(true);
	}
}

bool ItemList::onPointerMove(hkvVec2 touchLocation)
{
	if (!stabilizing)
	{
		return ScrollingList::onPointerMove(touchLocation);
	}
	return false;
}

void ItemList::update(float dt)
{
	ScrollingList::update(dt);
	if (stabilizing)
	{
		stabilizingTimer -= dt;
		float dx;
		if (stabilizingTimer < 0)
		{
			dx = this->GetPosition().y - this->currentItem->GetPosition().y;
			stabilizing = false;
		}	 
		else
		{
			int signOfVariation = (this->GetPosition().y > this->currentItem->GetPosition().y) ? 1 : -1;
			dx = signOfVariation * stabilizingSpeed * dt;
		}
		scroll(dx);
	}
}

////////////////////////////////////ParticleEffectContainer////////////////////////////////////
ParticleEffectContainer::ParticleEffectContainer() : particleEffect(NULL),
	position(hkvVec3(0,0,0)),
	billboarding(false),
	rotationMatrix(hkvMat3()),
	scale(0)
{

}

ParticleEffectContainer::~ParticleEffectContainer()
{
	particleEffect->DisposeObject();
	particleEffect = NULL;
}

void ParticleEffectContainer::init(VisParticleEffectFile_cl* effectFile, hkvVec3 position, bool billboarding, hkvMat3 rotationMatrix, float scale)
{
	this->position = position;
	this->billboarding = billboarding;
	this->rotationMatrix = rotationMatrix;
	this->scale = scale;

	hkvVec3 orientation = hkvVec3(0,0,0);
	if (!billboarding)
	{
		VisObject3D_cl tmpObject;
		tmpObject.SetUseEulerAngles(false);
		tmpObject.SetRotationMatrix(rotationMatrix);
		orientation = tmpObject.GetActualOrientation();
	}

	particleEffect = effectFile->CreateParticleEffectInstance(position, orientation, scale);
}

////////////////////////////////////Veil////////////////////////////////////
void Veil::init(VColorRef color, float opacity)
{
	hkvVec2 screenSize = hkvVec2(0,0);
	if (MenuManager::sharedInstance()->isContextActive())
		screenSize = MenuManager::sharedInstance()->getScreenSize();
	if (GameDialogManager::sharedInstance()->isContextActive())
		screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	VTextureObject* whiteTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	SpriteScreenMask::init(0.5f * screenSize, screenSize, whiteTexture);
	this->SetColor(color);
	this->SetOpacity(opacity);
}

void Veil::IncOpacity(float deltaOpacity)
{
	SpriteScreenMask::IncOpacity(deltaOpacity);

	vector<PrintTextLabel*> backgroundTextLabels;
	if (MenuManager::sharedInstance()->isContextActive())
		backgroundTextLabels = MenuManager::sharedInstance()->retrieveAllTextLabels();
	if (GameDialogManager::sharedInstance()->isContextActive())
		backgroundTextLabels = GameDialogManager::sharedInstance()->retrieveAllTextLabels();

	for (int textLabelIndex = 0; textLabelIndex != backgroundTextLabels.size(); textLabelIndex++)
	{
		PrintTextLabel* textLabel = backgroundTextLabels[textLabelIndex];
		if (textLabel->isRenderedBehindVeils())
		{
			float textLabelOpacity = textLabel->GetOpacity();
			float textLabelNewOpacity = 1 - this->currentOpacity;
			if (deltaOpacity >= 0 && textLabelOpacity > textLabelNewOpacity
				||
				deltaOpacity <= 0 && textLabelOpacity < textLabelNewOpacity)
				textLabel->SetOpacity(textLabelNewOpacity);
		}
	}
}

////////////////////////////////////TransitionVeil////////////////////////////////////
void TransitionVeil::init(VColorRef color, CallFuncObject* action, float peakDuration, float fadeInDuration, float fadeOutDuration)
{
	Veil::init(color, 0);
	this->action = action;
	this->peakDuration = peakDuration;
	this->fadeInDuration = fadeInDuration;
	this->fadeOutDuration = fadeOutDuration;
}

void TransitionVeil::update(float dt)
{
	float oldTime = veilElapsedTime;
	veilElapsedTime += dt;

	if (veilElapsedTime < fadeInDuration)
	{
		float deltaOpacity = dt / fadeOutDuration;
		this->SetOpacity(this->currentOpacity + deltaOpacity);
	}
	else if (oldTime < fadeInDuration &&  veilElapsedTime >= fadeInDuration)
	{
		//execute the function
		if (action)
		{
			VBaseObject* target = action->getTarget();
			CallFunc func = action->getFunc();
			(target->*func)();

			this->SetOpacity(1.0f);
		}
	}
	else if (veilElapsedTime > (fadeInDuration + peakDuration) && veilElapsedTime <= (fadeInDuration + peakDuration + fadeOutDuration))
	{
		float deltaOpacity = -dt / fadeOutDuration;
		this->SetOpacity(this->currentOpacity + deltaOpacity);
	}
	else if (veilElapsedTime > (fadeInDuration + peakDuration + fadeOutDuration))
	{
		this->SetOpacity(0);
		this->running = false;
		MenuManager::sharedInstance()->removeVeil();
	}
}

////////////////////////////////////YesNoDialogPanel////////////////////////////////////
void YesNoDialogPanel::init(string text)
{
	SceneGUIManager* guiManager;
	if (GameDialogManager::sharedInstance()->isContextActive())
		guiManager = GameDialogManager::sharedInstance();
	else
		guiManager = MenuManager::sharedInstance();

	hkvVec2 screenSize = guiManager->getScreenSize();
	float scale = guiManager->getScale();

	ScreenMaskPanel::init(hkvVec2(0,0));

	//text label
	vector<string> splittedString;
	StringsXmlHelper::sharedInstance().splitStringInArray(splittedString, text, 30);
	int substringsCount = (int) splittedString.size();
	hkvVec2 popupMessagePosition = hkvVec2(0, -100.0f * scale);
	for (int substringIndex = 0; substringIndex != substringsCount; substringIndex++)
	{
		PrintTextLabel* textLabel = new PrintTextLabel();
		textLabel->AddRef();
		float fontHeight = 38.0f * scale;
		VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
		VColorRef fontColor = V_RGBA_WHITE;

		float relativeIndex;
		int halfCount = substringsCount / 2;
		if (substringsCount % 2 == 0) //even
			relativeIndex = (substringIndex < halfCount) ? substringIndex - halfCount + 0.5f : substringIndex - halfCount + 1 - 0.5f;
		else //odd
			relativeIndex = (float) (substringIndex - halfCount);
		hkvVec2 splittedStringPosition = hkvVec2(popupMessagePosition.x, popupMessagePosition.y + relativeIndex * 30.0f * scale);
		textLabel->init(splittedStringPosition, splittedString[substringIndex], neuropolFont, fontHeight, fontColor);

		this->addTextLabel(textLabel);
	}	

	//buttons
	YesNoButton* yesBtn = new YesNoButton();
	yesBtn->AddRef();
	yesBtn->init(this, VGUIManager::GetID(ID_YES_BTN));

	YesNoButton* noBtn = new YesNoButton();
	noBtn->AddRef();
	noBtn->init(this, VGUIManager::GetID(ID_NO_BTN));

	this->SetOrder(POPUP_ORDER);
}

//////////////////////////PopupButton//////////////////////////
void YesNoButton::init(YesNoDialogPanel* parentDialogPanel, int id)
{
	this->parentDialogPanel = parentDialogPanel;
	this->m_iID = id;
	
	float scale;
	if (GameDialogManager::sharedInstance()->isContextActive())
		scale = GameDialogManager::sharedInstance()->getScale();
	else if (MenuManager::sharedInstance()->isContextActive())
		scale = MenuManager::sharedInstance()->getScale();

	if (id == VGUIManager::GetID(ID_YES_BTN) || id == VGUIManager::GetID(ID_NO_BTN))
	{
		VTextureObject* popupBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\blue_circle_button.png");
		float popupBtnWidth = 128.0f * scale;
		float popupBtnHeight = 128.0f * scale;
		hkvVec2 popupBtnSize = hkvVec2(popupBtnWidth, popupBtnHeight);
		hkvVec2 popupBtnPosition;
		if (id == VGUIManager::GetID(ID_YES_BTN))
			popupBtnPosition = hkvVec2(-150.0f * scale, 60.0f * scale);
		else 
			popupBtnPosition = hkvVec2(150.0f * scale, 60.0f * scale);
		ButtonScreenMask::init(popupBtnPosition, popupBtnSize, popupBtnTexture);
		parentDialogPanel->addScreenMask(this);
		this->SetOrder(POPUP_ORDER - 1);

		PrintTextLabel* textLabel = new PrintTextLabel();
		textLabel->AddRef();
		string text = StringsXmlHelper::sharedInstance().getStringForTag((id == VGUIManager::GetID(ID_YES_BTN)) ? "yes" : "no");
		float fontHeight = 40 * scale;
		VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
		VColorRef fontColor = V_RGBA_WHITE;
		hkvVec2 textLabelPosition = popupBtnPosition + hkvVec2(0, 75.0f *scale);
		textLabel->init(textLabelPosition, text, neuropolFont, fontHeight, fontColor);
		parentDialogPanel->addTextLabel(textLabel);
	}
}

void YesNoButton::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_YES_BTN))
		parentDialogPanel->onClickYes();
	else if (this->m_iID == VGUIManager::GetID(ID_NO_BTN))
		parentDialogPanel->onClickNo();
}

//////////////////////Slider//////////////////////
V_IMPLEMENT_DYNCREATE(Slider, ScreenMaskPanel, Vision::GetEngineModule());

void Slider::init(bool horizontal, hkvVec2 position, hkvVec2 trackSize, hkvVec2 knobSize, VTextureObject* trackTexture, VTextureObject* knobTexture)
{
	this->horizontal = horizontal;
	ScreenMaskPanel::init(position);
	this->trackSize = trackSize;
	this->knobSize = knobSize;

	float scale = GameDialogManager::sharedInstance()->getScale();

	//slider bg
	sliderTrack = new SpriteScreenMask();
	sliderTrack->AddRef();
	sliderTrack->init(hkvVec2(0,0), trackSize, trackTexture);
	this->addScreenMask(sliderTrack);

	//slider knob
	sliderKnob = new SpriteScreenMask();
	sliderKnob->AddRef();
	sliderKnob->init(hkvVec2(0,0), knobSize, knobTexture);
	this->addScreenMask(sliderKnob);
}

bool Slider::containsTouch(hkvVec2 touchLocation)
{
	return sliderTrack->containsTouch(touchLocation) || sliderKnob->containsTouch(touchLocation);
}

void Slider::onPointerDown(hkvVec2 touchLocation)
{
	Touchable::onPointerDown(touchLocation);

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 knobAbsPosition = sliderKnob->GetAbsPosition();
	hkvVec2 knobTouchArea = hkvVec2(1.5f * knobSize.x, 1.5f * knobSize.y); //area is 1.5x bigger than the actual size of the knob
	hkvVec2 bboxMin = knobAbsPosition - anchorPoint.compMul(knobTouchArea);
	hkvVec2 bboxMax = knobAbsPosition + anchorPoint.compMul(knobTouchArea);
	VRectanglef bbox = VRectanglef(bboxMin, bboxMax);

	if (bbox.IsInside(touchLocation))
		this->knobSelected = true;
}

bool Slider::onPointerMove(hkvVec2 touchLocation)
{
	if (!Touchable::onPointerMove(touchLocation))
		return false;

	if (!this->knobSelected)
		return false;

	float scale = GameDialogManager::sharedInstance()->getScale();

	//find min and max values the cursor can reach
	hkvVec2 knobMinPosition, knobMaxPosition;
	if (horizontal)
	{
		knobMinPosition = this->sliderTrack->GetPosition() - hkvVec2(0.5f * trackSize.x, 0);
		knobMaxPosition = this->sliderTrack->GetPosition() + hkvVec2(0.5f * trackSize.x, 0);
	}
	else
	{
		knobMinPosition = this->sliderTrack->GetPosition() - hkvVec2(0, 0.5f * trackSize.y);
		knobMaxPosition = this->sliderTrack->GetPosition() + hkvVec2(0, 0.5f * trackSize.y);
	}

	hkvVec2 deltaTouchLocation = horizontal ? hkvVec2(this->deltaTouchLocation.x, 0) : hkvVec2(this->deltaTouchLocation.y);
	hkvVec2 knobNewPosition = sliderKnob->GetPosition() + deltaTouchLocation;
	if (horizontal)
	{
		if (knobNewPosition.x < knobMinPosition.x)
			knobNewPosition.x = knobMinPosition.x;
		else if (knobNewPosition.x > knobMaxPosition.x)
			knobNewPosition.x = knobMaxPosition.x;
	}
	else
	{
		if (knobNewPosition.y < knobMinPosition.y)
			knobNewPosition.y = knobMinPosition.y;
		else if (knobNewPosition.y > knobMaxPosition.y)
			knobNewPosition.y = knobMaxPosition.y;
	}

	sliderKnob->SetPosition(knobNewPosition);

	if (horizontal)
		percentage = (knobNewPosition.x - knobMinPosition.x) / (float) (knobMaxPosition.x - knobMinPosition.x);
	else
		percentage = (knobNewPosition.y - knobMinPosition.y) / (float) (knobMaxPosition.y - knobMinPosition.y);

	return true;
}

void Slider::onPointerUp(hkvVec2 touchLocation)
{
	Touchable::onPointerUp(touchLocation);
	this->knobSelected = false;
}

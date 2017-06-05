#include "OrbsPluginPCH.h"
#include "HaloOverlays.h"
#include "GameManager.h"

/////////////////////////////////////////////////SelectionHalo/////////////////////////////////////////////////
void SelectionHalo::init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale, float angle)
{
	RotatingSpriteScreenMask::init(position, unscaledTargetSize, textureObj, SELECTION_HALO_ANGULAR_SPEED, scale, angle);

	//arrows
	buildArrows();
}

void SelectionHalo::buildArrows()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	float sizeX, sizeY;
	this->GetTargetSize(sizeX, sizeY);
	hkvVec2 size = hkvVec2(sizeX, sizeY);

	float arrowWidth = 32.0f * scale;
	float arrowHeight = 32.0f * scale;
	hkvVec2 arrowSize = hkvVec2(arrowWidth, arrowHeight);

	fixedArrow1 = new HaloArrow();
	fixedArrow1->AddRef();
	fixedArrow1->init(this, 0.5f * (size.x - arrowSize.x), -135);

	fixedArrow2 = new HaloArrow();
	fixedArrow2->AddRef();
	fixedArrow2->init(this, 0.5f * size.x, -135);

	mobileArrow = new HaloArrow();
	mobileArrow->AddRef();
	mobileArrow->init(this, 0.5f * (size.x - arrowSize.x), -135);
}

void SelectionHalo::updateForActionMode(int actionMode)
{
	VColorRef tintColor = V_RGBA_WHITE;
	float mobileArrowAngle = 0.0f;
	if (actionMode == ACTION_NORMAL_MODE)
	{
		if (vdynamic_cast<SacrificePit*>(GameManager::GlobalManager().getPickedNodeSphere()))
			tintColor = SACRIFICE_MODE_COLOR;
		//else if (vdynamic_cast<Artifact*>(parentNodeSphere) //TODO Artifact
		//	;
		else
			tintColor = NORMAL_MODE_COLOR;

		mobileArrowAngle = -135;
	}
	else if (actionMode == ACTION_TRAVEL_MODE)
	{
		tintColor = TRAVEL_MODE_COLOR;
		mobileArrowAngle = -90;
	}
	else if (actionMode == ACTION_BUILD_BRIDGE_MODE)
	{
		tintColor = BUILD_MODE_COLOR;
		mobileArrowAngle = 150;
	}
	else if (actionMode == ACTION_RALLY_POINT_MODE)
	{
		tintColor = RALLY_POINT_MODE_COLOR;
		mobileArrowAngle = 30;;
	}
	else if (actionMode == ACTION_SACRIFICE_MODE)
	{
		tintColor = SACRIFICE_MODE_COLOR;
		mobileArrowAngle = 135;;
	}

	this->updateMobileArrowAngle(mobileArrowAngle);
	this->SetColor(tintColor);
}

void SelectionHalo::updateMobileArrowAngle(float newAngle)
{
	this->mobileArrow->moveToAngle(newAngle);
}

void SelectionHalo::updateSize()
{
	//float scaleForCameraZoom = getScaleForCameraZoom();
	//this->SetScale(hkvVec2(scaleForCameraZoom, scaleForCameraZoom));

	////update arrows position
	float scale = GameDialogManager::sharedInstance()->getScale();
	//float haloSizeX, haloSizeY;
	//this->GetTargetSize(haloSizeX, haloSizeY);

	float sphereProjectedRadius = GameManager::GlobalManager().getPickedNodeSphere()->calculateProjectedRadius();
	sphereProjectedRadius *= 3.0f;
	if (sphereProjectedRadius < 120.0f)
		sphereProjectedRadius = 120.0f;
	float haloSizeX = 1.1f * sphereProjectedRadius;
	float haloSizeY = 1.1f * sphereProjectedRadius;
	this->SetUnscaledTargetSize(hkvVec2(haloSizeX, haloSizeY));

	float arrowWidth = 32.0f * scale;
	this->fixedArrow1->setDistanceFromHaloCenter(0.44f * haloSizeX + 0.5f * arrowWidth);
	this->fixedArrow2->setDistanceFromHaloCenter(0.44f * haloSizeX);
	this->mobileArrow->setDistanceFromHaloCenter(0.44f * haloSizeX);
}

//float SelectionHalo::getScaleForCameraZoom()
//{
//	float scale = GameDialogManager::sharedInstance()->getScale();
//	float zoomPercentage = GameManager::GlobalManager().getMainCamera()->getZoomPercentage();
//	int parentNodeSphereMaxLife = GameManager::GlobalManager().getPickedNodeSphere()->getMaxLife();
//
//	float sphereBaseRadius = parentNodeSphereMaxLife / 10.0f;
//	float haloMaxZoomRadius;
//	if (sphereBaseRadius >= 100.0f)
//		 haloMaxZoomRadius = 2.5f * sphereBaseRadius - 0.5f * (sphereBaseRadius - 100.0f);
//	else
//		haloMaxZoomRadius = 2.5f * sphereBaseRadius;
//	float haloMinZoomRadius;
//	if (sphereBaseRadius >= 100.0f)
//		haloMinZoomRadius = 1.5f * sphereBaseRadius - 0.5f * (sphereBaseRadius - 100.0f);
//	else
//		haloMinZoomRadius = 1.5f * sphereBaseRadius;
//
//	float haloRadius = haloMinZoomRadius + (1 - zoomPercentage) * (haloMaxZoomRadius - haloMinZoomRadius);
//
//	return haloRadius / 128.0f;
//}

void SelectionHalo::SetColor(VColorRef iColor)
{
	SpriteScreenMask::SetColor(iColor); //no fade animation, e.g no need to recalculate color alpha

	//update arrows color as well
	hkvVec4 iColorVec4 = iColor.getAsVec4();
	iColorVec4.w = fixedArrow1->GetColor().getAsVec4().w;
	VColorRef fixedArrow1Color = VColorRef::Float_To_RGBA(iColorVec4);
	this->fixedArrow1->SetColor(fixedArrow1Color);
	iColorVec4.w = fixedArrow2->GetColor().getAsVec4().w;
	VColorRef fixedArrow2Color = VColorRef::Float_To_RGBA(iColorVec4);
	this->fixedArrow2->SetColor(fixedArrow2Color);
	iColorVec4.w = mobileArrow->GetColor().getAsVec4().w;
	VColorRef mobileArrowColor = VColorRef::Float_To_RGBA(iColorVec4);
	this->mobileArrow->SetColor(mobileArrowColor);
}

/////////////////////////////////////////////////HaloArrow/////////////////////////////////////////////////
void HaloArrow::init(SelectionHalo* parentHalo, float distanceFromHaloCenter, float angle)
{
	VTextureObject* arrowTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\halo_arrow.png");

	this->parentHalo = parentHalo;
	this->distanceFromHaloCenter = distanceFromHaloCenter;
	this->angle = angle;

	float scale = GameDialogManager::sharedInstance()->getScale();
	float arrowWidth = 32.0f * scale;
	float arrowHeight = 32.0f * scale;
	hkvVec2 arrowSize = hkvVec2(arrowWidth, arrowHeight);

	SpriteScreenMask::init(hkvVec2(0,0), arrowSize, arrowTexture, hkvVec2(1.0f, 1.0f), angle);

	updatePositionAndAngle();
}

void HaloArrow::updatePositionAndAngle()
{
	float relativeXCoords = distanceFromHaloCenter * cos(GeometryUtils::degreesToRadians(angle));
	float relativeYCoords = distanceFromHaloCenter * sin(GeometryUtils::degreesToRadians(angle));

	this->SetPosition(hkvVec2(relativeXCoords, relativeYCoords));

	this->SetAngle(-90 - angle);
}

void HaloArrow::moveToAngle(float targetAngle)
{
	this->moving = true;
	this->startAngle = this->angle;
	this->targetAngle = targetAngle;
	if (startAngle == targetAngle)
	{
		this->moving = false;
		return;
	}
	//angle are supposed to be normalized between 0 and 360, no need to normalize them again
	float angleVariation = targetAngle - startAngle;
	if (abs(angleVariation) > 180)
		angleVariation = (targetAngle > startAngle) ? -(startAngle + 360 - targetAngle) : targetAngle + 360 - startAngle;
	this->angleTotalVariation = angleVariation;
	this->moveDuration = DEFAULT_ARROW_MOVE_DURATION;
	this->moveElapsedTime = 0;
}

float HaloArrow::angleLinearVariation(float dt)
{
	return (dt / moveDuration * angleTotalVariation);
}


void HaloArrow::update(float dt)
{
	SpriteScreenMask::update(dt);
	if (this->moving)
	{
		float newElapsedTime = this->moveElapsedTime + dt;
		if (newElapsedTime > moveDuration)
		{
			dt = moveDuration - this->moveElapsedTime;
			this->moving = false;
		}
		else 
			this->moveElapsedTime = newElapsedTime;
		this->angle += angleLinearVariation(dt);
	}
	updatePositionAndAngle();
}

/////////////////////////////////////////////////EndpointHalo/////////////////////////////////////////////////
void EndpointHalo::init(hkvVec2 position, hkvVec2 size, VColorRef tintColor, hkvVec2 scale, float angle)
{
	VTextureObject* haloTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\endpoint.png");
	SpriteScreenMask::init(position, size, haloTexture, scale, angle);
	this->SetColor(tintColor);
}

/////////////////////////////////////////////////Path/////////////////////////////////////////////////
Path::Path() : fadingOut(false), 
	fadeElapsedTime(0),
	fadeDuration(0), 
	fadeVariation(0), 
	color(V_RGBA_WHITE),
	opacity(0)
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

Path::~Path()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void Path::init(vector<NodeSphere*> &spheresPath, VColorRef color)
{
	this->spheresPath = spheresPath;
	buildSpheresPath(spheresPath);
	this->SetColor(color);
	this->SetOpacity(1.0f);
}

void Path::init(NodeSphere* startSphere, hkvVec2 endPoint, VColorRef color)
{
	buildSingleLinePath(startSphere, endPoint, false, true);
	this->spheresPath.push_back(startSphere);
	this->SetColor(color);
	this->SetOpacity(1.0f);
}

void Path::buildSingleLinePath(NodeSphere* startSphere, hkvVec2 endPoint, bool drawHalos, bool drawFirstPoint)
{
	float screenProjectionX, screenProjectionY;
	VisRenderContext_cl::GetCurrentContext()->Project2D(startSphere->GetPosition(), screenProjectionX, screenProjectionY);
	hkvVec2 startPoint = hkvVec2(screenProjectionX, screenProjectionY);

	//build link
	VTextureObject* pathLinkTexture = Vision::TextureManager.Load2DTexture("Textures\\path_texture.png");

	SpriteScreenMask* link = new SpriteScreenMask();
	link->AddRef();
	hkvVec2 diff = endPoint - startPoint;
	hkvVec2 linkPosition = 0.5f * (startPoint + endPoint);
	hkvVec2 linkSize = hkvVec2(diff.getLength(), PATH_LINK_THICKNESS);
	float linkAngle = atan2(diff.y, diff.x);
	link->init(linkPosition, linkSize, pathLinkTexture, hkvVec2(1.0f, 1.0f), GeometryUtils::radiansToDegrees(-linkAngle));
	link->SetOrder(SELECTION_HALO_PANEL_ORDER);
	links.push_back(link);

	//build points
	VTextureObject* pathPointTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\path_point.png");

	float scale = GameDialogManager::sharedInstance()->getScale();

	if (drawFirstPoint)
	{
		//start one
		SpriteScreenMask* startPathPoint = new SpriteScreenMask();
		startPathPoint->AddRef();
		hkvVec2 startPathPointSize = hkvVec2(64.0f * scale, 64.0f * scale);
		startPathPoint->init(startPoint, startPathPointSize, pathPointTexture);
		startPathPoint->SetOrder(SELECTION_HALO_PANEL_ORDER);
		points.push_back(startPathPoint);
	}

	//end one
	SpriteScreenMask* endPathPoint = new SpriteScreenMask();
	endPathPoint->AddRef();
	hkvVec2 endPathPointSize = hkvVec2(64.0f * scale, 64.0f * scale);
	endPathPoint->init(endPoint, endPathPointSize, pathPointTexture);
	endPathPoint->SetOrder(SELECTION_HALO_PANEL_ORDER);
	points.push_back(endPathPoint);

	//build halos
	if (drawHalos)
	{
		VTextureObject* pathHaloTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\path_halo.png");

		//end one
		SpriteScreenMask* endPathHalo = new SpriteScreenMask();
		endPathHalo->AddRef();
		hkvVec2 endPathHaloSize = hkvVec2(128.0f * scale, 128.0f * scale);
		endPathHalo->init(endPoint, endPathHaloSize, pathHaloTexture);
		endPathHalo->SetOrder(SELECTION_HALO_PANEL_ORDER);
		halos.push_back(endPathHalo);
	}
}

void Path::buildSpheresPath(vector<NodeSphere*> &spheresPath)
{
	int numberOfSpheres = (int) spheresPath.size();
	links.reserve(numberOfSpheres); //n links
	points.reserve(numberOfSpheres + 1); //n+1 points
	halos.reserve(numberOfSpheres); //n halos (first is selection halo and it is already drawn)

	for (int sphereIndex = 0; sphereIndex != numberOfSpheres - 1; sphereIndex++)
	{
		SpriteScreenMask* link = new SpriteScreenMask();
		link->AddRef();
		float screenProjectionX, screenProjectionY;
		NodeSphere* startSphere = spheresPath[sphereIndex];
		NodeSphere* endSphere = spheresPath[sphereIndex + 1];
		VisRenderContext_cl::GetCurrentContext()->Project2D(endSphere->GetPosition(), screenProjectionX, screenProjectionY);
		hkvVec2 endPoint = hkvVec2(screenProjectionX, screenProjectionY);

		buildSingleLinePath(startSphere, endPoint, true, sphereIndex == 0);
	}
}

void Path::updateSingleLinePath(int lineNumber, hkvVec2 newEndPointPosition)
{
	SpriteScreenMask* startPoint = this->points[lineNumber];
	SpriteScreenMask* endPoint = this->points[lineNumber + 1];
	int linksSize = (int) this->links.size();
	SpriteScreenMask* link = this->links[lineNumber];
	if ((int) halos.size() > lineNumber)
	{
		SpriteScreenMask* halo = this->halos[lineNumber];
		halo->SetPosition(newEndPointPosition);
	}

	endPoint->SetPosition(newEndPointPosition);
	hkvVec2 startPointPosition = startPoint->GetPosition();
	hkvVec2 diff = newEndPointPosition - startPointPosition;
	hkvVec2 linkPosition = 0.5f * (startPointPosition + newEndPointPosition);
	hkvVec2 linkSize = hkvVec2(diff.getLength(), PATH_LINK_THICKNESS);
	float linkAngle = atan2(diff.y, diff.x);

	link->SetUnscaledTargetSize(linkSize);
	link->SetPosition(linkPosition);
	link->SetAngle(GeometryUtils::radiansToDegrees(-linkAngle));
}

void Path::updateSpheresPath()
{
	int numberOfSpheres = (int) spheresPath.size();

	if (numberOfSpheres > 1)
	{
		for (int sphereIndex = 0; sphereIndex != numberOfSpheres - 1; sphereIndex++)
		{
			float screenProjectionX, screenProjectionY;
			NodeSphere* startSphere = spheresPath[sphereIndex];
			NodeSphere* endSphere = spheresPath[sphereIndex + 1];
			VisRenderContext_cl::GetCurrentContext()->Project2D(endSphere->GetPosition(), screenProjectionX, screenProjectionY);
			hkvVec2 endPointPosition = hkvVec2(screenProjectionX, screenProjectionY);

			if (sphereIndex == 0) //update the position of the first point and thus the first link
			{
				VisRenderContext_cl::GetCurrentContext()->Project2D(startSphere->GetPosition(), screenProjectionX, screenProjectionY);
				hkvVec2 startPointPosition = hkvVec2(screenProjectionX, screenProjectionY);
				
				SpriteScreenMask* firstLink = this->links[0]; //first link
				SpriteScreenMask* firstPoint = this->points[0];

				firstPoint->SetPosition(startPointPosition);
				hkvVec2 diff = endPointPosition - startPointPosition;
				hkvVec2 linkPosition = 0.5f * (startPointPosition + endPointPosition);
				hkvVec2 linkSize = hkvVec2(diff.getLength(), PATH_LINK_THICKNESS);
				float linkAngle = atan2(diff.y, diff.x);

				firstLink->SetUnscaledTargetSize(linkSize);
				firstLink->SetPosition(linkPosition);
				firstLink->SetAngle(GeometryUtils::radiansToDegrees(-linkAngle));
			}

			updateSingleLinePath(sphereIndex, endPointPosition);
		}
	}
	else
	{
		NodeSphere* startSphere = spheresPath[0];
		float screenProjectionX, screenProjectionY;
		VisRenderContext_cl::GetCurrentContext()->Project2D(startSphere->GetPosition(), screenProjectionX, screenProjectionY);
		hkvVec2 startPointPosition = hkvVec2(screenProjectionX, screenProjectionY);

		SpriteScreenMask* startPointHalo = this->points[0];
		SpriteScreenMask* endPointHalo = this->points[1];
		startPointHalo->SetPosition(startPointPosition);
		updateSingleLinePath(0, endPointHalo->GetPosition());
	}
}

void Path::removeSelf()
{
	this->removeLinks();
	this->removePoints();
	this->removeHalos();
	this->Release();
}

void Path::removeLinks()
{
	for (int linkIndex = 0; linkIndex != links.size(); linkIndex++)
	{
		links[linkIndex]->Release();
	}
}

void Path::removePoints()
{
	for (int pointIndex = 0; pointIndex != points.size(); pointIndex++)
	{
		points[pointIndex]->Release();
	}
}

void Path::removeHalos()
{
	for (int haloIndex = 0; haloIndex != halos.size(); haloIndex++)
	{
		halos[haloIndex]->Release();
	}
}

void Path::fadeOut(float fadeDuration)
{
	this->fadingOut = true;
	this->fadeDuration = 0.5f;
	this->fadeVariation = -1.0f;
}

void Path::SetOpacity(float opacity)
{
	this->opacity = opacity;
	hkvVec4 floatColor = this->GetColor().getAsVec4();
	floatColor.w = opacity; //sets the new alpha

	this->SetColor(VColorRef::Float_To_RGBA(floatColor));
}

void Path::SetColor(VColorRef color)
{
	this->color = color;

	for (int linkIndex = 0; linkIndex != links.size(); linkIndex++)
	{
		links[linkIndex]->SetColor(color);
	}

	for (int pointIndex = 0; pointIndex != points.size(); pointIndex++)
	{
		points[pointIndex]->SetColor(color);
	}

	for (int haloIndex = 0; haloIndex != halos.size(); haloIndex++)
	{
		halos[haloIndex]->SetColor(color);
	}
}

void Path::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference();
		update(dt);
	}
}

void Path::update(float dt)
{
	if (fadingOut)
	{
		fadeElapsedTime += dt;
		if (fadeElapsedTime > fadeDuration)
		{
			this->SetOpacity(0);
			this->fadingOut = false;
			this->Release();
		}
		else
		{
			float fadeLinearVariation = dt / fadeDuration * fadeVariation;
			this->SetOpacity(opacity + fadeLinearVariation);
		}
	}
}

//Path::Path() : fadingOut(false), 
//	fadeElapsedTime(0),
//	fadeDuration(0), 
//	fadeVariation(0), 
//	color(V_RGBA_WHITE),
//	opacity(0)
//{
//	Vision::Callbacks.OnUpdateSceneBegin += this;
//}
//
//Path::~Path()
//{
//	Vision::Callbacks.OnUpdateSceneBegin -= this;
//}
//
//void Path::init(vector<NodeSphere*> &spheresPath, VColorRef color)
//{
//	buildLinks(spheresPath);
//	buildHalos(spheresPath);
//	this->color = color;
//	this->opacity = 1.0f;
//}
//
//void Path::init(hkvVec3 startPoint, hkvVec3 endPoint, VColorRef color)
//{
//	this->color = color;
//	this->opacity = 1.0f;
//}
//
//void Path::buildSingleLinePath(hkvVec3 startPoint, hkvVec3 endPoint)
//{
//	links.reserve(1);
//	halos.reserve(1);
//
//	//build link
//	Bridge* link = new Bridge();
//	link->AddRef();
//	link->init(startPoint, endPoint);
//	links.push_back(link);
//
//	//build halo
//	float scale = GameDialogManager::sharedInstance()->getScale();
//	BatchSprite* halo = new BatchSprite();
//	halo->AddRef();
//	hkvVec2 haloSize = hkvVec2(40.0f * scale, 40.0f * scale);
//	halo->init(haloPosition, haloSize);
//	halos.push_back(halo);
//}
//
//void Path::buildLinks(vector<NodeSphere*> &spheresPath)
//{
//	int numberOfSpheres = (int) spheresPath.size();
//	links.reserve(numberOfSpheres); //n links
//
//	for (int sphereIndex = 0; sphereIndex != numberOfSpheres - 1; sphereIndex++)
//	{
//		Bridge* link = new Bridge();
//		link->AddRef();
//		NodeSphere* startSphere = spheresPath[sphereIndex];
//		NodeSphere* endSphere = spheresPath[sphereIndex + 1];
//		link->init(startSphere, endSphere);
//		links.push_back(link);
//	}
//}
//
//void Path::buildHalos(vector<NodeSphere*> &spheresPath)
//{
//	int numberOfSpheres = (int) spheresPath.size();
//	halos.reserve(numberOfSpheres); //n halos (first is selection halo and it is already drawn)
//
//	for (int sphereIndex = 1; sphereIndex != numberOfSpheres; sphereIndex++)
//	{
//		BatchSprite* halo = new BatchSprite();
//		halo->AddRef();
//		NodeSphere* sphere = spheresPath[sphereIndex];
//		hkvVec3 haloPosition = sphere->GetPosition();
//		hkvVec2 haloSize = hkvVec2(2 * 1.2f * sphere->getInnerRadius(), 2 * 1.2f * sphere->getInnerRadius());
//		halo->init(haloPosition, haloSize);
//		halos.push_back(halo);
//	}
//}
//
//void Path::removeSelf()
//{
//	this->removeLinks();
//	this->removeHalos();
//	this->Release();
//}
//
//void Path::removeLinks()
//{
//	for (int linkIndex = 0; linkIndex != links.size(); linkIndex++)
//	{
//		links[linkIndex]->Release();
//	}
//}
//
//void Path::removeHalos()
//{
//	for (int haloIndex = 0; haloIndex != halos.size(); haloIndex++)
//	{
//		halos[haloIndex]->Release();
//	}
//}
//
//void Path::fadeOut(float fadeDuration)
//{
//	this->fadingOut = true;
//	this->fadeDuration = 0.5f;
//	this->fadeVariation = -1.0f;
//}
//
//void Path::SetOpacity(float opacity)
//{
//	this->opacity = opacity;
//	hkvVec4 floatColor = this->GetColor().getAsVec4();
//	floatColor.w = opacity; //sets the new alpha
//
//	this->color = VColorRef::Float_To_RGBA(floatColor);
//}
//
//void Path::OnHandleCallback(IVisCallbackDataObject_cl *pData)
//{
//	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
//	{
//		float dt = Vision::GetTimer()->GetTimeDifference();
//		update(dt);
//	}
//}
//
//void Path::update(float dt)
//{
//	if (fadingOut)
//	{
//		fadeElapsedTime += dt;
//		if (fadeElapsedTime > fadeDuration)
//		{
//			this->SetOpacity(0);
//			this->fadingOut = false;
//			GameManager::GlobalManager().removePath(this);
//		}
//		else
//		{
//			float fadeLinearVariation = dt / fadeDuration * fadeVariation;
//			this->SetOpacity(opacity + fadeLinearVariation);
//		}
//	}
//}


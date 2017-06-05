#include "OrbsPluginPCH.h"
#include "GameDialogManager.h"
#include "GameManager.h"

GameDialogManager* GameDialogManager::instance = NULL;

GameDialogManager::GameDialogManager(void) : 
gameHUD(NULL),
onPressProcessedByGUI(false)
{

}

GameDialogManager::~GameDialogManager(void)
{

}

GameDialogManager* GameDialogManager::sharedInstance()
{
	if (instance == NULL)
		instance = new GameDialogManager();
	return instance;
}

void GameDialogManager::initGUIContext()
{
	SceneGUIManager::initGUIContext();
	MessageHandler::sharedInstance().OneTimeInit();
	Vision::Callbacks.OnRenderHook += this;
}

void GameDialogManager::deInitGUIContext()
{
	MessageHandler::sharedInstance().OneTimeDeInit();

	SceneGUIManager::deInitGUIContext();
	Vision::Callbacks.OnRenderHook -= this;

	if (gameHUD)
		removeHUD();
	m_GUIContext->SetActivate(false);

	V_SAFE_DELETE(instance);
}


void GameDialogManager::defineScale()
{
	hkvVec2 screenSize = getScreenSize();

	float designScreenWidth = 2048.0f; //width as it was designed for an ipad4 retina screen
	float designScreenHeight = 1536.0f; //height as it was designed for an ipad4 retina screen
	float scaleX = screenSize.x / designScreenWidth;
	float scaleY = screenSize.y / designScreenHeight;
	scale = max(scaleX, scaleY); //take the smaller scale
}

vector<PrintTextLabel*> GameDialogManager::retrieveAllTextLabels()
{
	return gameHUD->retrieveAllTextLabels();
}

void GameDialogManager::showHUD()
{
	gameHUD = new GameHUD();
	gameHUD->init();
}

void GameDialogManager::removeHUD()
{
	gameHUD = NULL;
}

void GameDialogManager::showDefeatVictoryMenu(bool victory, float delay)
{
	endMenu = new DefeatVictoryMenu();
	endMenu->init(victory);

	CallFuncObject* showEndMenuAction = new CallFuncObject(endMenu, (CallFunc)(&DefeatVictoryMenu::show), delay);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showEndMenuAction);
	showEndMenuAction->executeFunc();
}

void GameDialogManager::removeDefeatVictoryMenu()
{
	endMenu->removeSelf();
}

bool GameDialogManager::clickContainedInGUI(hkvVec2 touchLocation)
{
	if (endMenu)
		return true;
	GameHUD* hud = GameDialogManager::sharedInstance()->getHUD();
	if (hud->getPauseMenu() || hud->getObjectivesMenu() || hud->getHelpMenu())
		return true;
	return gameHUD->containsClick(touchLocation);
}

bool GameDialogManager::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	if (endMenu) //click is swallowed by the defeat/victory menu
		return endMenu->processPointerEvent(touchLocation, eventType);

	return gameHUD->processPointerEvent(touchLocation, eventType);
}

void GameDialogManager::update(float dt)
{
	if (gameHUD)
		gameHUD->update(dt);
}

void GameDialogManager::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	SceneGUIManager::OnHandleCallback(pData);
	if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
		if (pRHDO->m_iEntryConst == VRH_PRE_SCREENMASKS)
		{		
			if (gameHUD)
			{
				gameHUD->updateSelectionHaloPanelPosition();
				gameHUD->updateRallySphereHaloPosition();
				gameHUD->updateBuildSphereHaloPosition();
				gameHUD->updatePathsPositions();
			}
		}
	}
}

//////////////////////////////GameHUD/////////////////////////////////////////
GameHUD::~GameHUD()
{
	infoPanel = NULL;
	corePanel = NULL;
	actionPanel = NULL;
	interfacePanel = NULL;
	miscPanel = NULL;
	timerPanel = NULL;
	objectivesMenu = NULL;
	helpMenu = NULL;
	pauseMenu = NULL;
	achievementUnlockedPanel = NULL;

	this->removeActiveTravelPath();
	this->removeActiveRallyPath();
	this->removeRallySphereHalo();
	this->removeBuildSphereHalo();
	this->removePotentialBridges();
}

void GameHUD::init()
{
	//disable panels (no node sphere selected at start)
	this->corePanelDismissed = true;
	this->infoPanelDismissed = true;

	this->buildInfoPanel();
	this->buildActionPanel();
	this->buildInterfacePanel();
	this->buildMiscPanel();
	//this->buildTimerPanel();
}


void GameHUD::buildCorePanel()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	if (corePanel)
		corePanel = NULL; //destroy the old one
	corePanel = new CorePanel();
	hkvVec2 corePanelPosition;
	corePanelPosition = hkvVec2(0.5f * screenSize.x, this->corePanelDismissed ? screenSize.y + 200.0f * scale : screenSize.y - 128.0f * scale);
	corePanel->init(corePanelPosition);

	corePanel->buildBackground();
	corePanel->buildAndShowMainContent();	
}

void GameHUD::buildInfoPanel()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	infoPanel = new ScreenMaskPanel();
	 
	hkvVec2 infoPanelPosition = hkvVec2(this->infoPanelDismissed ? -256.0f * scale : 256.0f * scale, screenSize.y - 200.0f *scale);
	infoPanel->init(infoPanelPosition);

	//load all textures used in this panel
	VTextureObject* bgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\info_panel\\info_panel_bg_texture.png");

	//Background
	SpriteScreenMask* infoPanelBg = new SpriteScreenMask();
	infoPanelBg->AddRef();

	hkvVec2 infoPanelBgSize = hkvVec2(512.0f, 256.0f);

	hkvVec2 infoPanelBgPosition = hkvVec2(0, 0);
	infoPanelBg->init(infoPanelBgPosition, infoPanelBgSize, bgTexture);

	infoPanel->addScreenMask(infoPanelBg);
	infoPanel->SetOrder(INFO_PANEL_ORDER);
	infoPanelBg->SetScale(hkvVec2(scale, scale));

	//text labels
	PrintTextLabel* alliedOrbsLabel = new PrintTextLabel();
	PrintTextLabel* enemyOrbsLabel = new PrintTextLabel();
	//PrintTextLabel* sphereLifeLabel = new PrintTextLabel();
	PrintTextLabel* alliedOrbsCountLabel = new PrintTextLabel();
	PrintTextLabel* enemyOrbsCountLabel = new PrintTextLabel();
	//PrintTextLabel* sphereLifeCountLabel = new PrintTextLabel();
	alliedOrbsLabel->AddRef();
	enemyOrbsLabel->AddRef();
	//sphereLifeLabel->AddRef();
	alliedOrbsCountLabel->AddRef();
	enemyOrbsCountLabel->AddRef();
	//sphereLifeCountLabel->AddRef();

	float fFontHeight = 38.0f * scale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fFontHeight);

	hkvVec2 alliedOrbsLabelPosition = hkvVec2(-170.0f * scale, -30.0f * scale);
	string alliedOrbsText = StringsXmlHelper::sharedInstance().getStringForTag("orbs");
	alliedOrbsLabel->init(alliedOrbsLabelPosition, alliedOrbsText, pFont, fFontHeight, V_RGBA_WHITE);
	alliedOrbsLabel->setRenderedBehindVeils(true);
	hkvVec2 alliedOrbsCountLabelPosition = hkvVec2(50.0f * scale, -30.0f * scale);
	alliedOrbsCountLabel->init(alliedOrbsCountLabelPosition, "", pFont, fFontHeight, V_RGBA_WHITE);
	alliedOrbsCountLabel->setRenderedBehindVeils(true);

	hkvVec2 enemyOrbsLabelPosition = hkvVec2(-170.0f * scale, 30.0f * scale);
	string enemyOrbsText = StringsXmlHelper::sharedInstance().getStringForTag("enemies");
	enemyOrbsLabel->init(enemyOrbsLabelPosition, enemyOrbsText, pFont, fFontHeight, V_RGBA_WHITE);
	enemyOrbsLabel->setRenderedBehindVeils(true);
	hkvVec2 enemyOrbsCountLabelPosition = hkvVec2(50.0f * scale, 30.0f * scale);
	enemyOrbsCountLabel->init(enemyOrbsCountLabelPosition, "", pFont, fFontHeight, V_RGBA_WHITE);
	enemyOrbsCountLabel->setRenderedBehindVeils(true);

	//hkvVec2 sphereLifeLabelPosition = hkvVec2(-150.0f * scale, 40.0f * scale);
	//string sphereLifeOrbsText = StringsXmlHelper::sharedInstance().getStringForTag("hp");
	//sphereLifeLabel->init(sphereLifeLabelPosition, sphereLifeOrbsText, font, fontHeight, V_RGBA_WHITE);
	//hkvVec2 sphereLifeOrbsCountLabelPosition = hkvVec2(50.0f * scale, 40.0f * scale);
	//sphereLifeCountLabel->init(sphereLifeOrbsCountLabelPosition, "", font, fontHeight, V_RGBA_WHITE);

	alliedOrbsLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	enemyOrbsLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	//sphereLifeLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	alliedOrbsCountLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	enemyOrbsCountLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	//sphereLifeCountLabel->SetAnchorPoint(hkvVec2(0, 0.5f));

	infoPanel->addTextLabel(alliedOrbsLabel);
	infoPanel->addTextLabel(enemyOrbsLabel);
	//infoPanel->addTextLabel(sphereLifeLabel);
	infoPanel->addTextLabel(alliedOrbsCountLabel);
	infoPanel->addTextLabel(enemyOrbsCountLabel);
	//infoPanel->addTextLabel(sphereLifeCountLabel);	

	alliedOrbsCountLabel->m_iID = VGUIManager::GetID(ID_ALLIED_ORBS_COUNT_LABEL);
	enemyOrbsCountLabel->m_iID = VGUIManager::GetID(ID_ENEMY_ORBS_COUNT_LABEL);
	//sphereLifeCountLabel->m_iID = VGUIManager::GetID(ID_SPHERE_LIFE_COUNT_LABEL);
}

void GameHUD::buildActionPanel()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	actionPanel = new ActionPanel();
	//hkvVec2 actionPanelPosition = hkvVec2(this->actionPanelDismissed ? screenSize.x + 90.0f * scale : screenSize.x - 90.0f * scale, 0.5f * screenSize.y);
	hkvVec2 actionPanelPosition = hkvVec2(screenSize.x, 0.5f * screenSize.y);
	actionPanel->init(actionPanelPosition, this->getNodeSphere());
}

void GameHUD::buildMoveSubActionPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	//textures
	VTextureObject* blankTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	float sliderWidth = 64.0f * scale;
	float sliderHeight = 512.0f * scale;
	hkvVec2 sliderSize = hkvVec2(sliderWidth, sliderHeight);

	//position of the move button
	ActionButton* moveBtn = (ActionButton*) actionPanel->findElementById(ID_MOVE_BTN);
	hkvVec2 moveBtnPosition = moveBtn->GetPosition();
	float moveBtnWidth, moveBtnHeight;
	moveBtn->GetTargetSize(moveBtnWidth, moveBtnHeight);

	//position of the move sub-menu panel
	float sliderHorizontalOffset = 200.0f * scale;
	float sliderVerticalOffset = 60.0f * scale;
	float subActionPanelWidth = 0.5f * sliderWidth + sliderHorizontalOffset;
	float subActionPanelHeight = sliderHeight;
	hkvVec2 sliderPosition = hkvVec2(-0.5f * subActionPanelWidth + 0.5f * sliderWidth, 0);
	
	//rectangle link
	SpriteScreenMask* greenLink = new SpriteScreenMask();
	greenLink->AddRef();
	hkvVec2 greenLinkSize = hkvVec2(sliderHorizontalOffset - 19.0f * scale, 4.0f * scale); 
	hkvVec2 greenLinkPosition = hkvVec2(0.5f * (subActionPanelWidth - greenLinkSize.x), -225.0f * scale);
	greenLink->init(greenLinkPosition, greenLinkSize, blankTexture);
	greenLink->SetColor(VColorRef(18, 192, 0));
	greenLink->SetOrder(ACTION_PANEL_ORDER + 2);

	//count text label
	PrintTextLabel* movingOrbsCountTextLabel = new PrintTextLabel();
	movingOrbsCountTextLabel->AddRef();
	hkvVec2 movingOrbsCountTextLabelPosition = greenLinkPosition - hkvVec2(0, 20.0f * scale);
	float movingOrbsCountFontHeight = 36.0f * scale;
	VisFont_cl* movingOrbsCountFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, movingOrbsCountFontHeight);
	movingOrbsCountTextLabel->init(movingOrbsCountTextLabelPosition, "", movingOrbsCountFont, movingOrbsCountFontHeight, V_RGBA_WHITE);
	movingOrbsCountTextLabel->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL_ORBS_COUNT);
	
	//slider
	MoveSubMenuSlider* slider = new MoveSubMenuSlider();
	slider->AddRef();
	slider->init(sliderPosition);
	slider->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER);

	//validate and cancel buttons
	hkvVec2 resultBtnSize = hkvVec2(128.0f * scale, 128.0f * scale);
	VTextureObject* validateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_validate_btn.png");
	VTextureObject* cancelBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_undo_btn.png");

	ActionButton* validateBtn = new ActionButton();
	validateBtn->AddRef();
	hkvVec2 validateBtnPosition = hkvVec2(0.5f * (subActionPanelWidth - resultBtnSize.x), -100.0f * scale);
	validateBtn->init(validateBtnPosition, resultBtnSize, validateBtnTexture);
	validateBtn->SetColor(V_RGBA_GREEN);
	validateBtn->setEnabledColor(V_RGBA_GREEN);
	validateBtn->setSelectedColor(V_RGBA_GREEN);
	validateBtn->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER_VALIDATE);

	ActionButton* cancelBtn = new ActionButton();
	cancelBtn->AddRef();
	hkvVec2 cancelBtnPosition = hkvVec2(0.5f * (subActionPanelWidth - resultBtnSize.x), 100.0f * scale);
	cancelBtn->init(cancelBtnPosition, resultBtnSize, cancelBtnTexture);
	cancelBtn->SetColor(V_RGBA_RED);
	cancelBtn->setEnabledColor(V_RGBA_RED);
	cancelBtn->setSelectedColor(V_RGBA_RED);
	cancelBtn->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER_CANCEL);

	//parent holder
	hkvVec2 moveSubActionPanelDismissedPosition = hkvVec2(0.6f * subActionPanelWidth, moveBtnPosition.y + 0.5f * moveBtnHeight + 0.5f * subActionPanelHeight + sliderVerticalOffset);
	hkvVec2 moveSubActionPanelShownPosition = hkvVec2(-0.5f * subActionPanelWidth, moveBtnPosition.y + 0.5f * moveBtnHeight + 0.5f * subActionPanelHeight + sliderVerticalOffset);

	MoveSubMenuPanel* moveSubActionPanel = new MoveSubMenuPanel();
	moveSubActionPanel->AddRef();
	moveSubActionPanel->init(moveSubActionPanelDismissedPosition);
	moveSubActionPanel->setDismissedPosition(moveSubActionPanelDismissedPosition);
	moveSubActionPanel->setShownPosition(moveSubActionPanelShownPosition);
	moveSubActionPanel->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL);
	
	moveSubActionPanel->addScreenMask(greenLink);
	moveSubActionPanel->addTextLabel(movingOrbsCountTextLabel);
	moveSubActionPanel->addPanel(slider);
	moveSubActionPanel->addScreenMask(validateBtn);
	moveSubActionPanel->addScreenMask(cancelBtn);
	actionPanel->addPanel(moveSubActionPanel);
}

void GameHUD::buildRallySubActionPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	//position of the rally button
	ActionButton* rallyBtn = (ActionButton*) actionPanel->findElementById(ID_RALLY_POINT_BTN);
	hkvVec2 rallyBtnPosition = rallyBtn->GetPosition();
	float rallyBtnWidth, rallyBtnHeight;
	rallyBtn->GetTargetSize(rallyBtnWidth, rallyBtnHeight);

	//validate and cancel buttons
	hkvVec2 resultBtnSize = hkvVec2(128.0f * scale, 128.0f * scale);
	VTextureObject* validateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_validate_btn.png");
	VTextureObject* cancelBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_undo_btn.png");
	VTextureObject* deleteBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_delete_btn.png");

	ActionButton* validateBtn = new ActionButton();
	validateBtn->AddRef();
	hkvVec2 validateBtnPosition = hkvVec2(0.5f * (resultBtnSize.x), -200.0f * scale);
	validateBtn->init(validateBtnPosition, resultBtnSize, validateBtnTexture);
	validateBtn->SetColor(V_RGBA_GREEN);
	validateBtn->setEnabledColor(V_RGBA_GREEN);
	validateBtn->setSelectedColor(V_RGBA_GREEN);
	validateBtn->m_iID = VGUIManager::GetID(ID_RALLY_SUB_PANEL_VALIDATE);

	ActionButton* cancelBtn = new ActionButton();
	cancelBtn->AddRef();
	hkvVec2 cancelBtnPosition = hkvVec2(0.5f * resultBtnSize.x, 200.0f * scale);
	cancelBtn->init(cancelBtnPosition, resultBtnSize, cancelBtnTexture);
	cancelBtn->SetColor(V_RGBA_RED);
	cancelBtn->setEnabledColor(V_RGBA_RED);
	cancelBtn->setSelectedColor(V_RGBA_RED);
	cancelBtn->m_iID = VGUIManager::GetID(ID_RALLY_SUB_PANEL_CANCEL);

	ActionButton* deleteBtn = new ActionButton();
	deleteBtn->AddRef();
	hkvVec2 deleteBtnPosition = hkvVec2(0.5f * resultBtnSize.x, -200.0f * scale);
	deleteBtn->init(deleteBtnPosition, resultBtnSize, deleteBtnTexture);
	deleteBtn->SetColor(V_RGBA_RED);
	deleteBtn->setEnabledColor(V_RGBA_RED);
	deleteBtn->setSelectedColor(V_RGBA_RED);
	deleteBtn->m_iID = VGUIManager::GetID(ID_RALLY_SUB_PANEL_DELETE);

	RallySubMenuPanel* rallySubActionPanel = new RallySubMenuPanel();
	rallySubActionPanel->AddRef();
	rallySubActionPanel->init(hkvVec2(0, rallyBtnPosition.y));
	rallySubActionPanel->m_iID = VGUIManager::GetID(ID_RALLY_SUB_PANEL);

	rallySubActionPanel->addScreenMask(validateBtn);
	rallySubActionPanel->addScreenMask(cancelBtn);
	rallySubActionPanel->addScreenMask(deleteBtn);
	actionPanel->addPanel(rallySubActionPanel);
}

void GameHUD::buildSelectionHaloPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	this->selectionHaloPanel = NULL; //release the old halo
	this->selectionHaloPanel = new ScreenMaskPanel();
	float positionOnScreenX, positionOnScreenY; 
	VisRenderContext_cl::GetCurrentContext()->Project2D(this->getNodeSphere()->GetPosition(), positionOnScreenX, positionOnScreenY);
	selectionHaloPanel->init(hkvVec2(positionOnScreenX, positionOnScreenY));

	VTextureObject* selectionHaloTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\selection_halo.png");

	//selection halo
	float selectionHaloWidth = 256.0f * scale;
	float selectionHaloHeight = 256.0f * scale;
	hkvVec2 selectionHaloSize = hkvVec2(selectionHaloWidth, selectionHaloHeight);

	SelectionHalo* selectionHalo = new SelectionHalo();
	selectionHalo->AddRef();
	selectionHalo->init(hkvVec2(0,0), selectionHaloSize, selectionHaloTexture);
	selectionHalo->m_iID = VGUIManager::GetID(ID_SELECTION_HALO);
	selectionHaloPanel->addScreenMask(selectionHalo);

	HaloArrow* fixedArrow1 = selectionHalo->getFixedArrow1();
	HaloArrow* fixedArrow2 = selectionHalo->getFixedArrow2();
	HaloArrow* mobileArrow = selectionHalo->getMobileArrow();
	selectionHaloPanel->addScreenMask(fixedArrow1);
	selectionHaloPanel->addScreenMask(fixedArrow2);
	selectionHaloPanel->addScreenMask(mobileArrow);

	selectionHaloPanel->SetOrder(SELECTION_HALO_PANEL_ORDER);

	selectionHalo->updateSize();
	selectionHalo->SetScale(hkvVec2(0,0));
	selectionHalo->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.0f);
	////animations
	//fixedArrow1->SetOpacity(0.0f);
	//fixedArrow2->SetOpacity(0.0f);
	//mobileArrow->SetOpacity(0.0f);
	//selectionHalo->SetScale(hkvVec2(0,0));

	//float selectionHaloScale = selectionHalo->getScaleForCameraZoom();
	//selectionHalo->scaleTo(hkvVec2(selectionHaloScale, selectionHaloScale), 0.2f, 0.0f);

	//float selectionHaloPanelFinalOpacity = this->updateSelectionHaloPanelOpacity();
	//selectionHalo->SetOpacity(0.0f);
	//selectionHalo->fadeTo(selectionHaloPanelFinalOpacity, 0.1f, 0.0f);
	//fixedArrow1->fadeTo(selectionHaloPanelFinalOpacity, 0.1f, 0.0f);
	//fixedArrow2->fadeTo(selectionHaloPanelFinalOpacity, 0.1f, 0.0f);
	//mobileArrow->fadeTo(selectionHaloPanelFinalOpacity, 0.1f, 0.0f);

	////update arrows position
	//hkvVec2 selectionHaloTargetSize = selectionHalo->GetUnscaledTargetSize() * selectionHaloScale;
	//float arrowWidth = 32.0f * scale;
	//fixedArrow1->setDistanceFromHaloCenter(0.44f * selectionHaloTargetSize.x + 0.5f * arrowWidth);
	//fixedArrow2->setDistanceFromHaloCenter(0.44f * selectionHaloTargetSize.x);
	//mobileArrow->setDistanceFromHaloCenter(0.44f * selectionHaloTargetSize.x);
}

void GameHUD::buildInterfacePanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	//buttons (help, objectives and pause)
	VTextureObject* pauseBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\pause_btn_light.png");
	VTextureObject* helpBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\help_btn_light.png");
	VTextureObject* objectivesBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\objectives_btn_light.png");
	float btnWidth = 128.0f * scale;
	float btnHeight = 128.0f * scale;

	float borderOffset = 20.0f * scale;
	interfacePanel = new ScreenMaskPanel();
	hkvVec2 interfacePanelPosition = hkvVec2(screenSize.x - 0.5f * (3 * btnWidth + 4 * borderOffset), 0.5f * btnHeight + borderOffset);
	interfacePanel->init(interfacePanelPosition);

	InterfaceButton* pauseBtn = new InterfaceButton();
	pauseBtn->AddRef();
	pauseBtn->init(hkvVec2(btnHeight + borderOffset, 0), hkvVec2(btnWidth, btnHeight), pauseBtnTexture); 
	pauseBtn->m_iID = VGUIManager::GetID(ID_PAUSE_BTN);
	interfacePanel->addScreenMask(pauseBtn);

	InterfaceButton* helpBtn = new InterfaceButton();
	helpBtn->AddRef();
	helpBtn->init(hkvVec2(0, 0), hkvVec2(btnWidth, btnHeight), helpBtnTexture); 
	helpBtn->m_iID = VGUIManager::GetID(ID_HELP_BTN);
	interfacePanel->addScreenMask(helpBtn);

	InterfaceButton* objectivesBtn = new InterfaceButton();
	objectivesBtn->AddRef();
	objectivesBtn->init(hkvVec2(-(btnHeight + borderOffset), 0), hkvVec2(btnWidth, btnHeight), objectivesBtnTexture); 
	objectivesBtn->m_iID = VGUIManager::GetID(ID_OBJECTIVES_BTN);
	interfacePanel->addScreenMask(objectivesBtn);

	interfacePanel->SetOrder(INTERFACE_PANEL_ORDER);
}

void GameHUD::buildMiscPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	miscPanel = new MiscPanel();
	//set the misc panel out of screen
	miscPanel->init(hkvVec2(0.5f * screenSize.x, -64.0f * scale));
	
	//bg
	VTextureObject* pBgBodyTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_body.png");
	VTextureObject* pBgLeftSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_left_side.png");
	VTextureObject* pBgRightSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_right_side.png");

	hkvVec2 vMiscPanelBgBodySize = hkvVec2(500.0f * scale, 128.0f * scale);
	hkvVec4 vMiscPanelBgBodyTextureRange = hkvVec4(0, 0, vMiscPanelBgBodySize.x, vMiscPanelBgBodySize.y / scale);
	SpriteScreenMask* pMiscPanelBgBody = new SpriteScreenMask();
	pMiscPanelBgBody->AddRef();
	pMiscPanelBgBody->init(hkvVec2(0,0), vMiscPanelBgBodySize, vMiscPanelBgBodyTextureRange, pBgBodyTexture, true);
	pMiscPanelBgBody->SetOrder(MISC_PANEL_ORDER);

	hkvVec2 vMiscPanelBgSideSize = hkvVec2(128.0f * scale, 128.0f * scale);
	hkvVec2 vMiscPanelBgLeftSidePosition = hkvVec2(-0.5f * (vMiscPanelBgBodySize.x + vMiscPanelBgSideSize.x), 0);
	SpriteScreenMask* pMiscPanelBgLeftSide = new SpriteScreenMask();
	pMiscPanelBgLeftSide->AddRef();
	pMiscPanelBgLeftSide->init(vMiscPanelBgLeftSidePosition, vMiscPanelBgSideSize, pBgLeftSideTexture);
	pMiscPanelBgLeftSide->SetOrder(MISC_PANEL_ORDER);

	hkvVec2 vMiscPanelBgRightSidePosition = hkvVec2(0.5f * (vMiscPanelBgBodySize.x + vMiscPanelBgSideSize.x), 0);
	SpriteScreenMask* pMiscPanelBgRightSide = new SpriteScreenMask();
	pMiscPanelBgRightSide->AddRef();
	pMiscPanelBgRightSide->init(vMiscPanelBgRightSidePosition, vMiscPanelBgSideSize, pBgRightSideTexture);
	pMiscPanelBgRightSide->SetOrder(MISC_PANEL_ORDER);

	miscPanel->addScreenMask(pMiscPanelBgBody);
	miscPanel->addScreenMask(pMiscPanelBgLeftSide);
	miscPanel->addScreenMask(pMiscPanelBgRightSide);
}

void GameHUD::buildTimerPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	//timer
	timerPanel = new ScreenMaskPanel();
	hkvVec2 timerPanelPosition = hkvVec2(128.0f * scale, 32.0f * scale);
	timerPanel->init(timerPanelPosition);

	VTextureObject* timerBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\timer_bg.png");
	float timerBgWidth = 256.0f * scale;
	float timerBgHeight = 64.0f * scale;
	hkvVec2 timerBgSize = hkvVec2(timerBgWidth, timerBgHeight);

	SpriteScreenMask* timerBg = new SpriteScreenMask();
	timerBg->AddRef();
	timerBg->init(hkvVec2(0,0), timerBgSize, timerBgTexture);
	timerPanel->addScreenMask(timerBg);

	PrintTextLabel* timerTextLabel = new PrintTextLabel();
	timerTextLabel->AddRef();
	VColorRef fontColor = V_RGBA_WHITE;
	string gameTime = GameTimer::sharedInstance().getTimeAsString();
	float fontHeight = (gameTime.length() > 5) ? 40 * scale : 44 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	hkvVec2 timerTextLabelPosition =  (gameTime.length() > 5) ? hkvVec2(-110.0f * scale, -22.0f * scale) : hkvVec2(-80.0f * scale, -22.0f * scale);
	timerTextLabel->init(timerTextLabelPosition, gameTime, neuropolFont, fontHeight, fontColor);
	timerTextLabel->SetAnchorPoint(hkvVec2(0.0f, 0.5f));
	timerTextLabel->m_iID = VGUIManager::GetID(ID_TIMER);
	timerPanel->addTextLabel(timerTextLabel);
}

//shows/dismisses panels on pick actions
void GameHUD::showCorePanel()
{
	if (this->getNodeSphere()->isSacrificePit()) //no core panel shown for sacrifice pits
		return;

	if (this->getNodeSphere()->getTeam() != TeamManager::sharedInstance().getDefaultTeam() && 
		this->getNodeSphere()->hasAtLeastOneMetaOrb()) //neutral or enemy team and meta-orbs are still on the sphere
	{
		dismissCorePanel();
		return;
	}

	if (this->getNodeSphere()->getVisibilityStatus() != VISIBILITY_STATUS_VISIBLE)
	{
		dismissCorePanel();
		return;
	}

	int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
	if (currentLevelNumber == 1 && !TutorialsHandler::sharedInstance().isCorePanelRevealed())
		return;

	if (corePanelDismissed)
	{
		Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
		if (vdynamic_cast<SacrificePit*>(this->getNodeSphere())) //sacrifice pit don't show the top panel
			return;
		else if (!this->getNodeSphere()) //sphere can be NULL on tutorial
			return;

		this->buildCorePanel();

		int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();

		this->corePanelDismissed = false;

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale =  GameDialogManager::sharedInstance()->getScale();
		
		hkvVec2 toPosition = hkvVec2(0.5f * screenSize.x, screenSize.y - 128.0f * scale);
		float absDeltaPositionY = abs(toPosition.y - corePanel->GetPosition().y);
		float animationDuration = 0.4f * absDeltaPositionY / 300.0f;
		
		corePanel->translateTo(toPosition, animationDuration, 0.0f);
	}
	else
	{
		Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
		//if (vdynamic_cast<SacrificePit*>(this->getNodeSphere()) || this->getNodeSphere()->getTeam() != defaultTeam)
		//	dismissCorePanel();
		//else
		this->buildCorePanel();
	}
}

void GameHUD::removeMetaOrbButton(int id)
{
	if (id == VGUIManager::GetID(ID_BTN_GENERATION_META_ORB))
	{
		ScreenMaskPanel* generationPanel = (ScreenMaskPanel*) corePanel->findElementById(ID_GENERATION_PANEL);
		SpriteScreenMask* metaOrbBtn = (SpriteScreenMask*) generationPanel->findElementById(ID_BTN_GENERATION_META_ORB);
		SpriteScreenMask* metaOrbBtnReflection = (SpriteScreenMask*) generationPanel->findElementById(ID_BTN_GENERATION_META_ORB_REFLECTION);
		generationPanel->removeScreenMask(metaOrbBtn);
		generationPanel->removeScreenMask(metaOrbBtnReflection);
	}
	else if (id == VGUIManager::GetID(ID_BTN_DEFENSE_META_ORB))
	{
		ScreenMaskPanel* defensePanel = (ScreenMaskPanel*) corePanel->findElementById(ID_DEFENSE_PANEL);
		SpriteScreenMask* metaOrbBtn = (SpriteScreenMask*) defensePanel->findElementById(ID_BTN_DEFENSE_META_ORB);
		SpriteScreenMask* metaOrbBtnReflection = (SpriteScreenMask*) defensePanel->findElementById(ID_BTN_DEFENSE_META_ORB_REFLECTION);
		defensePanel->removeScreenMask(metaOrbBtn);
		defensePanel->removeScreenMask(metaOrbBtnReflection);
	}
	else if (id == VGUIManager::GetID(ID_BTN_ASSAULT_META_ORB))
	{
		ScreenMaskPanel* assaultPanel = (ScreenMaskPanel*) corePanel->findElementById(ID_ASSAULT_PANEL);
		SpriteScreenMask* metaOrbBtn = (SpriteScreenMask*) assaultPanel->findElementById(ID_BTN_ASSAULT_META_ORB);
		SpriteScreenMask* metaOrbBtnReflection = (SpriteScreenMask*) assaultPanel->findElementById(ID_BTN_ASSAULT_META_ORB_REFLECTION);
		assaultPanel->removeScreenMask(metaOrbBtn);
		assaultPanel->removeScreenMask(metaOrbBtnReflection);
	}
}

void GameHUD::showInfoPanel()
{
	if (infoPanelDismissed)
	{
		this->infoPanelDismissed = false;

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale =  GameDialogManager::sharedInstance()->getScale();

		hkvVec2 toPosition = hkvVec2(214.0f * scale, screenSize.y - 200.0f *scale);
		float absDeltaPositionX = abs(toPosition.x - infoPanel->GetPosition().x);
		float animationDuration = absDeltaPositionX / 478.0f * 0.4f;

		infoPanel->translateTo(toPosition, animationDuration, 0.0f);
	}
}

void GameHUD::dismissCorePanel()
{
	if (!corePanelDismissed)
	{
		this->corePanelDismissed = true;

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale =  GameDialogManager::sharedInstance()->getScale();

		hkvVec2 toPosition = hkvVec2(0.5f * screenSize.x, screenSize.y + 200.0f * scale);
		float absDeltaPositionY = abs(toPosition.y - corePanel->GetPosition().y);
		float animationDuration = 0.4f * absDeltaPositionY / 300.0f;

		corePanel->translateTo(toPosition, animationDuration, 0.0f);
	}
}

void GameHUD::dismissInfoPanel()
{
	if (!infoPanelDismissed)
	{
		this->infoPanelDismissed = true;
		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale =  GameDialogManager::sharedInstance()->getScale();

		hkvVec2 toPosition = hkvVec2(-214.0f * scale, screenSize.y - 200.0f *scale);
		float absDeltaPositionX = abs(toPosition.x - infoPanel->GetPosition().x);
		float animationDuration = absDeltaPositionX / 478.0f * 0.4f;

		infoPanel->translateTo(toPosition, animationDuration, 0.0f);
	}
}

void GameHUD::dismissInterfaceMenu()
{
	if (objectivesMenu)
		dismissObjectivesMenu();
	else if (helpMenu)
		dismissHelpMenu();
	else if (pauseMenu)
		dismissPauseMenu();
}

void GameHUD::showObjectivesMenu(bool withVeil, bool autoClose, bool announceNewObjectives, const vector<int> &objectivesNumbers)
{
	objectivesMenu = new ObjectivesMenu();
	objectivesMenu->init();
	objectivesMenu->show(withVeil, autoClose, announceNewObjectives, objectivesNumbers);
}

void GameHUD::dismissObjectivesMenu()
{
	objectivesMenu->dismiss();
	
	CallFuncObject* removeMenuAction = new CallFuncObject(this, (CallFunc)(&GameHUD::removeObjectivesMenu), 0.4f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeMenuAction);
	removeMenuAction->executeFunc();
}

void GameHUD::showPauseMenu(bool withVeil, bool autoClose)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();	

	pauseMenu = new PauseMenu();
	pauseMenu->init();
	pauseMenu->show(withVeil, autoClose);
}

void GameHUD::dismissPauseMenu()
{
	pauseMenu->dismiss();
	
	//remove the menu
	CallFuncObject* removeMenuAction = new CallFuncObject(this, (CallFunc)(&GameHUD::removePauseMenu), 0.4f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeMenuAction);
	removeMenuAction->executeFunc();
}

float GameHUD::updateSelectionHaloPanelOpacity()
{
	MainCamera* mainCamera = GameManager::GlobalManager().getMainCamera();
	float cameraAltitude = mainCamera->GetPosition().z;
	NodeSphere* parentNodeSphere = this->getNodeSphere();
	if (selectionHaloPanel)
	{
		float zeroAlphaAltitude = 700;
		float maxAlphaAltitude = 850;
		float opacity;
		if (cameraAltitude < zeroAlphaAltitude)
			opacity = 0;
		else if (cameraAltitude > maxAlphaAltitude)
			opacity = 1;
		else
			opacity = (cameraAltitude - zeroAlphaAltitude) / (maxAlphaAltitude - zeroAlphaAltitude);
		selectionHaloPanel->SetOpacity(opacity);

		return opacity;
	}
	return 0;
}

void GameHUD::removeObjectivesMenu()
{
	objectivesMenu = NULL;
}

void GameHUD::removePauseMenu()
{
	pauseMenu = NULL;
}

void GameHUD::showHelpMenu(bool withVeil, bool autoClose)
{
	this->helpMenu = new HelpMenu();
	helpMenu->init();
	helpMenu->show(withVeil, autoClose);
}

void GameHUD::dismissHelpMenu()
{
	helpMenu->dismiss();

	CallFuncObject* removeMenuAction = new CallFuncObject(this, (CallFunc)(&GameHUD::removeHelpMenu), 0.4f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeMenuAction);
	removeMenuAction->executeFunc();
}

void GameHUD::removeHelpMenu()
{
	helpMenu = NULL;
}

void GameHUD::showSelectionHaloPanel()
{
	this->buildSelectionHaloPanel();
}

void GameHUD::dismissSelectionHaloPanel()
{
	this->selectionHaloPanel = NULL;
	this->actionPanel->setActionMode(ACTION_NORMAL_MODE);
}

void GameHUD::showRallySphereHalo(bool animated)
{
	NodeSphere* haloSphere = NULL;
	NodeSphere* rallySphere = this->getNodeSphere()->getRallySphere();
	if (!rallySphere) //rally sphere has not been set, take the temporary actiontarget sphere
		haloSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
	else
		haloSphere = rallySphere;

	if (!haloSphere) //not in rally point mode and no rally sphere set for this sphere
		return;

	float positionOnScreenX, positionOnScreenY;
	VisRenderContext_cl::GetCurrentContext()->Project2D(haloSphere->GetPosition(), positionOnScreenX, positionOnScreenY);
	if (!rallySphereHalo)
	{
		float scale = GameDialogManager::sharedInstance()->getScale();

		rallySphereHalo = new EndpointHalo();
		hkvVec2 rallySphereHaloSize = hkvVec2(200.0f * scale, 200.0f * scale);
		rallySphereHalo->init(hkvVec2(positionOnScreenX, positionOnScreenY), rallySphereHaloSize, RALLY_POINT_MODE_COLOR);
		rallySphereHalo->SetOrder(SELECTION_HALO_PANEL_ORDER);
	}
	else
		rallySphereHalo->SetPosition(hkvVec2(positionOnScreenX, positionOnScreenY));

	if (animated)
	{
		rallySphereHalo->SetScale(hkvVec2(0,0));
		rallySphereHalo->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.0f);
	}
}

void GameHUD::removeRallySphereHalo()
{
	rallySphereHalo = NULL;
}

void GameHUD::showRallySpherePath()
{
	//path start sphere
	NodeSphere* pickedNodeSphere = this->getNodeSphere();

	//path end sphere
	NodeSphere* haloSphere = NULL;
	NodeSphere* rallySphere = this->getNodeSphere()->getRallySphere();
	if (!rallySphere) //rally sphere has not been set, take the temporary actiontarget sphere
		haloSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
	else
		haloSphere = rallySphere;

	if (!haloSphere) //not in rally point mode and no rally sphere set for this sphere
		return;
	
	activeRallyPath = new Path();
	activeRallyPath->AddRef();
	vector<NodeSphere*> nodesPath;
	GameManager::GlobalManager().findNodesPath(pickedNodeSphere, haloSphere, nodesPath);
	activeRallyPath->init(nodesPath, RALLY_POINT_MODE_COLOR);	
}

void GameHUD::removeActiveRallyPath()
{
	if (activeRallyPath)
	{
		activeRallyPath->removeSelf();
		activeRallyPath = NULL;
	}
}

void GameHUD::showActionPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	MoveSubMenuPanel* moveSubMenu = (MoveSubMenuPanel*) actionPanel->findElementById(ID_MOVE_SUB_PANEL);
	
	dismissMoveSubActionPanel();
	
	if (vdynamic_cast<SacrificePit*>(this->getNodeSphere()) /*|| vdynamic_cast<Artifact*>(this->getNodeSphere())*/)
	{
		actionPanel->showMoveBtn(hkvVec2(-78.0f * scale, -180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
	}
	else
	{
		actionPanel->showMoveBtn(hkvVec2(-78.0f * scale, -180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
		
		if (this->getNodeSphere()->getTeam() == TeamManager::sharedInstance().getDefaultTeam())
		{
			actionPanel->showRallyBtn(hkvVec2(-78.0f * scale, 0.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
			if (this->getNodeSphere()->findPotentialBridges().size() > 0)
				actionPanel->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.0f, 0.0f);
			else
				actionPanel->dismissBuildBtn(true, 0.0f);
		}
		else
		{
			actionPanel->dismissRallyBtn(true, 0.0f);
			actionPanel->dismissBuildBtn(true, 0.0f);
		}
	}
}

void GameHUD::dismissActionPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ActionButton* moveBtn = (ActionButton*) actionPanel->findElementById(ID_MOVE_BTN);
	ActionButton* rallyBtn = (ActionButton*) actionPanel->findElementById(ID_RALLY_POINT_BTN);
	ActionButton* buildBtn = (ActionButton*) actionPanel->findElementById(ID_BUILD_BTN);
	MoveSubMenuPanel* moveSubMenu = (MoveSubMenuPanel*) actionPanel->findElementById(ID_MOVE_SUB_PANEL);

	//hkvVec2 moveBtnToPosition = hkvVec2(100.0f * scale, moveBtn->GetPosition().y);
	//float absDeltaPositionY = abs(moveBtnToPosition.x - moveBtn->GetPosition().x);
	//float animationDuration = 0.4f * absDeltaPositionY / 180.0f;
	//moveBtn->translateTo(moveBtnToPosition, animationDuration, 0.0f);
	//moveBtn->scaleTo(hkvVec2(1.0f, 1.0f), animationDuration, 0.0f);

	//hkvVec2 rallyBtnToPosition = hkvVec2(100.0f * scale, rallyBtn->GetPosition().y);
	//absDeltaPositionY = abs(rallyBtnToPosition.x - rallyBtn->GetPosition().x);
	//animationDuration = 0.4f * absDeltaPositionY / 180.0f;
	//rallyBtn->translateTo(rallyBtnToPosition, animationDuration, 0.0f);
	//rallyBtn->scaleTo(hkvVec2(1.0f, 1.0f), animationDuration, 0.0f);

	//hkvVec2 buildBtnToPosition = hkvVec2(100.0f * scale, buildBtn->GetPosition().y);
	//absDeltaPositionY = abs(buildBtnToPosition.x - buildBtn->GetPosition().x);
	//animationDuration = 0.4f * absDeltaPositionY / 180.0f;
	//buildBtn->translateTo(buildBtnToPosition, animationDuration, 0.0f);
	//buildBtn->scaleTo(hkvVec2(1.0f, 1.0f), animationDuration, 0.0f);
	actionPanel->dismissMoveBtn(true, 0.0f);
	actionPanel->dismissRallyBtn(true, 0.0f);
	actionPanel->dismissBuildBtn(true, 0.0f);

	dismissMoveSubActionPanel();
	dismissRallySubActionPanel();
	dismissBuildSubActionPanel();
}

void GameHUD::showMoveSubActionPanel()
{	
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	//translate off screen rally and build btn
	ActionButton* rallyBtn = (ActionButton*) actionPanel->findElementById(ID_RALLY_POINT_BTN);
	ActionButton* buildBtn = (ActionButton*) actionPanel->findElementById(ID_BUILD_BTN);
	actionPanel->dismissRallyBtn(true, 0.0f);
	actionPanel->dismissBuildBtn(true, 0.0f);

	//translate sub panel
	MoveSubMenuPanel* moveSubActionPanel = (MoveSubMenuPanel*) actionPanel->findElementById(ID_MOVE_SUB_PANEL);
	moveSubActionPanel->translateTo(moveSubActionPanel->getShownPosition(), 0.2f, 0.2f);

	//reset the slider to be at its max value
	MoveSubMenuSlider* pSlider = (MoveSubMenuSlider*) moveSubActionPanel->findElementById(ID_MOVE_SUB_PANEL_SLIDER);
	pSlider->setKnobPositionForPercentage(1.0f);
}

void GameHUD::showTravelPath()
{
	NodeSphere* actionNodeSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
	if (!actionNodeSphere || this->getNodeSphere() == actionNodeSphere) //same sphere as picked node sphere OR no sphere was picked
	{
		this->removeActiveTravelPath();
		return;
	}

	if (!this->activeTravelPath)
	{
		vector<NodeSphere*> spheresPath;
		if (GameManager::GlobalManager().findNodesPath(GameManager::GlobalManager().getPickedNodeSphere(), actionNodeSphere, spheresPath))
		{
			//draw it
			activeTravelPath = new Path();
			activeTravelPath->AddRef();
			activeTravelPath->init(spheresPath, TRAVEL_MODE_COLOR);
		}
	}
	if (activeTravelPath)
	{
		const vector<SpriteScreenMask*> &halos = activeTravelPath->getHalos();
		if ((int) halos.size() > 0)
		{
			SpriteScreenMask* pathLastHalo = halos[halos.size() - 1];
			pathLastHalo->scaleTo(hkvVec2(2.0f, 2.0f), 0.2f, 0.0f);
		}
	}
}

float GameHUD::dismissMoveSubActionPanel()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	//translate sub panel
	float sliderWidth = 64.0f * scale;
	float sliderHeight = 512.0f * scale;
	float sliderHorizontalOffset = 200.0f * scale;
	float sliderVerticalOffset = 60.0f * scale;
	float subActionPanelWidth = 0.5f * sliderWidth + sliderHorizontalOffset;
	float subActionPanelHeight = sliderHeight;
	MoveSubMenuPanel* moveSubActionPanel = (MoveSubMenuPanel*) actionPanel->findElementById(ID_MOVE_SUB_PANEL);
	float absDeltaPositionX = abs(moveSubActionPanel->getDismissedPosition().x - moveSubActionPanel->GetPosition().x);
	float wholeAbsDeltaPositionX = abs(moveSubActionPanel->getDismissedPosition().x - moveSubActionPanel->getShownPosition().x);
	float animationDuration = 0.2f * wholeAbsDeltaPositionX / wholeAbsDeltaPositionX;
	moveSubActionPanel->translateTo(moveSubActionPanel->getDismissedPosition(), 0.2f, 0.0f);

	return animationDuration;
}

void GameHUD::dismissRallySubActionPanel()
{
	RallySubMenuPanel* rallySubActionPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
	rallySubActionPanel->hideAll(0.0f);
}

void GameHUD::dismissBuildSubActionPanel()
{
	BuildSubMenuPanel* buildSubActionPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL);
	buildSubActionPanel->hideAll(0.0f);
}

void GameHUD::showMiscPanel()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 vScreenSize = GameDialogManager::sharedInstance()->getScreenSize();

	hkvVec2 vToPosition = hkvVec2(0.5f * vScreenSize.x, 64.0f * fScale);
	float fAbsDeltaPositionY = abs(miscPanel->GetPosition().y - vToPosition.y);
	float fAnimationDuration = 0.4f * fAbsDeltaPositionY / (128.0f * fScale);

	miscPanel->translateTo(vToPosition, fAnimationDuration, 0.0f);
}

void GameHUD::dismissMiscPanel()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 vScreenSize = GameDialogManager::sharedInstance()->getScreenSize();

	hkvVec2 vToPosition = hkvVec2(0.5f * vScreenSize.x, -64.0f * fScale);
	float fAbsDeltaPositionY = abs(miscPanel->GetPosition().y - vToPosition.y);
	float fAnimationDuration = 0.4f * fAbsDeltaPositionY / (128.0f * fScale);

	miscPanel->translateTo(vToPosition, fAnimationDuration, 0.0f);
	miscPanel->setDisplayedInfoID(-1); //reset the displayed info as the entire panel disappeared
}

void GameHUD::showAchievementUnlocked(string achievementID)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	if (this->achievementUnlockedPanel)
	{
		this->achievementUnlockedPanel = NULL;
	}

	this->achievementUnlockedPanel = new ScreenMaskPanel();
	hkvVec2 achievementUnlockedPanelPosition = hkvVec2(0.5f * screenSize.x, -128.0f * scale);
	this->achievementUnlockedPanel->init(achievementUnlockedPanelPosition);

	AchievementPanel* achievementPanel = new AchievementPanel();
	achievementPanel->AddRef();
	Achievement* achievement = AchievementManager::sharedInstance().getAchievementForTag(achievementID);
	VTextureObject* achievementIcon = Vision::TextureManager.Load2DTexture(achievement->getIconPath().c_str());
	achievementPanel->init(achievementID, hkvVec2(0,0), achievement->getTitle(), achievement->getDescription(), achievementIcon);
	achievementUnlockedPanel->addPanel(achievementPanel);

	//animation
	achievementUnlockedPanel->translateTo(hkvVec2(0.5f * screenSize.x, 128.0f * scale), 0.2f, 0.0f);

	//dismiss
	CallFuncObject* dismissAchievementPanel = new CallFuncObject(this, (CallFunc)(&GameHUD::dismissAchievementUnlocked), 5.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(dismissAchievementPanel);
	dismissAchievementPanel->executeFunc();
}

void GameHUD::dismissAchievementUnlocked()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	achievementUnlockedPanel->translateTo(hkvVec2(0.5f * screenSize.x, -128.0f * scale), 0.2f, 0.0f);

	CallFuncObject* removeAchievementPanel = new CallFuncObject(this, (CallFunc)(&GameHUD::removeAchievementUnlocked), 0.2f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeAchievementPanel);
	removeAchievementPanel->executeFunc();
}

void GameHUD::removeAchievementUnlocked()
{
	achievementUnlockedPanel = NULL;
}

bool GameHUD::containsClick(hkvVec2 touchLocation)
{
	if (pauseMenu)
		return true;
	else
		return panelContainsClick(infoPanel, touchLocation) ||
			   panelContainsClick(corePanel, touchLocation) ||
			   panelContainsClick(objectivesMenu, touchLocation) ||
			   panelContainsClick(actionPanel, touchLocation) ||
			   panelContainsClick(miscPanel, touchLocation) ||
			   panelContainsClick(interfacePanel, touchLocation);
}

bool GameHUD::panelContainsClick(ScreenMaskPanel* panel, hkvVec2 touchLocation)
{
	if (!panel)
		return false;

	//check masks
	vector<SpriteScreenMask*> masks = panel->getScreenMasks();
	for (int maskIndex = 0; maskIndex != masks.size(); maskIndex++)
	{
		SpriteScreenMask* mask = masks[maskIndex];
		if (mask->containsTouch(touchLocation))
			return true;
	}

	//check panels
	vector<ScreenMaskPanel*> panels = panel->getPanels();
	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		if (panelContainsClick(panels[panelIndex], touchLocation))
			return true;
	}

	return false;
}

vector<PrintTextLabel*> GameHUD::retrieveAllTextLabels()
{
	vector<PrintTextLabel*> allTextLabels;
	if (infoPanel)
	{
		vector<PrintTextLabel*> infoPanelTextLabels = infoPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), infoPanelTextLabels.begin(), infoPanelTextLabels.end());
	}
	if (corePanel)
	{
		vector<PrintTextLabel*> corePanelTextLabels = corePanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), corePanelTextLabels.begin(), corePanelTextLabels.end());
	}
	if (actionPanel)
	{
		vector<PrintTextLabel*> actionPanelTextLabels = actionPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), actionPanelTextLabels.begin(), actionPanelTextLabels.end());
	}
	if (miscPanel)
	{
		vector<PrintTextLabel*> miscPanelTextLabels = miscPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), miscPanelTextLabels.begin(), miscPanelTextLabels.end());
	}

	vector<HUDMessage*> activeMessages = MessageHandler::sharedInstance().getActiveMessages();
	for (int messageIndex = 0; messageIndex != activeMessages.size(); messageIndex++)
	{
		HUDMessage* message = activeMessages[messageIndex];
		vector<PrintTextLabel*>& messageTextLabels = message->getTextLabels(); 
		allTextLabels.insert(allTextLabels.end(), messageTextLabels.begin(), messageTextLabels.end());
	}
	//if (pauseMenu)
	//{
	//	vector<PrintTextLabel*> pauseMenuTextLabels = pauseMenu->retrieveAllTextLabels();
	//	allTextLabels.insert(allTextLabels.end(), pauseMenuTextLabels.begin(), pauseMenuTextLabels.end());
	//}
	if (team1SacrifiedOrbsCountLabel)
		allTextLabels.push_back(team1SacrifiedOrbsCountLabel);
	if (team2SacrifiedOrbsCountLabel)
		allTextLabels.push_back(team2SacrifiedOrbsCountLabel);
	if (team3SacrifiedOrbsCountLabel)
		allTextLabels.push_back(team3SacrifiedOrbsCountLabel);
	if (team4SacrifiedOrbsCountLabel)
		allTextLabels.push_back(team4SacrifiedOrbsCountLabel);



	return allTextLabels;
}

void GameHUD::drawTravelPath(NodeSphere* startSphere, NodeSphere* endSphere, hkvVec2 endTouchLocation)
{
	bool pathDrawn = false;
	if (endSphere) //draw a path with halos
	{
		pathDrawn = true;
		if (!activeTravelPathEndSphere) //no need to draw the same path
		{
			//find the spheres path
			vector<NodeSphere*> spheresPath;
			if (GameManager::GlobalManager().findNodesPath(startSphere, endSphere, spheresPath))
			{
				//destroy the previous one
				this->removeActiveTravelPath();

				//draw it
				activeTravelPath = new Path();
				activeTravelPath->AddRef();
				activeTravelPath->init(spheresPath, TRAVEL_MODE_COLOR);
				activeTravelPathEndSphere = endSphere;
			}
			else
				pathDrawn = false;
		}
	}

	if (!pathDrawn) //draw a path which is a simple line
	{
		if (this->activeTravelPathEndSphere || !this->activeTravelPath) //a sphere path exists, destroy it OR no path was drawn before, create it
		{
			this->removeActiveTravelPath();

			this->activeTravelPathEndSphere = NULL;
			Path* path = new Path();
			path->AddRef();
			path->init(startSphere, endTouchLocation, TRAVEL_MODE_COLOR);
			activeTravelPath = path;
		}
		else
		{
			activeTravelPath->updateSingleLinePath(0, endTouchLocation);
		}
	}
}

void GameHUD::removeActiveTravelPath()
{
	if (activeTravelPath)
	{
		activeTravelPath->removeSelf();
		activeTravelPath = NULL;
	}
}

void GameHUD::showBuildSphereHalo()
{
	NodeSphere* haloSphere = GameManager::GlobalManager().getActionTargetNodeSphere();

	if (!haloSphere) //not in rally point mode and no rally sphere set for this sphere
		return;

	float positionOnScreenX, positionOnScreenY;
	VisRenderContext_cl::GetCurrentContext()->Project2D(haloSphere->GetPosition(), positionOnScreenX, positionOnScreenY);

	float scale = GameDialogManager::sharedInstance()->getScale();

	if (!buildSphereHalo)
	{
		buildSphereHalo = new EndpointHalo();
		hkvVec2 buildSphereHaloSize = hkvVec2(200.0f * scale, 200.0f * scale);
		buildSphereHalo->init(hkvVec2(positionOnScreenX, positionOnScreenY), buildSphereHaloSize, BUILD_MODE_COLOR);
		buildSphereHalo->SetOrder(SELECTION_HALO_PANEL_ORDER);
	}
	else
		buildSphereHalo->SetPosition(hkvVec2(positionOnScreenX, positionOnScreenY));

	buildSphereHalo->SetScale(hkvVec2(0,0));
	buildSphereHalo->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.0f);
}

void GameHUD::removeBuildSphereHalo()
{
	buildSphereHalo = NULL;
}

void GameHUD::removePotentialBridges()
{
	for (int potentialBridgeIndex = 0; potentialBridgeIndex != potentialBridges.size(); potentialBridgeIndex++)
	{
		Bridge* potentialBridge = potentialBridges[potentialBridgeIndex];
		potentialBridge->Release();
	}
	potentialBridges.clear();
}

NodeSphere* GameHUD::getNodeSphere()
{
	return GameManager::GlobalManager().getPickedNodeSphere();
}

void GameHUD::update(float dt)
{
	this->updateInfoPanel();
	this->updateGlobalVariables();
}

void GameHUD::updateHUDForPickedNodeSphere()
{
	if (this->getNodeSphere())
	{
		//left panel
		this->getNodeSphere()->setPlayerAlliedOrbsCountDirty(true);
		this->getNodeSphere()->setPlayerEnemyOrbsCountDirty(true);
		//this->getNodeSphere()->setSphereLifeDirty(true);
	}
}

void GameHUD::updateSelectionHaloPanelPosition()
{
	if (this->selectionHaloPanel && this->getNodeSphere())
	{
		float positionOnScreenX, positionOnScreenY; 
		VisRenderContext_cl::GetCurrentContext()->Project2D(this->getNodeSphere()->GetPosition(), positionOnScreenX, positionOnScreenY);

		selectionHaloPanel->SetPosition(hkvVec2(positionOnScreenX, positionOnScreenY));
	}
}

void GameHUD::updateRallySphereHaloPosition()
{
	if (!this->getNodeSphere())
		return;

	NodeSphere* haloSphere = NULL;
	NodeSphere* rallySphere = this->getNodeSphere()->getRallySphere();
	if (!rallySphere) //rally sphere has not been set, take the temporary actiontarget sphere
		haloSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
	else
		haloSphere = rallySphere;

	if (!haloSphere) //not in rally point mode and no rally sphere set for this sphere
		return;

	if (this->rallySphereHalo)
	{
		float positionOnScreenX, positionOnScreenY; 
		VisRenderContext_cl::GetCurrentContext()->Project2D(haloSphere->GetPosition(), positionOnScreenX, positionOnScreenY);

		rallySphereHalo->SetPosition(hkvVec2(positionOnScreenX, positionOnScreenY));
	}
}

void GameHUD::updateBuildSphereHaloPosition()
{
	if (!this->getNodeSphere())
		return;

	NodeSphere* haloSphere = GameManager::GlobalManager().getActionTargetNodeSphere();

	if (!haloSphere) //not in rally point mode and no rally sphere set for this sphere
		return;

	if (this->buildSphereHalo)
	{
		float positionOnScreenX, positionOnScreenY; 
		VisRenderContext_cl::GetCurrentContext()->Project2D(haloSphere->GetPosition(), positionOnScreenX, positionOnScreenY);

		buildSphereHalo->SetPosition(hkvVec2(positionOnScreenX, positionOnScreenY));
	}
}

void GameHUD::updatePathsPositions()
{
	if (this->activeTravelPath)
		activeTravelPath->updateSpheresPath();
	if (this->activeRallyPath)
		activeRallyPath->updateSpheresPath();
}

void GameHUD::updateInfoPanel()
{
	if (this->getNodeSphere())
	{
		Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
		//allied orbs count
		if (this->getNodeSphere()->isPlayerAlliedOrbsCountDirty())
		{
			int alliedOrbsCount = this->getNodeSphere()->getAlliedOrbsCountForTeam(defaultTeam);
			PrintTextLabel* alliedOrbsCountLabel = (PrintTextLabel*) this->infoPanel->findElementById(ID_ALLIED_ORBS_COUNT_LABEL);
			alliedOrbsCountLabel->setText(stringFromInt(alliedOrbsCount));
		}
		//enemy orbs count
		if (this->getNodeSphere()->isPlayerEnemyOrbsCountDirty())
		{
			int enemyOrbsCount = this->getNodeSphere()->getEnemyOrbsCountForTeam(defaultTeam);
			PrintTextLabel* enemyOrbsCountLabel = (PrintTextLabel*) this->infoPanel->findElementById(ID_ENEMY_ORBS_COUNT_LABEL);
			enemyOrbsCountLabel->setText(stringFromInt(enemyOrbsCount));
		}
		//sphere life
		//if (this->getNodeSphere()->isSphereLifeDirty())
		//{
		//	float sphereLife = this->getNodeSphere()->getLife();
		//	PrintTextLabel* sphereLifeLabel = (PrintTextLabel*) this->infoPanel->findElementById(ID_SPHERE_LIFE_COUNT_LABEL);
		//	sphereLifeLabel->setText(stringFromInt((int) GeometryUtils::round(sphereLife)));
		//}
	}
	else
	{

	}
}

void GameHUD::updateGlobalVariables()
{
	//update time
	//PrintTextLabel* timeTextLabel = (PrintTextLabel*) timerPanel->findElementById(ID_TIMER);
	//string gameTime = GameTimer::sharedInstance().getTimeAsString();
	//timeTextLabel->setText(gameTime);
}

void GameHUD::completeObjective(int number)
{
	//change the texture of the objective point
	if (objectivesMenu)
		objectivesMenu->completeObjective(number);
}

bool GameHUD::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	if (pauseMenu)
		return panelProcessPointerEvent(pauseMenu, touchLocation, eventType);
	else if (objectivesMenu)
		panelProcessPointerEvent(objectivesMenu, touchLocation, eventType);
	else if (helpMenu)
		panelProcessPointerEvent(helpMenu, touchLocation, eventType);	
	else
		return panelProcessPointerEvent(corePanel, touchLocation, eventType) ||
			   panelProcessPointerEvent(actionPanel, touchLocation, eventType) ||
			   panelProcessPointerEvent(miscPanel, touchLocation, eventType) ||
			   panelProcessPointerEvent(interfacePanel, touchLocation, eventType);

	return false;
}

bool GameHUD::panelProcessPointerEvent(ScreenMaskPanel* panel, hkvVec2 touchLocation, int eventType)
{
	if (!panel)
		return false;

	//check direct child masks
	vector<SpriteScreenMask*> screenMasks = panel->getScreenMasks();
	for (int maskIndex = 0; maskIndex != screenMasks.size(); maskIndex++)
	{
		SpriteScreenMask* mask = screenMasks[maskIndex];
		if (mask->trigger(touchLocation, eventType))
		{
			return true;
		}
	}

	//check child panels
	vector<ScreenMaskPanel*> panels = panel->getPanels();
	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* childPanel = panels[panelIndex];
		if (vdynamic_cast<MoveSubMenuSlider*>(childPanel))
		{
			if (((MoveSubMenuSlider*)childPanel)->trigger(touchLocation, eventType))
				return true;
		}
		else if (vdynamic_cast<GameSpeedSlider*>(childPanel))
		{
			if (((GameSpeedSlider*)childPanel)->trigger(touchLocation, eventType))
				return true;
		}
		else if (vdynamic_cast<GaugeController*>(childPanel))
		{
			if (((GaugeController*)childPanel)->trigger(touchLocation, eventType))
				return true;
		}
		else if (vdynamic_cast<ClickableScreenMaskPanel*>(childPanel))
		{
			if (((ClickableScreenMaskPanel*)childPanel)->trigger(touchLocation, eventType))
				return true;
		}
		if (panelProcessPointerEvent(childPanel, touchLocation, eventType))
			return true;
	}

	return false;
}

//////////////////////////////////CorePanel//////////////////////////////////
void CorePanel::buildBackground()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* supportLeftSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_left_side.png");
	VTextureObject* supportRightSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_right_side.png");
	VTextureObject* supportBodyTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_body.png");

	//bg
	ScreenMaskPanel* supportPanel = new ScreenMaskPanel();
	supportPanel->AddRef();
	hkvVec2 supportPanelPosition = hkvVec2(0, 64.0f * scale);
	supportPanel->init(supportPanelPosition);
	supportPanel->m_iID = VGUIManager::GetID(ID_CORE_PANEL_BG);

	float supportSideWidth = 128.0f * scale;
	float supportSideHeight = 256.0f * scale;
	hkvVec2 supportSideSize = hkvVec2(supportSideWidth, supportSideHeight);

	SpriteScreenMask* supportLeftSide = new SpriteScreenMask();
	supportLeftSide->AddRef();
	hkvVec2 supportLeftSidePosition = hkvVec2(-514.0f * scale, 0);
	supportLeftSide->init(supportLeftSidePosition, supportSideSize, supportLeftSideTexture);
	supportPanel->addScreenMask(supportLeftSide);

	SpriteScreenMask* supportRightSide = new SpriteScreenMask();
	supportRightSide->AddRef();
	hkvVec2 supportRightSidePosition = hkvVec2(514.0f * scale, 0);
	supportRightSide->init(supportRightSidePosition, supportSideSize, supportRightSideTexture);
	supportPanel->addScreenMask(supportRightSide);

	SpriteScreenMask* supportBody = new SpriteScreenMask();
	supportBody->AddRef();
	hkvVec2 supportBodyPosition = hkvVec2(0, 0);
	hkvVec2 supportBodySize = hkvVec2(900.0f * scale, 256.0f * scale);
	hkvVec4 supportBodyTextureRange = hkvVec4(0, 0, supportBodySize.x / scale, supportBodySize.y / scale);
	supportBody->init(supportBodyPosition, supportBodySize, supportBodyTextureRange, supportBodyTexture, true);
	supportPanel->addScreenMask(supportBody);
	supportBody->SetFiltering(false);
	//supportBody->SetScale(hkvVec2(scale, scale));

	supportPanel->SetOrder(CORE_PANEL_ORDER);
	this->addPanel(supportPanel);
}

void CorePanel::buildAndShowMainContent()
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	NodeSphere* pickedNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();
	if (!pickedNodeSphere)
		return;

	//content holder panel
	ScreenMaskPanel* contentHolder = (ScreenMaskPanel*) this->findElementById(ID_CORE_PANEL_CONTENT_HOLDER);
	if (!contentHolder)
	{
		contentHolder = new ScreenMaskPanel();
		contentHolder->AddRef();
		contentHolder->init(hkvVec2(0,0));
		contentHolder->m_iID = VGUIManager::GetID(ID_CORE_PANEL_CONTENT_HOLDER);
		this->addPanel(contentHolder);
	}

	if (pickedNodeSphere->isCommonNodeSphere())
	{
		int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
		VTextureObject* generationMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\generation_meta_orb.png");
		VTextureObject* defenseMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\defense_meta_orb.png");
		VTextureObject* assaultMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\assault_meta_orb.png");

		float metaOrbWidth = 256.0f * scale;
		float metaOrbHeight = 256.0f * scale;
		hkvVec2 metaOrbSize = hkvVec2(metaOrbWidth, metaOrbHeight);

		//text labels
		float fontHeight = 36 * scale;
		VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fontHeight);

		//generation meta-orb
		ScreenMaskPanel* generationPanel = new ScreenMaskPanel();
		generationPanel->AddRef();
		hkvVec2 generationPanelPosition = (currentLevelNumber < 3) ? hkvVec2(0, 40.0f * scale) : hkvVec2(-307.0f * scale, 40.0f * scale);
		generationPanel->init(generationPanelPosition);
		generationPanel->m_iID = VGUIManager::GetID(ID_GENERATION_PANEL);

		contentHolder->addPanel(generationPanel);

		PrintTextLabel* generationTextLabel = new PrintTextLabel();
		generationTextLabel->AddRef();
		VColorRef generationFontColor = VColorRef(0, 255, 36);
		string generationText = StringsXmlHelper::sharedInstance().getStringForTag("generation");
		generationTextLabel->init(hkvVec2(0, -25.0f * scale), generationText, neuropolFont, fontHeight, generationFontColor);
		generationTextLabel->m_iID = VGUIManager::GetID(ID_GENERATION_TEXT_LABEL);
		generationTextLabel->setRenderedBehindVeils(true);
		generationPanel->addTextLabel(generationTextLabel);

		if (pickedNodeSphere->getGenerationMetaOrb())
		{
			showGauge(VGUIManager::GetID(ID_META_ORB_GENERATION), false);
		}
		else
		{
			generationTextLabel->SetPosition(hkvVec2(0, 55.0f * scale));

			CorePanelButton* generationMetaOrbBtn = new CorePanelButton();
			generationMetaOrbBtn->AddRef();
			hkvVec2 generationMetaOrbPosition = hkvVec2(0, -107.0f * scale);
			generationMetaOrbBtn->init(generationMetaOrbPosition, metaOrbSize, generationMetaOrbTexture);
			generationMetaOrbBtn->m_iID = VGUIManager::GetID(ID_BTN_GENERATION_META_ORB);
			generationPanel->addScreenMask(generationMetaOrbBtn);

			SpriteScreenMask* generationReflection = new SpriteScreenMask();
			generationReflection->AddRef();
			hkvVec2 generationReflectionPosition = hkvVec2(0, 107.0f * scale);
			generationReflection->init(generationReflectionPosition, metaOrbSize, generationMetaOrbTexture, hkvVec2(1.0f, 1.0f), 180);
			generationReflection->m_iID = VGUIManager::GetID(ID_BTN_GENERATION_META_ORB_REFLECTION);
			generationPanel->addScreenMask(generationReflection);
			generationReflection->SetOpacity(0.2f);
		}

		generationPanel->SetOrder(CORE_PANEL_ORDER - 1);

		//defense meta-orb
		ScreenMaskPanel* defensePanel = new ScreenMaskPanel();
		defensePanel->AddRef();
		hkvVec2 defensePanelPosition = hkvVec2(0, 40.0f * scale);
		defensePanel->init(defensePanelPosition);
		defensePanel->m_iID = VGUIManager::GetID(ID_DEFENSE_PANEL);

		contentHolder->addPanel(defensePanel);

		PrintTextLabel* defenseTextLabel = new PrintTextLabel();
		defenseTextLabel->AddRef();
		VColorRef defenseFontColor = VColorRef(0, 147, 250);
		string defenseText = StringsXmlHelper::sharedInstance().getStringForTag("defense");
		defenseTextLabel->init(hkvVec2(0, -10.0f * scale), defenseText, neuropolFont, fontHeight, defenseFontColor);
		defenseTextLabel->m_iID = VGUIManager::GetID(ID_DEFENSE_TEXT_LABEL);
		defenseTextLabel->setRenderedBehindVeils(true);
		defensePanel->addTextLabel(defenseTextLabel);

		if (pickedNodeSphere->getDefenseMetaOrb())
		{
			showGauge(VGUIManager::GetID(ID_META_ORB_DEFENSE), false);
		}
		else
		{
			defenseTextLabel->SetPosition(hkvVec2(0, 55.0f * scale));

			CorePanelButton* defenseMetaOrbBtn = new CorePanelButton();
			defenseMetaOrbBtn->AddRef();
			hkvVec2 defenseMetaOrbPosition = hkvVec2(0, -107.0f * scale);
			defenseMetaOrbBtn->init(defenseMetaOrbPosition, metaOrbSize, defenseMetaOrbTexture);
			defenseMetaOrbBtn->m_iID = VGUIManager::GetID(ID_BTN_DEFENSE_META_ORB);
			defensePanel->addScreenMask(defenseMetaOrbBtn);

			SpriteScreenMask* defenseReflection = new SpriteScreenMask();
			defenseReflection->AddRef();
			hkvVec2 defenseReflectionPosition = hkvVec2(0, 107.0f * scale);
			defenseReflection->init(defenseReflectionPosition, metaOrbSize, defenseMetaOrbTexture, hkvVec2(1.0f, 1.0f), 180);
			defenseReflection->m_iID = VGUIManager::GetID(ID_BTN_DEFENSE_META_ORB_REFLECTION);
			defensePanel->addScreenMask(defenseReflection);
			defenseReflection->SetOpacity(0.2f);
		}

		defensePanel->SetOrder(CORE_PANEL_ORDER - 1);

		//assault meta-orb
		ScreenMaskPanel* assaultPanel = new ScreenMaskPanel();
		assaultPanel->AddRef();
		hkvVec2 assaultPanelPosition = hkvVec2(307.0f * scale, 40.0f * scale);
		assaultPanel->init(assaultPanelPosition);
		assaultPanel->m_iID = VGUIManager::GetID(ID_ASSAULT_PANEL);
		contentHolder->addPanel(assaultPanel);

		PrintTextLabel* assaultTextLabel = new PrintTextLabel();
		assaultTextLabel->AddRef();
		VColorRef assaultFontColor = VColorRef(255, 0, 0);
		string assaultText = StringsXmlHelper::sharedInstance().getStringForTag("assault");
		assaultTextLabel->init(hkvVec2(0, -10.0f * scale), assaultText, neuropolFont, fontHeight, assaultFontColor);
		assaultTextLabel->m_iID = VGUIManager::GetID(ID_ASSAULT_TEXT_LABEL);
		assaultTextLabel->setRenderedBehindVeils(true);
		assaultPanel->addTextLabel(assaultTextLabel);

		if (pickedNodeSphere->getAssaultMetaOrb())
		{
			showGauge(VGUIManager::GetID(ID_META_ORB_ASSAULT), false);
		}
		else
		{
			assaultTextLabel->SetPosition(hkvVec2(0, 55.0f * scale));

			CorePanelButton* assaultMetaOrbBtn = new CorePanelButton();
			assaultMetaOrbBtn->AddRef();
			hkvVec2 assaultMetaOrbPosition = hkvVec2(0, -107.0f * scale);
			assaultMetaOrbBtn->init(assaultMetaOrbPosition, metaOrbSize, assaultMetaOrbTexture);
			assaultMetaOrbBtn->m_iID = VGUIManager::GetID(ID_BTN_ASSAULT_META_ORB);
			assaultPanel->addScreenMask(assaultMetaOrbBtn);

			SpriteScreenMask* assaultReflection = new SpriteScreenMask();
			assaultReflection->AddRef();
			hkvVec2 assaultReflectionPosition = hkvVec2(0, 107.0f * scale);
			assaultReflection->init(assaultReflectionPosition, metaOrbSize, assaultMetaOrbTexture, hkvVec2(1.0f, 1.0f), 180);
			assaultReflection->m_iID = VGUIManager::GetID(ID_BTN_ASSAULT_META_ORB_REFLECTION);
			assaultPanel->addScreenMask(assaultReflection);
			assaultReflection->SetOpacity(0.2f);
		}

		assaultPanel->SetOrder(CORE_PANEL_ORDER - 1);
		//}

		if (currentLevelNumber < 3) //remove defense and assault for lvl1 and lvl2
		{
			//ScreenMaskPanel* generationPanelToHide = (ScreenMaskPanel*) corePanel->findElementById(ID_GENERATION_PANEL);
			ScreenMaskPanel* defensePanelToHide = (ScreenMaskPanel*) contentHolder->findElementById(ID_DEFENSE_PANEL);
			ScreenMaskPanel* assaultPanelToHide = (ScreenMaskPanel*) contentHolder->findElementById(ID_ASSAULT_PANEL);

			if (defensePanelToHide)
			{
				contentHolder->removePanel(defensePanelToHide);
			}
			if (assaultPanelToHide)
			{
				contentHolder->removePanel(assaultPanelToHide);
			}
			//if (generationPanelToHide)
			//	generationPanelToHide->SetPosition(hkvVec2(0,0));
		}
	}
	else if (pickedNodeSphere->isArtifact())
	{
		Artifact* pArtifact = (Artifact*) pickedNodeSphere;
		if (pArtifact->getCooldownTimer()->isActive()) //a power has been used recently, show the timer
		{

		}
		else //show powers
		{
			float fPowerTitleFontHeight = 40.0f * scale;
			VisFont_cl* pPowerTitleFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fPowerTitleFontHeight);
			VColorRef fontColor = VColorRef(183, 255, 247);

			VTextureObject* pAnnihilationTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\artifact_icon_annihilation.png");
			VTextureObject* pRevelationTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\artifact_icon_revelation.png");
			VTextureObject* pBerserkTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\artifact_icon_berzerk.png");
			hkvVec2 artifactIconSize = hkvVec2(128.0f * scale, 128.0f * scale);

			//annihilation
			ScreenMaskPanel* pAnnihilationPanel = new ScreenMaskPanel();
			pAnnihilationPanel->AddRef();
			pAnnihilationPanel->init(hkvVec2(-292.0f * scale, 40.0f * scale));
			contentHolder->addPanel(pAnnihilationPanel);

			CorePanelButton* pAnnihilationBtn = new CorePanelButton();
			pAnnihilationBtn->AddRef();
			pAnnihilationBtn->init(hkvVec2(0, -16.0f * scale), artifactIconSize, pAnnihilationTexture);
			pAnnihilationPanel->addScreenMask(pAnnihilationBtn);

			PrintTextLabel* pAnnihilationTitle = new PrintTextLabel();
			pAnnihilationTitle->AddRef();
			string strAnnihilationTitle = StringsXmlHelper::sharedInstance().getStringForTag("annihilation");
			pAnnihilationTitle->init(hkvVec2(0, 60.0f * scale), strAnnihilationTitle, pPowerTitleFont, fPowerTitleFontHeight, fontColor);
			pAnnihilationPanel->addTextLabel(pAnnihilationTitle);

			//revelation
			ScreenMaskPanel* pRevelationPanel = new ScreenMaskPanel();
			pRevelationPanel->AddRef();
			pRevelationPanel->init(hkvVec2(0, 40.0f * scale));
			contentHolder->addPanel(pRevelationPanel);

			CorePanelButton* pRevelationBtn = new CorePanelButton();
			pRevelationBtn->AddRef();
			pRevelationBtn->init(hkvVec2(0, -16.0f * scale), artifactIconSize, pRevelationTexture);
			pRevelationPanel->addScreenMask(pRevelationBtn);

			PrintTextLabel* pRevelationTitle = new PrintTextLabel();
			pRevelationTitle->AddRef();
			string strRevelationTitle = StringsXmlHelper::sharedInstance().getStringForTag("annihilation");
			pRevelationTitle->init(hkvVec2(0, 60.0f * scale), strRevelationTitle, pPowerTitleFont, fPowerTitleFontHeight, fontColor);
			pRevelationPanel->addTextLabel(pRevelationTitle);

			//berserk
			ScreenMaskPanel* pBerserkPanel = new ScreenMaskPanel();
			pBerserkPanel->AddRef();
			pBerserkPanel->init(hkvVec2(0, 40.0f * scale));
			contentHolder->addPanel(pBerserkPanel);

			CorePanelButton* pBerserkBtn = new CorePanelButton();
			pBerserkBtn->AddRef();
			pBerserkBtn->init(hkvVec2(0, -16.0f * scale), artifactIconSize, pRevelationTexture);
			pBerserkPanel->addScreenMask(pBerserkBtn);

			PrintTextLabel* pBerserkTitle = new PrintTextLabel();
			pBerserkTitle->AddRef();
			string strBerserkTitle = StringsXmlHelper::sharedInstance().getStringForTag("annihilation");
			pBerserkTitle->init(hkvVec2(0, 60.0f * scale), strBerserkTitle, pPowerTitleFont, fPowerTitleFontHeight, fontColor);
			pBerserkPanel->addTextLabel(pBerserkTitle);
		}
	}

	//fade in content holder that was previously faded out
	contentHolder->fadeTo(1.0f, 0.15f, 0.0f);
}

void CorePanel::showGauge(int id, bool animated)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	MetaOrb* selectedMetaOrb = NULL;
	VTextureObject* updateBtnTexture = NULL;
	PrintTextLabel* titleTextLabel = NULL;
	ScreenMaskPanel* gaugePanel = NULL;
	int updateButtonID = 0;

	if (id == VGUIManager::GetID(ID_META_ORB_GENERATION))	
	{
		selectedMetaOrb = GameManager::GlobalManager().getPickedNodeSphere()->getGenerationMetaOrb();
		updateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_generation.png");
		titleTextLabel = (PrintTextLabel*) this->findElementById(ID_GENERATION_TEXT_LABEL);
		gaugePanel = (ScreenMaskPanel*) this->findElementById(ID_GENERATION_PANEL);
		updateButtonID = VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_GENERATION);
	}
	else if (id == VGUIManager::GetID(ID_META_ORB_DEFENSE))
	{
		selectedMetaOrb = GameManager::GlobalManager().getPickedNodeSphere()->getDefenseMetaOrb();
		updateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_defense.png");
		titleTextLabel = (PrintTextLabel*) this->findElementById(ID_DEFENSE_TEXT_LABEL);
		gaugePanel = (ScreenMaskPanel*) this->findElementById(ID_DEFENSE_PANEL);
		updateButtonID = VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_DEFENSE);
	}
	else if (id == VGUIManager::GetID(ID_META_ORB_ASSAULT))
	{
		selectedMetaOrb = GameManager::GlobalManager().getPickedNodeSphere()->getAssaultMetaOrb();
		updateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_assault.png");
		titleTextLabel = (PrintTextLabel*) this->findElementById(ID_ASSAULT_TEXT_LABEL);
		gaugePanel = (ScreenMaskPanel*) this->findElementById(ID_ASSAULT_PANEL);
		updateButtonID = VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_ASSAULT);
	}

	if (!selectedMetaOrb)
		return;

	Gauge* gauge = new Gauge();
	gauge->AddRef();
	hkvVec2 gaugePosition = hkvVec2(0, -90.0f * scale);
	gauge->init(gaugePosition, selectedMetaOrb);
	gaugePanel->addPanel(gauge);

	//show update button
	GaugeUpdateButton* updateBtn = new GaugeUpdateButton();
	updateBtn->AddRef();
	hkvVec2 updateBtnPosition =  hkvVec2(0, 45.0f * scale);
	hkvVec2 updateBtnSize = hkvVec2(128.0f * scale, 128.0f * scale);
	updateBtn->init(updateBtnPosition, updateBtnSize, updateBtnTexture);
	updateBtn->m_iID = updateButtonID;
	updateBtn->SetOrder(CORE_PANEL_ORDER - 1);
	gaugePanel->addScreenMask(updateBtn);

	if (animated)
	{
		////translate the title text label
		titleTextLabel->translateTo(hkvVec2(0, -25.0f * scale), 0.2f, 0.0f);

		////fade in masks
		gauge->SetOpacity(0);
		updateBtn->SetOpacity(0);

		gauge->fadeTo(1.0f, 0.2f, 0.25f);
		updateBtn->fadeTo(1.0f, 0.2f, 0.25f);
	}
}

void CorePanel::showGaugeUpgradePanel()
{
	if (!selectedMetaOrb)
		return;

	float scale = GameDialogManager::sharedInstance()->getScale();
	
	ScreenMaskPanel* contentHolder = (ScreenMaskPanel*) GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->findElementById(ID_CORE_PANEL_CONTENT_HOLDER);

	//gauge
	Gauge* gauge = new Gauge();
	gauge->AddRef();
	hkvVec2 gaugePosition = hkvVec2(0, -128.0f * scale);
	gauge->init(gaugePosition, selectedMetaOrb);
	contentHolder->addPanel(gauge);

	//gauge controller
	GaugeController* gaugeController = new GaugeController();
	gaugeController->AddRef();
	gaugeController->init(gauge, hkvVec2(0, 64.0f * scale));
	contentHolder->addPanel(gaugeController);

	//validate and back buttons
	VTextureObject* validateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_validate_btn.png");
	VTextureObject* backBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_back_btn.png");

	hkvVec2 btnSize = hkvVec2(80.0f * scale, 80.0f * scale);

	CorePanelButton* validateBtn = new CorePanelButton();
	validateBtn->AddRef();
	hkvVec2 validateBtnPosition = hkvVec2(350.0f * scale, 50.0f * scale);
	validateBtn->init(validateBtnPosition, btnSize, validateBtnTexture);
	validateBtn->SetOrder(CORE_PANEL_ORDER - 1);
	validateBtn->m_iID = VGUIManager::GetID(ID_CORE_PANEL_GAUGE_VALIDATE);
	contentHolder->addScreenMask(validateBtn);

	CorePanelButton* backBtn = new CorePanelButton();
	backBtn->AddRef();
	hkvVec2 backBtnPosition = hkvVec2(-350.0f * scale, 50.0f * scale);
	backBtn->init(backBtnPosition, btnSize, backBtnTexture);
	backBtn->SetOrder(CORE_PANEL_ORDER - 1);
	backBtn->m_iID = VGUIManager::GetID(ID_CORE_PANEL_GAUGE_BACK);
	contentHolder->addScreenMask(backBtn);

	//fade in content holder that was previously faded out
	contentHolder->fadeTo(1.0f, 0.2f, 0.0f);
}

//////////////////////////////////CorePanelButton//////////////////////////////////
V_IMPLEMENT_DYNCREATE(CorePanelButton, ButtonScreenMask, Vision::GetEngineModule());

void CorePanelButton::onClick()
{
	////Common NodeSphere
	NodeSphere* pNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();

	if (this->m_iID == VGUIManager::GetID(ID_BTN_GENERATION_META_ORB) ||
		this->m_iID == VGUIManager::GetID(ID_BTN_DEFENSE_META_ORB) ||
		this->m_iID == VGUIManager::GetID(ID_BTN_ASSAULT_META_ORB))
	{
		NodeSphere* pickedNodeSphere = GameDialogManager::sharedInstance()->getHUD()->getNodeSphere();
		Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();

		int metaOrbID;
		if (this->m_iID == VGUIManager::GetID(ID_BTN_GENERATION_META_ORB))
			metaOrbID = VGUIManager::GetID(ID_META_ORB_GENERATION);
		else if (this->m_iID == VGUIManager::GetID(ID_BTN_DEFENSE_META_ORB))
			metaOrbID = VGUIManager::GetID(ID_META_ORB_DEFENSE);
		else if (this->m_iID == VGUIManager::GetID(ID_BTN_ASSAULT_META_ORB))
			metaOrbID = VGUIManager::GetID(ID_META_ORB_ASSAULT);

		if (pickedNodeSphere->getTeam() == defaultTeam) //try to generate a meta-orb
		{
			if (pickedNodeSphere->getOrbsCountForTeam(defaultTeam) > META_ORB_CREATION_COST)
			{
				pNodeSphere->createMetaOrb(metaOrbID, -META_ORB_CREATION_COST);
				pNodeSphere->updateAggregatingOrbs(metaOrbID, META_ORB_CREATION_COST);
				GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->showGauge(metaOrbID, true);
				GameDialogManager::sharedInstance()->getHUD()->removeMetaOrbButton(this->m_iID);
			}
			else
				MessageHandler::sharedInstance().showErrorMessage(StringsXmlHelper::sharedInstance().getStringForTag("meta_orb_creation_min_orbs"));
		}
		else //neutral or enemy sphere, try to take control of it by generating a meta-orb
		{
			if (GameManager::GlobalManager().getPickedNodeSphere()->takeControl(defaultTeam, false, false))
			{
				GameDialogManager::sharedInstance()->getHUD()->updateHUDForPickedNodeSphere();
				pNodeSphere->createMetaOrb(metaOrbID, -META_ORB_CREATION_COST);
				pNodeSphere->updateAggregatingOrbs(metaOrbID, META_ORB_CREATION_COST);
				GameDialogManager::sharedInstance()->getHUD()->getCorePanel()->showGauge(metaOrbID, true);
				GameDialogManager::sharedInstance()->getHUD()->removeMetaOrbButton(this->m_iID);
			}
		}
	}
	else if (m_iID == VGUIManager::GetID(ID_CORE_PANEL_GAUGE_VALIDATE) || m_iID == VGUIManager::GetID(ID_CORE_PANEL_GAUGE_BACK))
	{
		CorePanel* corePanel = GameDialogManager::sharedInstance()->getHUD()->getCorePanel();
		ScreenMaskPanel* contentHolder = (ScreenMaskPanel*) corePanel->findElementById(ID_CORE_PANEL_CONTENT_HOLDER);
		GaugeController* gaugeController = (GaugeController*) contentHolder->findElementById(ID_CORE_PANEL_GAUGE_CONTROLLER);
		gaugeController->setEnabled(false);

		if (m_iID == VGUIManager::GetID(ID_CORE_PANEL_GAUGE_VALIDATE))
		{
			MetaOrb* selectedMetaOrb = corePanel->getSelectedMetaOrb();

			int targetOrbsValue = selectedMetaOrb->getTargetOrbsValue() + gaugeController->getCurrentValue();
			selectedMetaOrb->setTargetOrbsValue(targetOrbsValue);

			GameManager::GlobalManager().getPickedNodeSphere()->updateAggregatingOrbs(selectedMetaOrb->getID(), gaugeController->getCurrentValue());
		}

		//int targetOrbsValue = (int) GeometryUtils::round((cursor->GetPosition().x / (220.0f * scale) + 0.5f) * maxOrbsValue);
		//this->parentMetaOrb->setTargetOrbsValue(targetOrbsValue);

		//int aggregatingOrbsCount = targetOrbsValue - this->parentMetaOrb->getCurrentOrbsValue();
		//GameManager::GlobalManager().getPickedNodeSphere()->updateAggregatingOrbs(this->parentMetaOrb->getID(), aggregatingOrbsCount);

		//fade out core panel content
		contentHolder->fadeTo(0.0f, 0.15f, 0.0f);

		CallFuncObject* clearContentHolderAction = new CallFuncObject(contentHolder, (CallFunc)(&ScreenMaskPanel::removeAllChildren), 0.15f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(clearContentHolderAction);
		clearContentHolderAction->executeFunc();

		//down the core panel's bg
		float scale = GameDialogManager::sharedInstance()->getScale();
		ScreenMaskPanel* corePanelBg = (ScreenMaskPanel*) corePanel->findElementById(ID_CORE_PANEL_BG);
		corePanelBg->translateTo(hkvVec2(0, 64.0f * scale), 0.2, 0.25f);

		//show all gauges/meta-orbs buttons
		CallFuncObject* showMainContentAction = new CallFuncObject(corePanel, (CallFunc)(&CorePanel::buildAndShowMainContent), 0.5f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(showMainContentAction);
		showMainContentAction->executeFunc();
	}

	////Artifact
	if (pNodeSphere->isArtifact())
	{
		Artifact* pArtifact = (Artifact*) pNodeSphere;
		if (this->m_iID == VGUIManager::GetID(ID_CORE_PANEL_BUTTON_ARTIFACT_ANNIHILATION))
		{

		}
		else if (this->m_iID == VGUIManager::GetID(ID_CORE_PANEL_BUTTON_ARTIFACT_REVELATION))
		{

		}
		else if (this->m_iID == VGUIManager::GetID(ID_CORE_PANEL_BUTTON_ARTIFACT_BERZERK))
		{

		}
	}
}

//////////////////////////////////Gauge//////////////////////////////////
V_IMPLEMENT_DYNCREATE(Gauge, ScreenMaskPanel, Vision::GetEngineModule());

void Gauge::init(hkvVec2 position, MetaOrb* parentMetaOrb)
{
	ScreenMaskPanel::init(position);
	this->m_pParentMetaOrb = parentMetaOrb;

	VColorRef gaugeFillColor;
	VColorRef gaugeFillTargetColor;
	if (parentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_GENERATION))
	{
		gaugeFillColor = V_RGBA_GREEN; //green
		gaugeFillTargetColor = VColorRef(0,128,0);
	}
	else if (parentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_DEFENSE))
	{
		gaugeFillColor = V_RGBA_BLUE; //blue
		gaugeFillTargetColor = VColorRef(0,0,128);
	}
	else if (parentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_ASSAULT))
	{
		gaugeFillColor = V_RGBA_RED; //red
		gaugeFillTargetColor = VColorRef(128,0,0);
	}

	//build gauge
	VTextureObject* pGaugeBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\gauge_bg.png");
	VTextureObject* pGaugeGridTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\gauge_grid.png");
	VTextureObject* pGaugeFillTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	float scale = GameDialogManager::sharedInstance()->getScale();
	float gaugeWidth = 256.0f * scale;
	float gaugeHeight = 64.0f * scale;
	hkvVec2 gaugeSize = hkvVec2(gaugeWidth, gaugeHeight);

	SpriteScreenMask* gaugeBg = new SpriteScreenMask();
	gaugeBg->AddRef();
	gaugeBg->init(hkvVec2(0,0), gaugeSize, pGaugeBgTexture);
	gaugeBg->SetOrder(CORE_PANEL_ORDER - 1);
	gaugeBg->SetColor(gaugeFillColor);
	this->addScreenMask(gaugeBg);

	m_pGaugeFill = new SpriteScreenMask();
	m_pGaugeFill->AddRef();
	m_pGaugeFill->init(hkvVec2(-110.0f * scale, 0), hkvVec2(0,0), pGaugeFillTexture);
	m_pGaugeFill->SetAnchorPoint(hkvVec2(0, 0.5f));
	m_pGaugeFill->SetColor(gaugeFillColor);
	m_pGaugeFill->SetOrder(CORE_PANEL_ORDER - 3);
	this->addScreenMask(m_pGaugeFill);

	m_pGaugeFillTarget = new SpriteScreenMask();
	m_pGaugeFillTarget->AddRef();
	m_pGaugeFillTarget->init(hkvVec2(-110.0f * scale, 0), hkvVec2(0,0), pGaugeFillTexture);
	m_pGaugeFillTarget->SetAnchorPoint(hkvVec2(0, 0.5f));
	m_pGaugeFillTarget->SetColor(gaugeFillTargetColor);
	m_pGaugeFillTarget->SetOrder(CORE_PANEL_ORDER - 2);
	this->addScreenMask(m_pGaugeFillTarget);

	SpriteScreenMask* pGaugeGrid = new SpriteScreenMask();
	pGaugeGrid->AddRef();
	pGaugeGrid->init(hkvVec2(0,0), gaugeSize, pGaugeGridTexture);
	//pGaugeGrid->SetColor(gaugeColor);
	pGaugeGrid->SetOrder(CORE_PANEL_ORDER - 4);
	this->addScreenMask(pGaugeGrid);

	//build cursor
	PrintTextLabel* pCursorDeltaTextLabel = new PrintTextLabel();
	pCursorDeltaTextLabel->AddRef();
	float fCursorDeltaFontHeight = 32 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fCursorDeltaFontHeight);
	VColorRef cursorDeltaFontColor = V_RGBA_WHITE;
	hkvVec2 cursorDeltaTextLabelPosition = hkvVec2((m_iTargetOrbsValue / (float) m_iMaxOrbsValue - 0.5f) * 220.0f * scale, -45.0f * scale);
	pCursorDeltaTextLabel->init(cursorDeltaTextLabelPosition, "+0", neuropolFont, fCursorDeltaFontHeight, cursorDeltaFontColor); 
	pCursorDeltaTextLabel->m_iID = VGUIManager::GetID(ID_GAUGE_CURSOR_DELTA);
	this->addTextLabel(pCursorDeltaTextLabel);
}

void Gauge::update(float dt)
{
	ScreenMaskPanel::update(dt);

	//update values
	updateValues();

	//fill
	updateFill();
}

void Gauge::updateValues()
{
	m_iCurrentOrbsValue = this->m_pParentMetaOrb->getCurrentOrbsValue();
	m_iTargetOrbsValue = this->m_pParentMetaOrb->getTargetOrbsValue();
	m_iMaxOrbsValue = this->m_pParentMetaOrb->getMaxOrbsValue();
	if (m_iCurrentOrbsValue == m_iMaxOrbsValue) //meta-orb is full
	{
		m_iMaxReachableOrbsValue = 0;
		return;
	}

	NodeSphere* parentNodeSphere = this->m_pParentMetaOrb->getParentNodeSphere();
	int iSphereTeamOrbsCount = parentNodeSphere->getOrbsCountForTeam(parentNodeSphere->getTeam());
	m_iMaxReachableOrbsValue = min(this->m_iCurrentOrbsValue + iSphereTeamOrbsCount, m_iMaxOrbsValue);
}

void Gauge::updateFill()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();

	float fPercentageFill = m_iCurrentOrbsValue / (float) m_iMaxOrbsValue;
	if (fPercentageFill < 0)
		fPercentageFill = 0;	

	hkvVec2 gaugeFillSize = hkvVec2(fPercentageFill * 220.0f * fScale, 37.0f * fScale);
	m_pGaugeFill->SetUnscaledTargetSize(gaugeFillSize);

	float fPercentageFillTarget;
	if (m_pController)
		fPercentageFillTarget = (m_iTargetOrbsValue + m_pController->getCurrentValue()) / (float) m_iMaxOrbsValue;
	else
		fPercentageFillTarget = m_iTargetOrbsValue / (float) m_iMaxOrbsValue;

	if (fPercentageFillTarget < 0)
	{
		fPercentageFillTarget = 0;
		hkvLog::Error("targetOrbsValue:%f maxOrbsValue:%f", m_iTargetOrbsValue, m_iMaxOrbsValue);
	}

	hkvVec2 gaugeFillTargetSize = hkvVec2(fPercentageFillTarget * 220.0f * fScale, 37.0f * fScale);
	m_pGaugeFillTarget->SetUnscaledTargetSize(gaugeFillTargetSize);

	if (fPercentageFillTarget >= 1)
	{
		CorePanel* corePanel = GameDialogManager::sharedInstance()->getHUD()->getCorePanel();
		GaugeUpdateButton* pUpdateBtn = NULL;
		if (m_pParentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_GENERATION))
			pUpdateBtn = (GaugeUpdateButton*) corePanel->findElementById(ID_GAUGE_UPDATE_BUTTON_GENERATION);
		else if (m_pParentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_DEFENSE))
			pUpdateBtn = (GaugeUpdateButton*) corePanel->findElementById(ID_GAUGE_UPDATE_BUTTON_DEFENSE);
		else if (m_pParentMetaOrb->getID() == VGUIManager::GetID(ID_META_ORB_ASSAULT))
			pUpdateBtn = (GaugeUpdateButton*) corePanel->findElementById(ID_GAUGE_UPDATE_BUTTON_ASSAULT);

		if (pUpdateBtn)
		{
			pUpdateBtn->SetOpacity(0);
			pUpdateBtn->disable();
		}
	}
}

//////////////////////////////////GaugeController//////////////////////////////////
V_IMPLEMENT_DYNCREATE(GaugeController, Slider, Vision::GetEngineModule());

void GaugeController::init(Gauge* gauge, hkvVec2 position)
{
	this->m_bEnabled = true;
	this->m_pParentGauge = gauge;
	this->m_pParentGauge->setController(this);

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 trackSize = hkvVec2(512.0f * scale, 32.0f * scale);
	hkvVec2 knobSize = hkvVec2(64.0f * scale, 64.0f * scale);

	VTextureObject* trackTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_bg.png");
	VTextureObject* knobTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_knob.png");

	Slider::init(true, position, trackSize, knobSize, trackTexture, knobTexture);

	//set correct order for slider bg
	this->sliderTrack->SetOrder(CORE_PANEL_ORDER - 1);

	//slider knob
	hkvVec2 sliderKnobPosition = hkvVec2(-0.5f * trackSize.x, 0); 
	this->sliderKnob->SetPosition(sliderKnobPosition);
	this->sliderKnob->SetOrder(CORE_PANEL_ORDER - 2);

	//text label displaying the controller current value
	PrintTextLabel* currentValueTextLabel = new PrintTextLabel();
	currentValueTextLabel->AddRef();
	hkvVec2 currentValueTextLabelPosition = hkvVec2(-25.0f * scale, -50.0f * scale);
	string currentValueText = string("+") + stringFromInt(this->m_iCurrentValue);
	float fontHeight = 42.0f * scale;
	VisFont_cl* font = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	currentValueTextLabel->init(currentValueTextLabelPosition, currentValueText, font, fontHeight, V_RGBA_WHITE);
	currentValueTextLabel->SetAnchorPoint(hkvVec2(0, 0.5f));
	currentValueTextLabel->m_iID = VGUIManager::GetID(ID_GAUGE_CURRENT_VALUE_TEXT_LABEL);
	this->addTextLabel(currentValueTextLabel);

	this->m_iID = VGUIManager::GetID(ID_CORE_PANEL_GAUGE_CONTROLLER);
}

bool GaugeController::onPointerMove(hkvVec2 touchLocation)
{
	return Slider::onPointerMove(touchLocation);
}

void GaugeController::updateCurrentValueTextLabel()
{
	if (this->m_bEnabled)
	{
		PrintTextLabel* currentValueTextLabel = (PrintTextLabel*) this->findElementById(ID_GAUGE_CURRENT_VALUE_TEXT_LABEL);
		string currentValueText = string("+") + stringFromInt(this->m_iCurrentValue);
		currentValueTextLabel->setText(currentValueText);
	}
}

void GaugeController::update(float dt)
{
	Slider::update(dt);

	int maxReachableOrbs = this->m_pParentGauge->getMaxReachableOrbsValue();
	int previousTargetOrbsValue = this->m_pParentGauge->getTargetOrbsValue();
	int amplitude = maxReachableOrbs - previousTargetOrbsValue;

	int oldValue = this->m_iCurrentValue;
	this->m_iCurrentValue = GeometryUtils::round(percentage * amplitude);

	if (oldValue != m_iCurrentValue)
		updateCurrentValueTextLabel();
}

//////////////////////////////////GaugeUpdateButton//////////////////////////////////
void GaugeUpdateButton::onClick()
{
	CorePanel* corePanel = GameDialogManager::sharedInstance()->getHUD()->getCorePanel();

	if (m_iID == VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_GENERATION))
		corePanel->setSelectedMetaOrb(GameManager::GlobalManager().getPickedNodeSphere()->getGenerationMetaOrb());
	else if (m_iID == VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_DEFENSE))
		corePanel->setSelectedMetaOrb(GameManager::GlobalManager().getPickedNodeSphere()->getDefenseMetaOrb());
	else if (m_iID == VGUIManager::GetID(ID_GAUGE_UPDATE_BUTTON_ASSAULT))
		corePanel->setSelectedMetaOrb(GameManager::GlobalManager().getPickedNodeSphere()->getAssaultMetaOrb());

	//fade out core panel content
	ScreenMaskPanel* contentHolder = (ScreenMaskPanel*) corePanel->findElementById(ID_CORE_PANEL_CONTENT_HOLDER);
	contentHolder->fadeTo(0.0f, 0.2f, 0.0f);

	CallFuncObject* clearContentHolderAction = new CallFuncObject(contentHolder, (CallFunc)(&ScreenMaskPanel::removeAllChildren), 0.15f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(clearContentHolderAction);
	clearContentHolderAction->executeFunc();

	//up the core panel's bg
	float scale = GameDialogManager::sharedInstance()->getScale();
	ScreenMaskPanel* corePanelBg = (ScreenMaskPanel*) corePanel->findElementById(ID_CORE_PANEL_BG);
	corePanelBg->translateTo(hkvVec2(0, 0), 0.25f, 0.2f);

	//show panel only with gauge
	CallFuncObject* showGaugeOnlyAction = new CallFuncObject(corePanel, (CallFunc)(&CorePanel::showGaugeUpgradePanel), 0.5f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showGaugeOnlyAction);
	showGaugeOnlyAction->executeFunc();
}

//////////////////////////////////MiscPanel//////////////////////////////////
MiscPanel::~MiscPanel()
{
	m_pTimer = NULL;
}

void MiscPanel::init(hkvVec2 position)
{
	ScreenMaskPanel::init(position);

	this->m_iDisplayedInfoID = -1;

	//build the sacrified orbs panel only if there is at least one sacrifice altar in the scene
	bool bSceneContainsSacrificeAltars = false;
	vector<NodeSphere*> &allNodeSpheres = GameManager::GlobalManager().getNodeSpheres();
	for (int iSphereIndex = 0; iSphereIndex != allNodeSpheres.size(); iSphereIndex++)
	{
		NodeSphere* pSphere = allNodeSpheres[iSphereIndex];
		if (pSphere->isSacrificePit())
		{
			bSceneContainsSacrificeAltars = true;
			break;
		}
	}

	if (bSceneContainsSacrificeAltars)
		this->buildSacrifiedOrbsCount();
}

void MiscPanel::buildCountdownTimerMessage(string strTitle, float fStartTimeInSecs)
{
	ScreenMaskPanel* countdownTimerMessagePanel = (ScreenMaskPanel*) this->findElementById(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
	if (countdownTimerMessagePanel) //we already built this
		return;

	float fScale = GameDialogManager::sharedInstance()->getScale();

	//timer object
	m_pTimer = new CountdownTimer();
	m_pTimer->init(fStartTimeInSecs);
	m_pTimer->start();

	//holder
	ScreenMaskPanel* pCountdownTimerMessageHolder = new ScreenMaskPanel();
	pCountdownTimerMessageHolder->AddRef();
	pCountdownTimerMessageHolder->init(hkvVec2(0, 0));
	pCountdownTimerMessageHolder->m_iID = VGUIManager::GetID(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
	this->addPanel(pCountdownTimerMessageHolder);

	//texts
	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);

	PrintTextLabel* pTitleTextLabel = new PrintTextLabel();
	pTitleTextLabel->AddRef();
	pTitleTextLabel->init(hkvVec2(0, -30.0f * fScale), strTitle, pFont, fFontHeight, V_RGBA_WHITE);
	pTitleTextLabel->setRenderedBehindVeils(true);
	pCountdownTimerMessageHolder->addTextLabel(pTitleTextLabel);

	PrintTextLabel* pTimerValueTextLabel = new PrintTextLabel();
	pTimerValueTextLabel->AddRef();
	pTimerValueTextLabel->init(hkvVec2(0, 18.0f * fScale), m_pTimer->getRemainingTimeAsString(), pFont, fFontHeight, V_RGBA_WHITE);
	pTimerValueTextLabel->setRenderedBehindVeils(true);
	pTimerValueTextLabel->m_iID = VGUIManager::GetID(ID_MISC_PANEL_COUNTDOWN_TIMER_VALUE);
	pCountdownTimerMessageHolder->addTextLabel(pTimerValueTextLabel);
}

void MiscPanel::buildSacrifiedOrbsCount()
{
	ScreenMaskPanel* sacrifiedOrbsCountPanel = (ScreenMaskPanel*) this->findElementById(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT);
	if (sacrifiedOrbsCountPanel) //we already built this
		return;

	float fScale = GameDialogManager::sharedInstance()->getScale();

	//holder
	ScreenMaskPanel* pSacrifiedOrbsCountHolder = new ScreenMaskPanel();
	pSacrifiedOrbsCountHolder->AddRef();
	pSacrifiedOrbsCountHolder->init(hkvVec2(0, -128.0f * fScale));
	pSacrifiedOrbsCountHolder->m_iID = VGUIManager::GetID(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT);
	this->addPanel(pSacrifiedOrbsCountHolder);

	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);

	//title
	PrintTextLabel* pTitleTextLabel = new PrintTextLabel();
	pTitleTextLabel->AddRef();
	string strSacrifiedOrbs = StringsXmlHelper::sharedInstance().getStringForTag("sacrified_orbs");
	pTitleTextLabel->init(hkvVec2(0, -40.0f * fScale), strSacrifiedOrbs, pFont, fFontHeight, V_RGBA_WHITE);
	pTitleTextLabel->setRenderedBehindVeils(true);
	pSacrifiedOrbsCountHolder->addTextLabel(pTitleTextLabel);

	//sacrified orbs count per team
	vector<Team*> allTeams = TeamManager::sharedInstance().getAllTeams();
	int iTeamsCount = (int) allTeams.size();
	iTeamsCount--; //we don't need to take the neutral team into account

	for (int iTeamIndex = 0; iTeamIndex != iTeamsCount ; iTeamIndex++)
	{
		Team* pTeam = allTeams[iTeamIndex];

		//create a label to display the sacrified orbs value
		MiscPanelSacrifiedOrbsLabel* pSacrifiedOrbsLabel = new MiscPanelSacrifiedOrbsLabel();
		pSacrifiedOrbsLabel->AddRef();
		float fYPosition = (iTeamsCount <= 3) ? 8.0f * fScale : ((iTeamIndex < 3) ? -6.0f * fScale : 31.0f * fScale);
		float fXPosition;
		if (iTeamIndex % 3 == 0)
			fXPosition = -209.0f * fScale;
		else if (iTeamIndex % 3 == 1)
			fXPosition = -8.0f * fScale;
		if (iTeamIndex % 3 == 2)
			fXPosition = 197.0f * fScale;
		pSacrifiedOrbsLabel->init(hkvVec2(fXPosition, fYPosition), pTeam);
		pSacrifiedOrbsCountHolder->addPanel(pSacrifiedOrbsLabel);
	}
}

void MiscPanel::buildAndStackNewObjectivesAlert()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();

	//holder
	MiscPanelAlert* pNewObjectivesAlertHolder = new MiscPanelAlert();
	pNewObjectivesAlertHolder->AddRef();
	pNewObjectivesAlertHolder->init(hkvVec2(0, -128.0f * fScale), hkvVec2(350.0f * fScale, 128.0f * fScale), hkvVec2(1.0f, 1.0f), 1.0f);
	pNewObjectivesAlertHolder->m_iID = VGUIManager::GetID(ID_MISC_PANEL_ALERT_NEW_OBJECTIVES);
	this->addPanel(pNewObjectivesAlertHolder);

	//texts
	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);

	PrintTextLabel* pTitleTextLabel = new PrintTextLabel();
	pTitleTextLabel->AddRef();
	string strTitle = StringsXmlHelper::sharedInstance().getStringForTag("new_objectives");
	pTitleTextLabel->init(hkvVec2(0, -30.0f * fScale), strTitle, pFont, fFontHeight, V_RGBA_WHITE);
	pTitleTextLabel->setRenderedBehindVeils(true);
	pNewObjectivesAlertHolder->addTextLabel(pTitleTextLabel);

	PrintTextLabel* pClickToSeeTextLabel = new PrintTextLabel();
	pClickToSeeTextLabel->AddRef();
	string strClickToSee = StringsXmlHelper::sharedInstance().getStringForTag("click_to_see");
	pClickToSeeTextLabel->init(hkvVec2(0, 18.0f * fScale), strClickToSee, pFont, fFontHeight, V_RGBA_WHITE);
	pClickToSeeTextLabel->setRenderedBehindVeils(true);
	pNewObjectivesAlertHolder->addTextLabel(pClickToSeeTextLabel);

	this->m_pendingAlerts.push_back(pNewObjectivesAlertHolder);

	if (!m_pCurrentAlert)
	{
		showNextAlert();
	}
}

void MiscPanel::buildAndStackAchievementUnlockedAlert(string strAchievementID)
{
	float fScale = GameDialogManager::sharedInstance()->getScale();

	//holder
	MiscPanelAlert* pAchievementAlertHolder = new MiscPanelAlert();
	pAchievementAlertHolder->AddRef();
	pAchievementAlertHolder->init(hkvVec2(0, -128.0f * fScale), hkvVec2(350.0f * fScale, 128.0f * fScale), hkvVec2(1.0f, 1.0f), 1.0f);
	pAchievementAlertHolder->m_iID = VGUIManager::GetID(ID_MISC_PANEL_ALERT_ACHIEVEMENT_UNLOCKED);
	this->addPanel(pAchievementAlertHolder);

	//texts
	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);

	PrintTextLabel* pTitleTextLabel = new PrintTextLabel();
	pTitleTextLabel->AddRef();
	string strTitle = "Achievement unlocked";
	pTitleTextLabel->init(hkvVec2(0, -30.0f * fScale), strTitle, pFont, fFontHeight, V_RGBA_WHITE);
	pTitleTextLabel->setRenderedBehindVeils(true);
	pAchievementAlertHolder->addTextLabel(pTitleTextLabel);

	PrintTextLabel* pClickToSeeTextLabel = new PrintTextLabel();
	pClickToSeeTextLabel->AddRef();
	pClickToSeeTextLabel->init(hkvVec2(0, 18.0f * fScale), strAchievementID, pFont, fFontHeight, V_RGBA_WHITE);
	pClickToSeeTextLabel->setRenderedBehindVeils(true);
	pAchievementAlertHolder->addTextLabel(pClickToSeeTextLabel);

	this->m_pendingAlerts.push_back(pAchievementAlertHolder);

	if (!m_pCurrentAlert)
	{
		showNextAlert();
	}
}

//void MiscPanel::showCountdownTimerMessage()
//{
//	ScreenMaskPanel* pCountdownTimerMessageHolder = (ScreenMaskPanel*) this->findElementById(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
//	if (pCountdownTimerMessageHolder)
//	{
//		this->m_iDisplayedInfoID = VGUIManager::GetID(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
//
//		//pCountdownTimerMessageHolder->translateTo(hkvVec2(0,0), 0.2f, 0.0f);
//		pCountdownTimerMessageHolder->
//	}
//}
//
//void MiscPanel::showSacrifiedOrbsCount()
//{
//	this->m_iDisplayedInfoID = VGUIManager::GetID(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT);
//}
//
//void MiscPanel::showNewObjectivesAlert()
//{
//	this->m_iDisplayedInfoID = VGUIManager::GetID(ID_MISC_PANEL_NEW_OBJECTIVES_ALERT_HOLDER);
//}

void MiscPanel::showInfo(const char* pszInfoID)
{
	ScreenMaskPanel* pInfoHolder = (ScreenMaskPanel*) this->findElementById(pszInfoID);

	if (pInfoHolder)
	{
		if (this->m_iDisplayedInfoID > 0)
		{
			this->dismissCurrentDisplayedInfo(); //dismiss the previous info

			pInfoHolder->SetOpacity(0);
			pInfoHolder->fadeTo(1.0f, 0.2f, 0.3f);
		}
		else
			GameDialogManager::sharedInstance()->getHUD()->showMiscPanel();
		pInfoHolder->SetPosition(hkvVec2(0, 0));
		
		//set the id of the new displayed info
		this->m_iDisplayedInfoID = VGUIManager::GetID(pszInfoID);
	}
}

void MiscPanel::showNextAlert()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();
	MiscPanelAlert* pAlertHolder = m_pendingAlerts[0];

	pAlertHolder->SetPosition(hkvVec2(0, 0));
	if (this->m_iDisplayedInfoID > 0)
	{
		if (m_pCurrentAlert) //dismiss the previous alert
			dismissAndRemoveAlert(m_pCurrentAlert);
		else
			this->dismissCurrentDisplayedInfo(); //dismiss the previous info
		pAlertHolder->SetOpacity(0);
		pAlertHolder->fadeTo(1.0f, 0.2f, 0.3f);
	}
	else
	{
		if (!m_pCurrentAlert)
			GameDialogManager::sharedInstance()->getHUD()->showMiscPanel();
		else
		{
			dismissAndRemoveAlert(m_pCurrentAlert);
			pAlertHolder->SetOpacity(0);
			pAlertHolder->fadeTo(1.0f, 0.2f, 0.3f);
		}
	}

	m_pCurrentAlert = pAlertHolder;
}

void MiscPanel::dismissCurrentDisplayedInfo()
{
	ScreenMaskPanel* pCurrentDisplayedInfoHolder = (ScreenMaskPanel*) this->findElementById(VGUIManager::GetIDName(this->m_iDisplayedInfoID));	

	if (pCurrentDisplayedInfoHolder)
	{
		float fScale = GameDialogManager::sharedInstance()->getScale();

		pCurrentDisplayedInfoHolder->translateTo(hkvVec2(0.0f, -128.0f * fScale), 0.3f, 0.0f);
	}
}

void MiscPanel::dismissAndRemoveAlert(MiscPanelAlert* pAlert)
{
	float fScale = GameDialogManager::sharedInstance()->getScale();
	m_pCurrentAlert->translateTo(hkvVec2(0.0f, -128.0f * fScale), 0.3f, 0.0f);

	//release the view
	CallFuncObject* removeAlertAction = new CallFuncObject(pAlert, (CallFunc)(&MiscPanelAlert::removeSelf), 0.3f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeAlertAction);
	removeAlertAction->executeFunc();
}

void MiscPanel::update(float dt)
{
	ScreenMaskPanel::update(dt);

	if (this->m_pendingAlerts.size() > 0)
	{
		m_fPendingAlertElapsedTime += dt;
		if (m_fPendingAlertElapsedTime > MISC_PANEL_ALERT_DURATION)
		{
			m_pendingAlerts.erase(m_pendingAlerts.begin());
			m_fPendingAlertElapsedTime = 0;
			if (m_pendingAlerts.size() > 0)
				showNextAlert();
			else
			{
				if (m_iDisplayedInfoID < 0)
					GameDialogManager::sharedInstance()->getHUD()->dismissMiscPanel();
				else
				{
					showInfo(VGUIManager::GetIDName(m_iDisplayedInfoID));
					dismissAndRemoveAlert(m_pCurrentAlert);
				}

				m_pCurrentAlert = NULL;
			}
		}
	}
	else
	{
		//update countdown timer
		if (this->m_iDisplayedInfoID == VGUIManager::GetID(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE))
		{
			if (this->m_pTimer->getRemainingTime() <= 0) //timer has ended dismiss the misc panel
				GameDialogManager::sharedInstance()->getHUD()->dismissMiscPanel();
			else
			{
				string strRemainingTime = this->m_pTimer->getRemainingTimeAsString();

				ScreenMaskPanel* pCountdownTimerMessageHolder = (ScreenMaskPanel*) this->findElementById(ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE);
				PrintTextLabel* pRemainingTimeValueTextLabel = (PrintTextLabel*) pCountdownTimerMessageHolder->findElementById(ID_MISC_PANEL_COUNTDOWN_TIMER_VALUE);

				pRemainingTimeValueTextLabel->setText(strRemainingTime);
			}
		}

		//update sacrifiedOrbsCount
		if (this->m_iDisplayedInfoID == VGUIManager::GetID(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT))
		{
			ScreenMaskPanel* pSacrifiedOrbsCountHolder = (ScreenMaskPanel*) this->findElementById(ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT);
			vector<ScreenMaskPanel*> &childPanels = pSacrifiedOrbsCountHolder->getPanels();
			for (int iChildIndex = 0; iChildIndex != childPanels.size(); iChildIndex++)
			{
				MiscPanelSacrifiedOrbsLabel* pChild = (MiscPanelSacrifiedOrbsLabel*) childPanels[iChildIndex];
				pChild->updateOrbsCountValue();
			}
		}
	}
}

//////////////////////////////////MiscPanelAlert//////////////////////////////////
void MiscPanelAlert::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_MISC_PANEL_ALERT_NEW_OBJECTIVES))
		hkvLog::Error("CLICK TO SEE");
}

void MiscPanelAlert::removeSelf()
{
	this->parentPanel->removePanel(this);
}

//////////////////////////////////MiscPanelSacrifiedOrbsLabel//////////////////////////////////
void MiscPanelSacrifiedOrbsLabel::init(hkvVec2 position, Team* pTeam)
{
	ScreenMaskPanel::init(position);

	this->m_pTeam = pTeam;

	float fScale = GameDialogManager::sharedInstance()->getScale();

	//Hexagon icon
	VTextureObject* pHexagonTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\hexagon.png");
	SpriteScreenMask* pHexagonIcon = new SpriteScreenMask();
	pHexagonIcon->AddRef();
	pHexagonIcon->init(hkvVec2(-20.0f * fScale, 0.0f), hkvVec2(32.0f * fScale, 32.0f * fScale), pHexagonTexture); 
	pHexagonIcon->SetColor(pTeam->getColor());
	pHexagonIcon->SetOrder(MISC_PANEL_ORDER - 1);
	this->addScreenMask(pHexagonIcon);

	//Sacrified orbs count value
	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);
	PrintTextLabel* pOrbsCountTextLabel = new PrintTextLabel();
	pOrbsCountTextLabel->AddRef();
	pOrbsCountTextLabel->init(hkvVec2(6.0f * fScale, -2.0f * fScale), stringFromInt(0), pFont, fFontHeight, V_RGBA_WHITE);
	pOrbsCountTextLabel->SetAnchorPoint(hkvVec2(0.0f, 0.5f));
	pOrbsCountTextLabel->setRenderedBehindVeils(true);
	pOrbsCountTextLabel->m_iID = VGUIManager::GetID(ID_SACRIFIED_ORBS_COUNT_TEXT_LABEL);
	this->addTextLabel(pOrbsCountTextLabel);
}

void MiscPanelSacrifiedOrbsLabel::updateOrbsCountValue()
{
	PrintTextLabel* pOrbsCountTextLabel = (PrintTextLabel*) this->findElementById(ID_SACRIFIED_ORBS_COUNT_TEXT_LABEL);

	if (pOrbsCountTextLabel)
	{
		pOrbsCountTextLabel->setText(stringFromInt(this->m_pTeam->getSacrifiedOrbCount()));
	}
}

//////////////////////////////////InterfaceMenu//////////////////////////////////
InterfaceMenu::~InterfaceMenu()
{
	//black veil is not a child of the interface menu, remove it separately
	this->blackVeil = NULL;
}

void InterfaceMenu::init()
{	
	hkvVec2 screenSize = hkvVec2(0,0);
	if (MenuManager::sharedInstance()->isContextActive())
		screenSize = MenuManager::sharedInstance()->getScreenSize();
	else if (GameDialogManager::sharedInstance()->isContextActive())
		screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	ScreenMaskPanel::init(0.5f * screenSize);
}

void InterfaceMenu::show(hkvVec2 size, bool withVeil, bool autoClose)
{
	this->size = size;
	this->buildAndShowMainFrame(withVeil, autoClose);
	this->animateMainFrame(size);

	this->autoClose = autoClose;
	if (autoClose)
	{
		autoCloseElapsedTime = 0;
		autoCloseTime = INTERFACE_MENU_AUTO_CLOSE_TIME;
	}
}

void InterfaceMenu::buildAndShowMainFrame(bool withVeil, bool autoClose)
{
	float scale = 0;
	if (GameDialogManager::sharedInstance()->isContextActive())
		scale = GameDialogManager::sharedInstance()->getScale();
	else if (MenuManager::sharedInstance()->isContextActive())
		scale = MenuManager::sharedInstance()->getScale();

	//Textures
	VTextureObject* blankTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	//dimensions
	float smallHorizontalContourWidth = 40 * scale;
	float smallHorizontalContourHeight = 4 * scale;
	hkvVec2 smallHorizontalContourSize = hkvVec2(smallHorizontalContourWidth, smallHorizontalContourHeight);
	float smallVerticalContourWidth = 4 * scale;
	float smallVerticalContourHeight = 40 * scale;
	hkvVec2 smallVerticalContourSize = hkvVec2(smallVerticalContourWidth, smallVerticalContourHeight);
	float largeHorizontalContourWidth = size.x - 2 * smallHorizontalContourWidth;
	float largeHorizontalContourHeight = 4 * scale;
	hkvVec2 largeHorizontalContourSize = hkvVec2(largeHorizontalContourWidth, largeHorizontalContourHeight);
	float largeVerticalContourWidth = 4 * scale;
	float largeVerticalContourHeight = size.y - 2 * smallVerticalContourHeight;
	hkvVec2 largeVerticalContourSize = hkvVec2(largeVerticalContourWidth, largeVerticalContourHeight);
	float commonBtnWidth = 128 * scale;
	float commonBtnHeight = 128 * scale;
	hkvVec2 commonBtnSize = hkvVec2(commonBtnWidth, commonBtnHeight);

	//veil
	if (withVeil)
	{
		blackVeil = new Veil();
		blackVeil->init(V_RGBA_BLACK, 0.0f);
		blackVeil->SetOrder(INTERFACE_MENU_ORDER + 1);
		blackVeil->fadeTo(1.0f, 0.2f, 0.0f);
	}

	//background
	if (!withVeil)
	{
		SpriteScreenMask* bg = new SpriteScreenMask();
		bg->AddRef();
		bg->init(hkvVec2(0,0), size, blankTexture);
		bg->SetColor(V_RGBA_BLACK);
		bg->SetOrder(INTERFACE_MENU_ORDER);
		bg->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_BACKGROUND);
		this->addScreenMask(bg);
		bg->SetScale(hkvVec2(0,0));
	}

	//angles
	ScreenMaskPanel* topLeftAngle = new ScreenMaskPanel();
	topLeftAngle->AddRef();
	topLeftAngle->init(hkvVec2(0,0));
	SpriteScreenMask* horizontalContour = new SpriteScreenMask();
	horizontalContour->AddRef();
	hkvVec2 horizontalContourPosition = hkvVec2(0, -0.5f * (smallVerticalContourHeight - smallHorizontalContourHeight));
	horizontalContour->init(horizontalContourPosition, smallHorizontalContourSize, blankTexture);
	horizontalContour->SetColor(VColorRef(187,255,251));
	topLeftAngle->addScreenMask(horizontalContour);
	SpriteScreenMask* verticalContour = new SpriteScreenMask();
	verticalContour->AddRef();
	hkvVec2 verticalContourPosition = hkvVec2(-0.5f * (smallHorizontalContourWidth - smallVerticalContourWidth), 0);
	verticalContour->init(verticalContourPosition, smallVerticalContourSize, blankTexture);
	verticalContour->SetColor(VColorRef(187,255,251));
	topLeftAngle->addScreenMask(verticalContour);
	topLeftAngle->SetOrder(INTERFACE_MENU_ORDER - 1);
	topLeftAngle->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_TOP_LEFT_ANGLE);

	ScreenMaskPanel* topRightAngle = new ScreenMaskPanel();
	topRightAngle->AddRef();
	topRightAngle->init(hkvVec2(0,0));
	horizontalContour = new SpriteScreenMask();
	horizontalContour->AddRef();
	horizontalContourPosition = hkvVec2(0, -0.5f * (smallVerticalContourHeight - smallHorizontalContourHeight));
	horizontalContour->init(horizontalContourPosition, smallHorizontalContourSize, blankTexture);
	horizontalContour->SetColor(VColorRef(187,255,251));
	topRightAngle->addScreenMask(horizontalContour);
	verticalContour = new SpriteScreenMask();
	verticalContour->AddRef();
	verticalContourPosition = hkvVec2(0.5f * (smallHorizontalContourWidth - smallVerticalContourWidth), 0);
	verticalContour->init(verticalContourPosition, smallVerticalContourSize, blankTexture);
	verticalContour->SetColor(VColorRef(187,255,251));
	topRightAngle->addScreenMask(verticalContour);
	topRightAngle->SetOrder(INTERFACE_MENU_ORDER - 1);
	topRightAngle->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_TOP_RIGHT_ANGLE);

	ScreenMaskPanel* bottomRightAngle = new ScreenMaskPanel();
	bottomRightAngle->AddRef();
	bottomRightAngle->init(hkvVec2(0,0));
	horizontalContour = new SpriteScreenMask();
	horizontalContour->AddRef();
	horizontalContourPosition = hkvVec2(0, 0.5f * (smallVerticalContourHeight - smallHorizontalContourHeight));
	horizontalContour->init(horizontalContourPosition, smallHorizontalContourSize, blankTexture);
	horizontalContour->SetColor(VColorRef(187,255,251));
	bottomRightAngle->addScreenMask(horizontalContour);
	verticalContour = new SpriteScreenMask();
	verticalContour->AddRef();
	verticalContourPosition = hkvVec2(0.5f * (smallHorizontalContourWidth - smallVerticalContourWidth), 0);
	verticalContour->init(verticalContourPosition, smallVerticalContourSize, blankTexture);
	verticalContour->SetColor(VColorRef(187,255,251));
	bottomRightAngle->addScreenMask(verticalContour);
	bottomRightAngle->SetOrder(INTERFACE_MENU_ORDER - 1);
	bottomRightAngle->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_BOTTOM_RIGHT_ANGLE);

	ScreenMaskPanel* bottomLeftAngle = new ScreenMaskPanel();
	bottomLeftAngle->AddRef();
	bottomLeftAngle->init(hkvVec2(0,0));
	horizontalContour = new SpriteScreenMask();
	horizontalContour->AddRef();
	horizontalContourPosition = hkvVec2(0, 0.5f * (smallVerticalContourHeight - smallHorizontalContourHeight));
	horizontalContour->init(horizontalContourPosition, smallHorizontalContourSize, blankTexture);
	horizontalContour->SetColor(VColorRef(187,255,251));
	bottomLeftAngle->addScreenMask(horizontalContour);
	verticalContour = new SpriteScreenMask();
	verticalContour->AddRef();
	verticalContourPosition = hkvVec2(-0.5f * (smallHorizontalContourWidth - smallVerticalContourWidth), 0);
	verticalContour->init(verticalContourPosition, smallVerticalContourSize, blankTexture);
	verticalContour->SetColor(VColorRef(187,255,251));
	bottomLeftAngle->addScreenMask(verticalContour);
	bottomLeftAngle->SetOrder(INTERFACE_MENU_ORDER - 1);
	bottomLeftAngle->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_BOTTOM_LEFT_ANGLE);

	this->addPanel(topLeftAngle);
	this->addPanel(topRightAngle);
	this->addPanel(bottomLeftAngle);
	this->addPanel(bottomRightAngle);

	//contours
	if (!withVeil)
	{
		SpriteScreenMask* topContour = new SpriteScreenMask();
		topContour->AddRef();
		topContour->init(hkvVec2(0,0), largeHorizontalContourSize, blankTexture);
		topContour->SetOrder(INTERFACE_MENU_ORDER - 1);
		topContour->SetColor(VColorRef(13,34,35));
		topContour->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_TOP_CONTOUR);
		this->addScreenMask(topContour);

		SpriteScreenMask* rightContour = new SpriteScreenMask();
		rightContour->AddRef();
		rightContour->init(hkvVec2(0,0), largeVerticalContourSize, blankTexture);
		rightContour->SetOrder(INTERFACE_MENU_ORDER - 1);
		rightContour->SetColor(VColorRef(13,34,35));
		rightContour->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_RIGHT_CONTOUR);
		this->addScreenMask(rightContour);

		SpriteScreenMask* bottomContour = new SpriteScreenMask();
		bottomContour->AddRef();
		bottomContour->init(hkvVec2(0,0), largeHorizontalContourSize, blankTexture);
		bottomContour->SetOrder(INTERFACE_MENU_ORDER - 1);
		bottomContour->SetColor(VColorRef(13,34,35));
		bottomContour->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_BOTTOM_CONTOUR);
		this->addScreenMask(bottomContour);

		SpriteScreenMask* leftContour = new SpriteScreenMask();
		leftContour->AddRef();
		leftContour->init(hkvVec2(0,0), largeVerticalContourSize, blankTexture);
		leftContour->SetOrder(INTERFACE_MENU_ORDER - 1);
		leftContour->SetColor(VColorRef(13,34,35));
		leftContour->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_LEFT_CONTOUR);
		this->addScreenMask(leftContour);
	}
}

void InterfaceMenu::animateMainFrame(hkvVec2 finalBox)
{
	float scale = 0;
	if (GameDialogManager::sharedInstance()->isContextActive())
		scale = GameDialogManager::sharedInstance()->getScale();
	else if (MenuManager::sharedInstance()->isContextActive())
		scale = MenuManager::sharedInstance()->getScale();

	//scale the background
	SpriteScreenMask* bg = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_BACKGROUND);
	if (bg)
	{
		bg->SetUnscaledTargetSize(finalBox);
		bg->scaleTo(hkvVec2(1.0f, 1.0f), 0.5f, 0.0f);
	}

	//move angles
	SpriteScreenMask* topLeftAngle = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_TOP_LEFT_ANGLE);
	SpriteScreenMask* topRightAngle = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_TOP_RIGHT_ANGLE);
	SpriteScreenMask* bottomLeftAngle = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_BOTTOM_LEFT_ANGLE);
	SpriteScreenMask* bottomRightAngle = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_BOTTOM_RIGHT_ANGLE);

	float smallHorizontalContourWidth = 40 * scale;
	float smallHorizontalContourHeight = 4 * scale;
	float smallVerticalContourWidth = 4 * scale;
	float smallVerticalContourHeight = 40 * scale;

	topLeftAngle->translateTo(hkvVec2(-0.5f * (finalBox.x - smallHorizontalContourWidth), -0.5f * (finalBox.y - smallVerticalContourHeight)), 0.5f, 0.0f); 
	topRightAngle->translateTo(hkvVec2(0.5f * (finalBox.x - smallHorizontalContourWidth), -0.5f * (finalBox.y - smallVerticalContourHeight)), 0.5f, 0.0f); 
	bottomLeftAngle->translateTo(hkvVec2(-0.5f * (finalBox.x - smallHorizontalContourWidth), 0.5f * (finalBox.y - smallVerticalContourHeight)), 0.5f, 0.0f); 
	bottomRightAngle->translateTo(hkvVec2(0.5f * (finalBox.x - smallHorizontalContourWidth), 0.5f * (finalBox.y - smallVerticalContourHeight)), 0.5f, 0.0f);

	//animate contours
	float largeHorizontalContourWidth = size.x - 2 * smallHorizontalContourWidth;
	float largeHorizontalContourHeight = 4.0f * scale;
	float largeVerticalContourWidth = 4.0f * scale;
	float largeVerticalContourHeight = size.y - 2 * smallVerticalContourHeight;

	hkvVec2 topContourPosition = hkvVec2(0, -0.5f * size.y + 0.5f * largeHorizontalContourHeight);
	hkvVec2 rightContourPosition = hkvVec2(0.5f * size.x - 0.5f * largeVerticalContourWidth, 0);
	hkvVec2 bottomContourPosition = hkvVec2(0, 0.5f * size.y - 0.5f * largeHorizontalContourHeight);
	hkvVec2 leftContourPosition = hkvVec2(-0.5f * size.x + 0.5f * largeVerticalContourWidth, 0);

	SpriteScreenMask* topContour = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_TOP_CONTOUR);
	if (topContour)
	{
		SpriteScreenMask* rightContour = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_RIGHT_CONTOUR);
		SpriteScreenMask* bottomContour = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_BOTTOM_CONTOUR);
		SpriteScreenMask* leftContour = (SpriteScreenMask*) this->findElementById(ID_INTERFACE_MENU_LEFT_CONTOUR);

		topContour->translateTo(topContourPosition, 0.5f, 0.0f);
		rightContour->translateTo(rightContourPosition, 0.5f, 0.0f);
		bottomContour->translateTo(bottomContourPosition, 0.5f, 0.0f);
		leftContour->translateTo(leftContourPosition, 0.5f, 0.0f);

		topContour->SetScale(hkvVec2(0.0f, 1.0f));
		rightContour->SetScale(hkvVec2(1.0f, 0.0f));
		bottomContour->SetScale(hkvVec2(0.0f, 1.0f));
		leftContour->SetScale(hkvVec2(1.0f, 0.0f));

		topContour->scaleTo(hkvVec2(1.0f, 1.0f), 0.5f, 0.0f);
		rightContour->scaleTo(hkvVec2(1.0f, 1.0f), 0.5f, 0.0f);
		bottomContour->scaleTo(hkvVec2(1.0f, 1.0f), 0.5f, 0.0f);
		leftContour->scaleTo(hkvVec2(1.0f, 1.0f), 0.5f, 0.0f);
	}

	//Close button
	InterfaceCloseButton* closeBtn = (InterfaceCloseButton*) this->findElementById(ID_INTERFACE_MENU_CLOSE_BUTTON);
	if (!closeBtn)
	{
		VTextureObject* closeBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\close_btn.png");
		closeBtn = new InterfaceCloseButton();
		closeBtn->AddRef();
		closeBtn->init(hkvVec2(0,0));
		closeBtn->SetOrder(INTERFACE_MENU_ORDER - 1);
		closeBtn->m_iID = VGUIManager::GetID(ID_INTERFACE_MENU_CLOSE_BUTTON);
		this->addScreenMask(closeBtn);
	}

	//fade in close button
	closeBtn->SetOpacity(0);
	closeBtn->SetPosition(hkvVec2(0.5f * finalBox.x - 60.0f * scale, -0.5f * finalBox.y + 60.0f * scale));
	closeBtn->fadeTo(1.0f, 0.3f, 0.5f);
}

void InterfaceMenu::dismiss()
{
	if (blackVeil)
		blackVeil->fadeTo(0.0f, 0.2f, 0.2f);

	this->fadeTo(0.0f, 0.2f, 0.0f);
}

void InterfaceMenu::update(float dt)
{
	ScreenMaskPanel::update(dt);

	if (this->autoClose)
	{
		autoCloseElapsedTime += dt;
		if (autoCloseElapsedTime > autoCloseTime)
		{
			this->autoClose = false;
			GameTimer::sharedInstance().setPaused(false);
			GameDialogManager::sharedInstance()->getHUD()->dismissInterfaceMenu();
		}
	}
}

//////////////////////////////////PauseMenu//////////////////////////////////
void PauseMenu::init()
{
	InterfaceMenu::init();
}

void PauseMenu::show(bool withVeil, bool autoClose)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 pauseMenuSize = hkvVec2(1024.0f * scale, 600.0f * scale);
	InterfaceMenu::show(pauseMenuSize, withVeil, autoClose);

	buildMainContent();
	showMainContent(false);
}

void PauseMenu::buildMainContent()
{
	mainContentHolder = new ScreenMaskPanel();
	mainContentHolder->AddRef();
	mainContentHolder->init(hkvVec2(0,0));
	this->addPanel(mainContentHolder);

	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();	

	//Textures
	VTextureObject* saveBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\overlay_save.png");
	VTextureObject* retryBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_reset.png");
	VTextureObject* menuBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_credits.png");

	//dimensions
	float commonBtnWidth = 128 * scale;
	float commonBtnHeight = 128 * scale;
	hkvVec2 commonBtnSize = hkvVec2(commonBtnWidth, commonBtnHeight);

	//Buttons
	PauseMenuButton* saveBtn = new PauseMenuButton();
	saveBtn->AddRef();
	hkvVec2 saveBtnPosition = hkvVec2(-280.0f * scale, -150.0f * scale);
	saveBtn->init(saveBtnPosition, commonBtnSize, saveBtnTexture);
	saveBtn->m_iID = VGUIManager::GetID(ID_PAUSE_MENU_BTN_SAVE);
	mainContentHolder->addScreenMask(saveBtn);

	PauseMenuButton* retryBtn = new PauseMenuButton();
	retryBtn->AddRef();
	hkvVec2 retryBtnPosition = hkvVec2(0, -150.0f * scale);
	retryBtn->init(retryBtnPosition, commonBtnSize, retryBtnTexture);
	retryBtn->m_iID = VGUIManager::GetID(ID_PAUSE_MENU_BTN_RETRY);
	mainContentHolder->addScreenMask(retryBtn);

	PauseMenuButton* menuBtn = new PauseMenuButton();
	menuBtn->AddRef();
	hkvVec2 menuBtnPosition = hkvVec2(280.0f * scale, -150.0f * scale);
	menuBtn->init(menuBtnPosition, commonBtnSize, menuBtnTexture);
	menuBtn->m_iID = VGUIManager::GetID(ID_PAUSE_MENU_BTN_MENU);
	mainContentHolder->addScreenMask(menuBtn);

	PauseMenuButton* musicBtn = new PauseMenuButton();
	musicBtn->AddRef();
	hkvVec2 musicBtnPosition = hkvVec2(-140.0f * scale, 30.0f * scale);
	bool musicActive = PersistentDataManager::sharedInstance().isMusicActive();
	const char* texturePath = musicActive ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png";
	VTextureObject* musicBtnTexture = Vision::TextureManager.Load2DTexture(texturePath);
	musicBtn->init(musicBtnPosition, commonBtnSize, musicBtnTexture);
	musicBtn->m_iID = VGUIManager::GetID(ID_PAUSE_MENU_BTN_MUSIC);
	mainContentHolder->addScreenMask(musicBtn);

	PauseMenuButton* soundBtn = new PauseMenuButton();
	soundBtn->AddRef();
	hkvVec2 soundBtnPosition = hkvVec2(140.0f * scale, 30.0f * scale);
	bool soundActive = PersistentDataManager::sharedInstance().isMusicActive();
	texturePath = soundActive ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png";
	VTextureObject* soundBtnTexture = Vision::TextureManager.Load2DTexture(texturePath);
	soundBtn->init(soundBtnPosition, commonBtnSize, soundBtnTexture);
	soundBtn->m_iID = VGUIManager::GetID(ID_PAUSE_MENU_BTN_SOUND);
	mainContentHolder->addScreenMask(soundBtn);

	//game speed slider
	GameSpeedSlider* gameSpeedSlider = new GameSpeedSlider();
	gameSpeedSlider->AddRef();
	hkvVec2 gameSpeedSliderPosition = hkvVec2(0, 200.0f * scale);
	gameSpeedSlider->init(gameSpeedSliderPosition);
	mainContentHolder->addPanel(gameSpeedSlider);

	//texts
	float fontHeight = 35 *scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	VColorRef fontColor = VColorRef(183, 255, 247);
	string saveText = StringsXmlHelper::sharedInstance().getStringForTag("pause_menu_save");
	string retryText = StringsXmlHelper::sharedInstance().getStringForTag("pause_menu_restart");
	string menuText = StringsXmlHelper::sharedInstance().getStringForTag("pause_menu_menu");
	string musicText = StringsXmlHelper::sharedInstance().getStringForTag("options_music");
	string soundText = StringsXmlHelper::sharedInstance().getStringForTag("options_sound");

	PrintTextLabel* saveTextLabel = new PrintTextLabel();
	saveTextLabel->AddRef();
	hkvVec2 saveTextPosition = saveBtnPosition + hkvVec2(0, 0.5f * commonBtnHeight);
	saveTextLabel->init(saveTextPosition, saveText, neuropolFont, fontHeight, fontColor);
	saveTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.0f));
	mainContentHolder->addTextLabel(saveTextLabel);

	PrintTextLabel* retryTextLabel = new PrintTextLabel();
	retryTextLabel->AddRef();
	hkvVec2 retryTextPosition = retryBtnPosition + hkvVec2(0, 0.5f * commonBtnHeight);
	retryTextLabel->init(retryTextPosition, retryText, neuropolFont, fontHeight, fontColor);
	retryTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.0f));
	mainContentHolder->addTextLabel(retryTextLabel);

	PrintTextLabel* menuTextLabel = new PrintTextLabel();
	menuTextLabel->AddRef();
	hkvVec2 menuTextPosition = menuBtnPosition + hkvVec2(0, 0.5f * commonBtnHeight);
	menuTextLabel->init(menuTextPosition, menuText, neuropolFont, fontHeight, fontColor);
	menuTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.0f));
	mainContentHolder->addTextLabel(menuTextLabel);

	PrintTextLabel* musicTextLabel = new PrintTextLabel();
	musicTextLabel->AddRef();
	hkvVec2 musicTextPosition = musicBtnPosition + hkvVec2(0, 0.5f * commonBtnHeight);
	musicTextLabel->init(musicTextPosition, musicText, neuropolFont, fontHeight, fontColor);
	musicTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.0f));
	mainContentHolder->addTextLabel(musicTextLabel);

	PrintTextLabel* soundTextLabel = new PrintTextLabel();
	soundTextLabel->AddRef();
	hkvVec2 soundTextPosition = soundBtnPosition + hkvVec2(0, 0.5f * commonBtnHeight);
	soundTextLabel->init(soundTextPosition, soundText, neuropolFont, fontHeight, fontColor);
	soundTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.0f));
	mainContentHolder->addTextLabel(soundTextLabel);

	mainContentHolder->SetOrder(INTERFACE_MENU_ORDER - 1);
	mainContentHolder->SetOpacity(0);
}


void PauseMenu::buildAndShowSecondaryContent(int clickedBtnID)
{
	if (clickedBtnID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_MENU))
	{
		secondaryContentHolder = new QuitDialogPanel();
		secondaryContentHolder->AddRef();
		((QuitDialogPanel*)secondaryContentHolder)->init();

	}
	else if (clickedBtnID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_RETRY))	
	{
		secondaryContentHolder = new RetryDialogPanel();
		secondaryContentHolder->AddRef();
		((RetryDialogPanel*)secondaryContentHolder)->init();
	}

	secondaryContentHolder->SetOrder(INTERFACE_MENU_ORDER - 1);

	this->addPanel(secondaryContentHolder);
	secondaryContentHolder->SetOpacity(0);
	secondaryContentHolder->fadeTo(1.0f, 0.3f, 0.5f);

	//animate the angles
	float scale = GameDialogManager::sharedInstance()->getScale();
	float bgWidth = 768 * scale;
	float bgHeight = 450 * scale;
	float smallHorizontalContourWidth = 40 * scale;
	float smallHorizontalContourHeight = 4 * scale;
	float smallVerticalContourWidth = 4 * scale;
	float smallVerticalContourHeight = 40 * scale;
	hkvVec2 finalBox = hkvVec2(bgWidth - smallHorizontalContourWidth, bgHeight - smallVerticalContourHeight);
	animateMainFrame(finalBox);
}

void PauseMenu::showMainContent(bool bAnimateMainFrame)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	mainContentHolder->SetOpacity(0);
	mainContentHolder->fadeTo(1.0f, 0.3f, 0.5f);
	mainContentHolder->SetPosition(hkvVec2(0,0));

	if (bAnimateMainFrame)
	{
		//animate the angles
		float bgWidth = 1024 * scale;
		float bgHeight = 600 * scale;
		float smallHorizontalContourWidth = 40 * scale;
		float smallHorizontalContourHeight = 4 * scale;
		float smallVerticalContourWidth = 4 * scale;
		float smallVerticalContourHeight = 40 * scale;
		hkvVec2 finalBox = hkvVec2(bgWidth, bgHeight);
		animateMainFrame(finalBox);
	}
}

void PauseMenu::dismissMainContent()
{
	mainContentHolder->SetOpacity(0);
	mainContentHolder->SetPosition(hkvVec2(0,3000.0f));
}

void PauseMenu::dismissSecondaryContent()
{
	this->removePanel(secondaryContentHolder);
	secondaryContentHolder = NULL;
}

//////////////////////////////////HelpMenu//////////////////////////////////
void HelpMenu::init()
{
	InterfaceMenu::init();
}

void HelpMenu::show(bool withVeil, bool autoClose)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 helpMenuSize = hkvVec2(1036.0f * scale, 750.0f * scale);
	InterfaceMenu::show(helpMenuSize, withVeil, autoClose);

	//buildPages
	buildPage1(); //controls
	buildPage2(); //meta-orbs
	buildPage3(); //artifacts

	//build arrows
	buildArrows();

	////show page
	int iLastPageReached = PersistentDataManager::sharedInstance().getHelpMenuLastReachedPage();

	showPage(iLastPageReached);
	this->m_iCurrentPageNumber = iLastPageReached;

	//show arrows
	HelpMenuButton* pLeftArrowBtn = (HelpMenuButton*) this->findElementById(ID_HELP_MENU_ARROW_LEFT);
	HelpMenuButton* pRightArrowBtn = (HelpMenuButton*) this->findElementById(ID_HELP_MENU_ARROW_RIGHT);
	pLeftArrowBtn->updateState(iLastPageReached);
	pRightArrowBtn->updateState(iLastPageReached);
	float fLeftArrowTargetOpacity = pLeftArrowBtn->GetOpacity();
	float pRightArrowTargetOpacity = pRightArrowBtn->GetOpacity();
	pLeftArrowBtn->SetOpacity(0);
	pRightArrowBtn->SetOpacity(0);
	pLeftArrowBtn->fadeTo(fLeftArrowTargetOpacity, 0.3f, 0.5f);
	pRightArrowBtn->fadeTo(pRightArrowTargetOpacity, 0.3f, 0.5f);
}

void HelpMenu::buildArrows()
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	VTextureObject* blankTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	//arrows
	VTextureObject* arrowTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\help_menu\\help_menu_arrow.png");
	hkvVec2 arrowBtnSize = hkvVec2(64.0f * scale, 64.0f * scale);

	HelpMenuButton* leftArrowBtn = new HelpMenuButton();
	leftArrowBtn->AddRef();
	hkvVec2 leftArrowBtnPosition = hkvVec2(-200.0f * scale, -0.5f * size.y + 50.0f * scale);
	leftArrowBtn->init(leftArrowBtnPosition, arrowBtnSize, arrowTexture);
	leftArrowBtn->m_iID = VGUIManager::GetID(ID_HELP_MENU_ARROW_LEFT);
	this->addScreenMask(leftArrowBtn);

	HelpMenuButton* rightArrowBtn = new HelpMenuButton();
	rightArrowBtn->AddRef();
	hkvVec2 rightArrowBtnPosition = hkvVec2(200.0f * scale, -0.5f * size.y + 50.0f * scale);
	rightArrowBtn->init(rightArrowBtnPosition, arrowBtnSize, arrowTexture);
	rightArrowBtn->SetAngle(180);
	rightArrowBtn->m_iID = VGUIManager::GetID(ID_HELP_MENU_ARROW_RIGHT);
	this->addScreenMask(rightArrowBtn);

	leftArrowBtn->SetOpacity(0);
	rightArrowBtn->SetOpacity(0);
}

void HelpMenu::buildPage1()
{
	ScreenMaskPanel* page1 = new ScreenMaskPanel();
	page1->AddRef();
	page1->init(hkvVec2(0,0));

	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* blankTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	VTextureObject* iconTranslateTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\help_menu\\icon_translate.png");
	VTextureObject* iconZoomTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\help_menu\\icon_zoom.png");
	VTextureObject* iconRotateTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\help_menu\\icon_rotate.png");

	hkvVec2 iconSize = hkvVec2(128.0f * scale, 128.0f * scale);
	float fontHeight = 40.0f * scale;
	VisFont_cl* font = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	VColorRef fontColor = VColorRef(180,255,255);

	//title
	PrintTextLabel* title = new PrintTextLabel();
	title->AddRef();
	hkvVec2 titlePosition = hkvVec2(0, -0.5f * size.y + 45.0f * scale);
	string titleText = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_controls");
	title->init(titlePosition, titleText, font, fontHeight, fontColor);
	page1->addTextLabel(title);

	//translate
	SpriteScreenMask* translateIcon = new SpriteScreenMask();
	translateIcon->AddRef();
	hkvVec2 translateIconPosition = hkvVec2(-367.0f * scale, -217.0f * scale);
	translateIcon->init(translateIconPosition, iconSize, iconTranslateTexture);
	translateIcon->SetOrder(INTERFACE_MENU_ORDER - 1);
	page1->addScreenMask(translateIcon);

	PrintTextLabel* translateTextLabel = new PrintTextLabel();
	translateTextLabel->AddRef();
	hkvVec2 translateTextLabelPosition = hkvVec2(-202.0f * scale, -217.0f * scale);
	string translateText = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_translate");
	string translateTextWithLineSeparators;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(translateTextWithLineSeparators, translateText, 40);
	translateTextLabel->init(translateTextLabelPosition, translateTextWithLineSeparators, font, fontHeight, fontColor);
	translateTextLabel->SetAnchorPoint(hkvVec2(0,0.5f));
	page1->addTextLabel(translateTextLabel);

	//zoom
	SpriteScreenMask* zoomIcon = new SpriteScreenMask();
	zoomIcon->AddRef();
	hkvVec2 zoomIconPosition = hkvVec2(-367.0f * scale, 0);
	zoomIcon->init(zoomIconPosition, iconSize, iconZoomTexture);
	zoomIcon->SetOrder(INTERFACE_MENU_ORDER - 1);
	page1->addScreenMask(zoomIcon);

	PrintTextLabel* zoomTextLabel = new PrintTextLabel();
	zoomTextLabel->AddRef();
	hkvVec2 zoomTextLabelPosition = hkvVec2(-202.0f * scale, 0);
	string zoomText = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_zoom");
	string zoomTextWithLineSeparators;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(zoomTextWithLineSeparators, zoomText, 40);
	zoomTextLabel->init(zoomTextLabelPosition, zoomTextWithLineSeparators, font, fontHeight, fontColor);
	zoomTextLabel->SetAnchorPoint(hkvVec2(0,0.5f));
	page1->addTextLabel(zoomTextLabel);

	//rotate
	SpriteScreenMask* rotateIcon = new SpriteScreenMask();
	rotateIcon->AddRef();
	hkvVec2 rotateIconPosition = hkvVec2(-367.0f * scale, 217.0f * scale);
	rotateIcon->init(rotateIconPosition, iconSize, iconRotateTexture);
	rotateIcon->SetOrder(INTERFACE_MENU_ORDER - 1);
	page1->addScreenMask(rotateIcon);

	PrintTextLabel* rotateTextLabel = new PrintTextLabel();
	rotateTextLabel->AddRef();
	hkvVec2 rotateTextLabelPosition = hkvVec2(-202.0f * scale, 217.0f * scale);
	string rotateText = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_rotate");
	string rotateTextWithLineSeparators;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(rotateTextWithLineSeparators, rotateText, 40);
	rotateTextLabel->init(rotateTextLabelPosition, rotateTextWithLineSeparators, font, fontHeight, fontColor);
	rotateTextLabel->SetAnchorPoint(hkvVec2(0,0.5f));
	page1->addTextLabel(rotateTextLabel);

	//separations
	hkvVec2 separationSize = hkvVec2(900.0f * scale, 4.0f * scale);

	SpriteScreenMask* separation1 = new SpriteScreenMask();
	separation1->AddRef();
	hkvVec2 separation1Position = hkvVec2(0, -108.0f * scale);
	separation1->init(separation1Position, separationSize, blankTexture);
	separation1->SetColor(fontColor);
	separation1->SetOrder(INTERFACE_MENU_ORDER - 1);
	page1->addScreenMask(separation1);

	SpriteScreenMask* separation2 = new SpriteScreenMask();
	separation2->AddRef();
	hkvVec2 separation2Position = hkvVec2(0, 108.0f * scale);
	separation2->init(separation2Position, separationSize, blankTexture);
	separation2->SetColor(fontColor);
	separation2->SetOrder(INTERFACE_MENU_ORDER - 1);
	page1->addScreenMask(separation2);

	this->addPanel(page1);
	page1->SetOpacity(0);
	m_pages.push_back(page1);
}

void HelpMenu::buildPage2()
{
	float fScale = GameDialogManager::sharedInstance()->getScale();

	ScreenMaskPanel* pPage2 = new ScreenMaskPanel();
	pPage2->AddRef();
	pPage2->init(hkvVec2(0,0));

	float fFontHeight = 40.0f * fScale;
	VisFont_cl* pFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fFontHeight);
	VColorRef fontColor = VColorRef(180,255,255);

	//title
	PrintTextLabel* pTitle = new PrintTextLabel();
	pTitle->AddRef();
	hkvVec2 titlePosition = hkvVec2(0, -0.5f * size.y + 45.0f * fScale);
	string strTitleText = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_meta_orbs");
	pTitle->init(titlePosition, strTitleText, pFont, fFontHeight, fontColor);
	pPage2->addTextLabel(pTitle);

	//meta-orbs icons
	VTextureObject* pGenerationMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\generation_meta_orb.png");
	VTextureObject* pDefenseMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\defense_meta_orb.png");
	VTextureObject* pAssaultMetaOrbTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\assault_meta_orb.png");
	hkvVec2 iconSize = hkvVec2(200.0f * fScale, 200.0f * fScale);

	SpriteScreenMask* pGenerationMetaOrbIcon = new SpriteScreenMask();
	pGenerationMetaOrbIcon->AddRef();
	hkvVec2 generationMetaOrbIconPosition = hkvVec2(-344.0f * fScale, -210.0f * fScale);
	pGenerationMetaOrbIcon->init(generationMetaOrbIconPosition, iconSize, pGenerationMetaOrbTexture);
	pPage2->addScreenMask(pGenerationMetaOrbIcon);

	SpriteScreenMask* pDefenseMetaOrbIcon = new SpriteScreenMask();
	pDefenseMetaOrbIcon->AddRef();
	hkvVec2 defenseMetaOrbIconPosition = hkvVec2(-344.0f * fScale, 0);
	pDefenseMetaOrbIcon->init(defenseMetaOrbIconPosition, iconSize, pDefenseMetaOrbTexture);
	pPage2->addScreenMask(pDefenseMetaOrbIcon);

	SpriteScreenMask* pAssaultMetaOrbIcon = new SpriteScreenMask();
	pAssaultMetaOrbIcon->AddRef();
	hkvVec2 assaultMetaOrbIconPosition = hkvVec2(-344.0f * fScale, 210.0f * fScale);
	pAssaultMetaOrbIcon->init(assaultMetaOrbIconPosition, iconSize, pAssaultMetaOrbTexture);
	pPage2->addScreenMask(pAssaultMetaOrbIcon);

	this->addPanel(pPage2);
	pPage2->SetOpacity(0);
	m_pages.push_back(pPage2);

	//texts
	float fTitleFontHeight = 44.0f * fScale;
	float fTextFontHeight = 40.0f * fScale;
	VisFont_cl* pTitleFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fTitleFontHeight);
	VisFont_cl* pTextFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fTextFontHeight);

	//generation
	PrintTextLabel* pGenerationTitle = new PrintTextLabel();
	pGenerationTitle->AddRef();
	hkvVec2 pGenerationTitlePosition = hkvVec2(-207.0f * fScale, -243.0f * fScale);
	string strTitleGeneration = StringsXmlHelper::sharedInstance().getStringForTag("generation");
	pGenerationTitle->init(pGenerationTitlePosition, strTitleGeneration, pTitleFont, fTitleFontHeight, fontColor);
	pGenerationTitle->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pGenerationTitle);

	PrintTextLabel* pGenerationText = new PrintTextLabel();
	pGenerationText->AddRef();
	hkvVec2 pGenerationTextPosition = hkvVec2(-207.0f * fScale, pGenerationTitlePosition.y + 35.0f * fScale);
	string strTextGeneration = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_meta_orb_generation_description");
	string strSplitTextGeneration;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(strSplitTextGeneration, strTextGeneration, 50);
	pGenerationText->init(pGenerationTextPosition, strSplitTextGeneration, pTextFont, fTextFontHeight, fontColor);
	pGenerationText->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pGenerationText);

	//defense
	PrintTextLabel* pDefenseTitle = new PrintTextLabel();
	pDefenseTitle->AddRef();
	hkvVec2 pDefenseTitlePosition = hkvVec2(-207.0f * fScale, -38.0f * fScale);
	string strTitleDefense = StringsXmlHelper::sharedInstance().getStringForTag("defense");
	pDefenseTitle->init(pDefenseTitlePosition, strTitleDefense, pTitleFont, fTitleFontHeight, fontColor);
	pDefenseTitle->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pDefenseTitle);

	PrintTextLabel* pDefenseText = new PrintTextLabel();
	pDefenseText->AddRef();
	hkvVec2 pDefenseTextPosition = hkvVec2(-207.0f * fScale, pDefenseTitlePosition.y + 35.0f * fScale);
	string strTextDefense = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_meta_orb_defense_description");
	string strSplitTextDefense;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(strSplitTextDefense, strTextDefense, 50);
	pDefenseText->init(pDefenseTextPosition, strSplitTextDefense, pTextFont, fTextFontHeight, fontColor);
	pDefenseText->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pDefenseText);

	//assault
	PrintTextLabel* pAssaultTitle = new PrintTextLabel();
	pAssaultTitle->AddRef();
	hkvVec2 pAssaultTitlePosition = hkvVec2(-207.0f * fScale, 169.0f * fScale);
	string strTitleAssault = StringsXmlHelper::sharedInstance().getStringForTag("assault");
	pAssaultTitle->init(pAssaultTitlePosition, strTitleAssault, pTitleFont, fTitleFontHeight, fontColor);
	pAssaultTitle->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pAssaultTitle);

	PrintTextLabel* pAssaultText = new PrintTextLabel();
	pAssaultText->AddRef();
	hkvVec2 pAssaultTextPosition = hkvVec2(-207.0f * fScale, pAssaultTitlePosition.y + 35.0f * fScale);
	string strTextAssault = StringsXmlHelper::sharedInstance().getStringForTag("help_menu_meta_orb_assault_description");
	string strSplitTextAssault;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(strSplitTextAssault, strTextAssault, 50);
	pAssaultText->init(pAssaultTextPosition, strTextAssault, pTextFont, fTextFontHeight, fontColor);
	pAssaultText->SetAnchorPoint(hkvVec2(0.0f, 0.0f));
	pPage2->addTextLabel(pAssaultText);
}

void HelpMenu::buildPage3()
{

}

void HelpMenu::showPage(int iPageNumber)
{
	//dismiss current displayed page
	if (m_iCurrentPageNumber > 0)
		m_pages[m_iCurrentPageNumber - 1]->fadeTo(0.0f, 0.3f, 0.0f);

	//show page
	m_pages[iPageNumber - 1]->fadeTo(1.0f, 0.3f, 0.5f);

	m_iCurrentPageNumber = iPageNumber;
}

//////////////////////////////////ObjectivesMenu//////////////////////////////////
void ObjectivesMenu::init()
{
	InterfaceMenu::init();
}

void ObjectivesMenu::show(bool withVeil, bool autoClose, bool announceNewObjectives, const vector<int> &objectivesNumbers)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 objectivesMenuSize = hkvVec2(1200.0f * scale, 600.0f * scale);

	InterfaceMenu::show(objectivesMenuSize, withVeil, autoClose);

	buildAndShow(announceNewObjectives, objectivesNumbers);
}

void ObjectivesMenu::buildAndShow(bool announceNewObjectives, const vector<int> &objectivesNumbers)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	hkvVec2 bgSize = hkvVec2(1200.0f * scale, 600.0f * scale);
	float contourThickness = 4.0f * scale;

	////PRIMARY TITLE
	VTextureObject* hexagonTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\hexagon.png");

	hkvVec2 stripesSize = hkvVec2(bgSize.x - 8.0f * scale, 64.0f);
	hkvVec4 stripesTextureRange = hkvVec4(0, 0, stripesSize.x, stripesSize.y);
	hkvVec2 hexagonSize = hkvVec2(32.0f * scale, 32.0f * scale);

	float primaryTitleYCoords = -0.5f * bgSize.y + 136.0f * scale;

	SpriteScreenMask* primaryHexagon = new SpriteScreenMask();
	primaryHexagon->AddRef();
	hkvVec2 primaryHexagonPosition = hkvVec2(-0.5f * bgSize.x + 36.0f * scale, primaryTitleYCoords);
	primaryHexagon->init(primaryHexagonPosition, hexagonSize, hexagonTexture);
	primaryHexagon->SetOrder(INTERFACE_MENU_ORDER - 2);
	this->addScreenMask(primaryHexagon);
	primaryHexagon->SetOpacity(0);
	primaryHexagon->fadeTo(1.0f, 0.3f, 0.5f);

	PrintTextLabel* primaryTextLabel = new PrintTextLabel();
	primaryTextLabel->AddRef();
	hkvVec2 primaryTextLabelPosition = hkvVec2(-0.5f * bgSize.x + 70.0f * scale, primaryTitleYCoords - 6.0f * scale);
	string primaryText = StringsXmlHelper::sharedInstance().getStringForTag("primary_obj");
	float primaryFontHeight = 46.0f * scale;
	VisFont_cl* primaryFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, primaryFontHeight);
	primaryTextLabel->init(primaryTextLabelPosition, primaryText, primaryFont, primaryFontHeight, V_RGBA_WHITE);
	primaryTextLabel->SetAnchorPoint(hkvVec2(0.0f, 0.5f));
	this->addTextLabel(primaryTextLabel);
	primaryTextLabel->SetOpacity(0);
	primaryTextLabel->fadeTo(1.0f, 0.3f, 0.5f);

	////PRIMARY OBJECTIVES
	float titleVerticalOffset = 50.0f * scale;

	LevelManager& levelManager = LevelManager::sharedInstance();
	vector<Objective*> objectives = levelManager.getCurrentLevel()->getObjectives();
	ObjectiveLabel* lastObjectiveLabel = NULL;
	int primaryObjectivesCount = 0;
	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		Objective* objective = objectives[objectiveIndex];
		if (!objective->isPrimary())
			continue;
		if (objective->isHidden())
			continue;
		if (announceNewObjectives && !objective->isContainedInObjectivesList(objectivesNumbers))
			continue;
		primaryObjectivesCount++;
		ObjectiveLabel* objectiveLabel = new ObjectiveLabel();
		objectiveLabel->AddRef();

		float verticalOffset = 20.0f * scale;
		hkvVec2 objectiveLocalPos;
		if (!lastObjectiveLabel)
		{
			objectiveLocalPos =  hkvVec2(-0.5f * bgSize.x + 70.0f * scale, primaryTitleYCoords + 42.0f * scale); //position of the top left hand corner of the objectiveLabel, relative to its parent position
		}
		else
		{
			int objectiveNumber = objectiveIndex + 1; //objective index starts at 1
			hkvVec2 lastObjectivePos = lastObjectiveLabel->getPos();
			objectiveLocalPos = hkvVec2(lastObjectivePos.x, lastObjectivePos.y + lastObjectiveLabel->getSize().y + verticalOffset);
		}
		lastObjectiveLabel = objectiveLabel;

		float fontHeight = 42.0f * scale;
		VColorRef fontColor = V_RGBA_WHITE;
		objectiveLabel->init(objectiveLocalPos, 70, objective, fontHeight, fontColor, objectiveIndex + 1);
		objectiveLabel->SetOrder(INTERFACE_MENU_ORDER - 1);
		this->addPanel(objectiveLabel);
		objectiveLabel->SetOpacity(0);
		objectiveLabel->fadeTo(1.0f, 0.3f, 0.5f);
	}

	if (primaryObjectivesCount == 0) //no primary objectives to display, remove the title
	{
		this->removeScreenMask(primaryHexagon);
		this->removeTextLabel(primaryTextLabel);
	}

	int secondaryObjectivesCount = 0;
	////SECONDARY TITLE
	float secondaryTitleYCoords;
	if (lastObjectiveLabel)
		secondaryTitleYCoords = lastObjectiveLabel->getPos().y + lastObjectiveLabel->getSize().y + 70.0f * scale;
	else //no primary objectives to display
		secondaryTitleYCoords = -0.5f * bgSize.y + 136.0f * scale;

	SpriteScreenMask* secondaryHexagon = new SpriteScreenMask();
	secondaryHexagon->AddRef();
	hkvVec2 secondaryHexagonPosition = hkvVec2(-0.5f * bgSize.x + 36.0f * scale, secondaryTitleYCoords);
	secondaryHexagon->init(secondaryHexagonPosition, hexagonSize, hexagonTexture);
	secondaryHexagon->SetOrder(INTERFACE_MENU_ORDER - 2);
	this->addScreenMask(secondaryHexagon);
	secondaryHexagon->SetOpacity(0);
	secondaryHexagon->fadeTo(1.0f, 0.3f, 0.5f);

	PrintTextLabel* secondaryTextLabel = new PrintTextLabel();
	secondaryTextLabel->AddRef();
	hkvVec2 secondaryTextLabelPosition = hkvVec2(-0.5f * bgSize.x + 70.0f * scale, secondaryTitleYCoords - 6.0f * scale);
	string secondaryText = StringsXmlHelper::sharedInstance().getStringForTag("secondary_obj");
	float secondaryFontHeight = 46.0f * scale;
	VisFont_cl* secondaryFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, secondaryFontHeight);
	secondaryTextLabel->init(secondaryTextLabelPosition, secondaryText, secondaryFont, secondaryFontHeight, V_RGBA_WHITE);
	secondaryTextLabel->SetAnchorPoint(hkvVec2(0.0f, 0.5f));
	this->addTextLabel(secondaryTextLabel);
	secondaryTextLabel->SetOpacity(0);
	secondaryTextLabel->fadeTo(1.0f, 0.3f, 0.5f);

	////SECONDARY OBJECTIVES
	objectives = levelManager.getCurrentLevel()->getObjectives();
	lastObjectiveLabel = NULL;
	for (int objectiveIndex = 0; objectiveIndex != objectives.size(); objectiveIndex++)
	{
		Objective* objective = objectives[objectiveIndex];
		if (objective->isPrimary())
			continue;
		if (objective->isHidden())
			continue;
		if (announceNewObjectives && !objective->isContainedInObjectivesList(objectivesNumbers))
			continue;
		secondaryObjectivesCount++;
		ObjectiveLabel* objectiveLabel = new ObjectiveLabel();
		objectiveLabel->AddRef();

		float verticalOffset = 20.0f * scale;
		hkvVec2 objectiveLocalPos;
		if (!lastObjectiveLabel)
		{
			objectiveLocalPos =  hkvVec2(-0.5f * bgSize.x + 70.0f * scale, secondaryTitleYCoords + 42.0f * scale); //position of the top left hand corner of the objectiveLabel, relative to its parent position
		}
		else
		{
			int objectiveNumber = objectiveIndex + 1; //objective index starts at 1
			hkvVec2 lastObjectivePos = lastObjectiveLabel->getPos();
			objectiveLocalPos = hkvVec2(lastObjectivePos.x, lastObjectivePos.y + lastObjectiveLabel->getSize().y + verticalOffset);
		}
		lastObjectiveLabel = objectiveLabel;

		float fontHeight = 42.0f * scale;
		VColorRef fontColor = V_RGBA_WHITE;
		objectiveLabel->init(objectiveLocalPos, 70, objective, fontHeight, fontColor, objectiveIndex + 1);
		objectiveLabel->SetOrder(INTERFACE_MENU_ORDER - 1);
		this->addPanel(objectiveLabel);
		objectiveLabel->SetOpacity(0);
		objectiveLabel->fadeTo(1.0f, 0.3f, 0.5f);
	}

	if (secondaryObjectivesCount == 0) //no primary objectives to display, remove the title
	{
		this->removeScreenMask(secondaryHexagon);
		this->removeTextLabel(secondaryTextLabel);
	}

	//TITLE
	PrintTextLabel* objectivesTitleTextLabel = new PrintTextLabel();
	objectivesTitleTextLabel->AddRef();
	hkvVec2 objectivesTitlePosition = hkvVec2(0, -0.5f * bgSize.y + 54.0f * scale);
	float objectiveTitleFontHeight = 52.0f * scale;
	VisFont_cl* objectiveTitleFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, objectiveTitleFontHeight);
	
	string objectivesTitleText;
	if (announceNewObjectives)
	{
		objectivesTitleText = StringsXmlHelper::sharedInstance().getStringForTag((objectivesNumbers.size() > 1) ? "new_objectives" : "new_objective");
	}
	else
	{
		objectivesTitleText = StringsXmlHelper::sharedInstance().getStringForTag("objectives");	
	}
	objectivesTitleTextLabel->init(objectivesTitlePosition, objectivesTitleText, objectiveTitleFont, objectiveTitleFontHeight, VColorRef(183, 255, 247));
	this->addTextLabel(objectivesTitleTextLabel);

	objectivesTitleTextLabel->SetOpacity(0);
	objectivesTitleTextLabel->fadeTo(1.0f, 0.3f, 0.5f);
}


ObjectiveLabel* ObjectivesMenu::findObjectiveLabelByNumber(int number)
{
	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		if (vdynamic_cast<ObjectiveLabel*>(panels[panelIndex]))
		{
			ObjectiveLabel* objectiveLabel = (ObjectiveLabel*) panels[panelIndex];
			if (objectiveLabel->getNumber() == number)
				return objectiveLabel;
		}
	}

	return NULL;
}

void ObjectivesMenu::completeObjective(int number)
{
	ObjectiveLabel* objective = findObjectiveLabelByNumber(number);
	if (objective)
		objective->complete();
}

//////////////////////////////////HelpMenuButton//////////////////////////////////
void HelpMenuButton::onClick()
{
	HelpMenu* pHelpMenu = GameDialogManager::sharedInstance()->getHUD()->getHelpMenu();
	int iCurrentPageNumber = pHelpMenu->getCurrentPageNumber();

	if (this->m_iID == VGUIManager::GetID(ID_HELP_MENU_ARROW_LEFT))
	{
		if (iCurrentPageNumber > 1)
		{
			pHelpMenu->showPage(--iCurrentPageNumber);
		}
	}
	else if (this->m_iID == VGUIManager::GetID(ID_HELP_MENU_ARROW_RIGHT))
	{
		int iLastPageReached = PersistentDataManager::sharedInstance().getHelpMenuLastReachedPage();
		if (iCurrentPageNumber < iLastPageReached)
		{
			pHelpMenu->showPage(++iCurrentPageNumber);
		}
	}

	HelpMenuButton* pArrowLeft = (HelpMenuButton*) pHelpMenu->findElementById(ID_HELP_MENU_ARROW_LEFT);
	HelpMenuButton* pArrowRight = (HelpMenuButton*) pHelpMenu->findElementById(ID_HELP_MENU_ARROW_RIGHT);
	pArrowLeft->updateState(iCurrentPageNumber);
	pArrowRight->updateState(iCurrentPageNumber);
}

void HelpMenuButton::updateState(int iCurrentPageNumber)
{
	if (this->m_iID == VGUIManager::GetID(ID_HELP_MENU_ARROW_LEFT))
	{
		if (iCurrentPageNumber == 1) //we reached the first page
		{
			this->SetOpacity(0.5f);
			this->disable();
		}
		else
		{
			this->SetOpacity(1.0f);
			this->enable();
		}
	}
	else if (this->m_iID == VGUIManager::GetID(ID_HELP_MENU_ARROW_RIGHT))
	{
		int iLastPageReached = PersistentDataManager::sharedInstance().getHelpMenuLastReachedPage();
		if (iCurrentPageNumber == iLastPageReached) //we reached the last page
		{
			this->SetOpacity(0.5f);
			this->disable();
		}
		else
		{
			this->SetOpacity(1.0f);
			this->enable();
		}
	}
}

//////////////////////////////////InterfaceCloseButton//////////////////////////////////
void InterfaceCloseButton::init(hkvVec2 position)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* closeBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\close_btn.png");
	hkvVec2 closeBtnSize = hkvVec2(64.0f * scale, 64.0f * scale);

	ButtonScreenMask::init(position, closeBtnSize, closeBtnTexture);
}

void InterfaceCloseButton::fadeIn()
{
	this->fadeTo(1.0f, 0.1f, 0.0f);
}

void InterfaceCloseButton::fadeOut()
{
	this->fadeTo(0.0f, 0.1f, 0.0f);
}

void InterfaceCloseButton::setAside()
{
	this->SetPosition(hkvVec2(0, -3000.0f));
}

void InterfaceCloseButton::onClick()
{
	GameHUD* hud = GameDialogManager::sharedInstance()->getHUD();
	HelpMenu* helpMenu = hud->getHelpMenu();
	PauseMenu* pauseMenu = hud->getPauseMenu();
	ObjectivesMenu* objectivesMenu = hud->getObjectivesMenu();
	if (helpMenu)
	{
		GameTimer::sharedInstance().setPaused(false);
		//helpMenu->setShowing(false);
		//helpMenu->onClose();
		GameDialogManager::sharedInstance()->getHUD()->dismissHelpMenu();
	}
	else if (pauseMenu)
	{
		if (pauseMenu->getSecondaryContent())
		{
			GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->dismissSecondaryContent();
			GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->showMainContent(true);
		}
		else
		{
			GameTimer::sharedInstance().setPaused(false);
			GameDialogManager::sharedInstance()->getHUD()->dismissPauseMenu();
		}
	}
	else if (objectivesMenu)
	{
		GameTimer::sharedInstance().setPaused(false);
		GameDialogManager::sharedInstance()->getHUD()->dismissObjectivesMenu();
	}
}

//////////////////////////////////ActionButton//////////////////////////////////
ActionButton::~ActionButton()
{

}

bool ActionButton::trigger(hkvVec2 touchLocation, int eventType)
{
	if (this->visible)
		return Clickable::trigger(touchLocation, eventType);
	else 
		return false;
}

void ActionButton::onClick()
{
	if (!GameDialogManager::sharedInstance()->getHUD()->getNodeSphere()) //no node sphere selected, no action
		return;
	if (this->m_iID == VGUIManager::GetID(ID_MOVE_BTN))
	{
		GameManager::GlobalManager().performMoveAction();		
	}
	else if (this->m_iID == VGUIManager::GetID(ID_RALLY_POINT_BTN))
	{
		GameManager::GlobalManager().performRallyAction();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BUILD_BTN))
	{
		GameManager::GlobalManager().performBuildBridgesAction();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER_VALIDATE))
	{
		if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1)	
		{
			if (!LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[2]->isComplete()
				&&
				!GameManager::GlobalManager().areInputEventsLocked())
			{
				GameManager::GlobalManager().lockInputEvents();
			}
		}

		Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
		NodeSphere* startSphere = GameManager::GlobalManager().getPickedNodeSphere();
		NodeSphere* endSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
		MoveSubMenuPanel* moveSubMenuPanel = (MoveSubMenuPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->findElementById(ID_MOVE_SUB_PANEL);
		int number = moveSubMenuPanel->getCurrentMovingOrbsCount();
		GameManager::GlobalManager().makeOrbsTravel(defaultTeam, startSphere, endSphere,  number);
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
		GameManager::GlobalManager().performMoveAction();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER_CANCEL))
	{
		GameDialogManager::sharedInstance()->getHUD()->removeActiveTravelPath();
		GameDialogManager::sharedInstance()->getHUD()->dismissMoveSubActionPanel();
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);
		GameManager::GlobalManager().performMoveAction();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_RALLY_SUB_PANEL_VALIDATE))
	{
		if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1)	
		{
			if (!LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[2]->isComplete()
				&&
				!GameManager::GlobalManager().areInputEventsLocked())
			{
				GameManager::GlobalManager().lockInputEvents();
			}
		}

		NodeSphere* pickedNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();
		NodeSphere* actionNodeSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
		pickedNodeSphere->setRallySphere(actionNodeSphere);
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);

		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		RallySubMenuPanel* rallySubMenuPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
		rallySubMenuPanel->hideAll(0.0f);
		GameManager::GlobalManager().performRallyAction();
		GameDialogManager::sharedInstance()->getHUD()->removeRallySphereHalo();
		GameDialogManager::sharedInstance()->getHUD()->removeActiveRallyPath();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_RALLY_SUB_PANEL_CANCEL))
	{
		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		RallySubMenuPanel* rallySubMenuPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
		rallySubMenuPanel->hideAll(0.0f);

		GameManager::GlobalManager().performRallyAction();
		GameDialogManager::sharedInstance()->getHUD()->removeRallySphereHalo();
		GameDialogManager::sharedInstance()->getHUD()->removeActiveRallyPath();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_RALLY_SUB_PANEL_DELETE))
	{
		NodeSphere* pickedNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();
		pickedNodeSphere->setRallySphere(NULL);
		GameManager::GlobalManager().setActionTargetNodeSphere(NULL);

		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		RallySubMenuPanel* rallySubMenuPanel = (RallySubMenuPanel*) actionPanel->findElementById(ID_RALLY_SUB_PANEL);
		rallySubMenuPanel->hideAll(0.0f);
		GameManager::GlobalManager().performRallyAction();

		GameDialogManager::sharedInstance()->getHUD()->removeRallySphereHalo();
		GameDialogManager::sharedInstance()->getHUD()->removeActiveRallyPath();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BUILD_SUB_PANEL_VALIDATE))
	{
		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		BuildSubMenuPanel* buildSubMenuPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL);
		buildSubMenuPanel->hideAll(0.0f);

		NodeSphere* pickedNodeSphere = GameManager::GlobalManager().getPickedNodeSphere();
		NodeSphere* actionTargetNodeSphere = GameManager::GlobalManager().getActionTargetNodeSphere();
		if (pickedNodeSphere->buildBridge(actionTargetNodeSphere))
		{
			GameDialogManager::sharedInstance()->getHUD()->removeBuildSphereHalo();
			GameManager::GlobalManager().performBuildBridgesAction();
		}
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BUILD_SUB_PANEL_CANCEL))
	{
		ActionPanel* actionPanel = (ActionPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		BuildSubMenuPanel* buildSubMenuPanel = (BuildSubMenuPanel*) actionPanel->findElementById(ID_BUILD_SUB_PANEL);
		buildSubMenuPanel->hideAll(0.0f);

		GameDialogManager::sharedInstance()->getHUD()->removeBuildSphereHalo();

		GameManager::GlobalManager().performBuildBridgesAction();
	}
}

//////////////////////////////////InterfaceButton//////////////////////////////////
void InterfaceButton::onClick()
{
	//pause the game
	GameTimer::sharedInstance().setPaused(true);

	if (this->m_iID == VGUIManager::GetID(ID_PAUSE_BTN))
		GameDialogManager::sharedInstance()->getHUD()->showPauseMenu(true, false);
	else if (this->m_iID == VGUIManager::GetID(ID_HELP_BTN))
		GameDialogManager::sharedInstance()->getHUD()->showHelpMenu(true, false);
	else if (this->m_iID == VGUIManager::GetID(ID_OBJECTIVES_BTN))
	{
		vector<int> empty;
		GameDialogManager::sharedInstance()->getHUD()->showObjectivesMenu(false, false, false, empty);
	}
}

//////////////////////////////////PauseMenuButton//////////////////////////////////
void PauseMenuButton::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_SAVE))
		; //TODO save the current state of the game
	else if (this->m_iID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_RETRY))
	{
		GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->buildAndShowSecondaryContent(m_iID);
		GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->dismissMainContent();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_MENU))	
	{
		GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->buildAndShowSecondaryContent(m_iID);
		GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->dismissMainContent();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_MUSIC))
	{
		bool music = PersistentDataManager::sharedInstance().toggleMusic();
		const char* texturePath = music ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png";
		VTextureObject* musicTexture = Vision::TextureManager.Load2DTexture(texturePath);
		this->updateTexture(musicTexture);
		AudioManager::sharedInstance().toggleMusic();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_PAUSE_MENU_BTN_SOUND))
	{
		bool sound = PersistentDataManager::sharedInstance().toggleSound();
		const char* texturePath = sound ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png";
		VTextureObject* soundTexture = Vision::TextureManager.Load2DTexture(texturePath);
		this->updateTexture(soundTexture);
		AudioManager::sharedInstance().toggleSound();
	}
}

//////////////////////////////////Objective//////////////////////////////////
V_IMPLEMENT_DYNCREATE(ObjectiveLabel, ScreenMaskPanel, Vision::GetEngineModule());

ObjectiveLabel::ObjectiveLabel() : pos(hkvVec2(0,0)), width(0), height(0), number(-1)
{
	
}

void ObjectiveLabel::init(hkvVec2 pos, int maxCharsPerLine, Objective* objective, float fontHeight, VColorRef fontColor, int number)
{
	this->pos = pos;
	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* emptyPointTexture = objective->isComplete() ? Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\point_on.png")
																: Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\point_off.png");
	VisFont_cl* font = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);;

	float pointMaskWidth = 32 * scale;
	float pointMaskHeight = 32 * scale;
	hkvVec2 pointMaskSize = hkvVec2(pointMaskWidth, pointMaskHeight);

	float fontScale = fontHeight / font->GetFontHeight();	
	VRectanglef textDim;
	string objectiveText = objective->getText();
	font->GetTextDimension(objectiveText.c_str(), textDim);
	textDim.operator*=(fontScale);
	float lineWidth = textDim.GetSizeX();
	float lineHeight = textDim.GetSizeY();
	string textWithLineSeparators;
	int numberOfLines = StringsXmlHelper::sharedInstance().splitStringAndCreateLines(textWithLineSeparators, objectiveText, maxCharsPerLine);

	this->width = lineWidth;
	if (numberOfLines > 1)
		this->height = numberOfLines * lineHeight;
	else
		this->height = max((float) pointMaskHeight, lineHeight);
	hkvVec2 objectivePanelSize = hkvVec2(width, height);

	hkvVec2 position = pos + 0.5f * hkvVec2(width, height);
	ScreenMaskPanel::init(position);

	hkvVec2 pointMaskPosition = -0.5f * (objectivePanelSize - pointMaskSize);
	SpriteScreenMask* pointMask = new SpriteScreenMask();
	pointMask->AddRef();
	pointMask->init(pointMaskPosition, pointMaskSize, emptyPointTexture);
	pointMask->SetOrder(INTERFACE_MENU_ORDER - 1);
	this->addScreenMask(pointMask);

	float textOffsetX = 0.5f * pointMaskWidth;
	PrintTextLabel* textLabel = new PrintTextLabel();
	textLabel->AddRef();
	hkvVec2 textLabelLocalPos = pointMaskPosition + hkvVec2(0.5f * pointMaskWidth, -0.5f * pointMaskHeight) + hkvVec2(textOffsetX, 0);
	textLabel->init(hkvVec2(0,0), textWithLineSeparators.c_str(), font, fontHeight, fontColor);
	textLabel->SetPosition(textLabelLocalPos + 0.5f * textLabel->getSize());
	this->addTextLabel(textLabel);

	this->number = number;
}

void ObjectiveLabel::complete()
{
	VTextureObject* pointOnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\point_on.png");
	SpriteScreenMask* pointMask = this->getScreenMasks()[0];
	if (pointMask)
		pointMask->updateTexture(pointOnTexture);
}

////////////////////////////////////DefeatVictoryMenu////////////////////////////////////
DefeatVictoryMenu::DefeatVictoryMenu() : VRefCounter(),
		victory(false),
		topBanner(NULL),
		bottomBanner(NULL),
		infosBg(NULL),
		infosPanel(NULL),
		buttonsPanel(NULL),
		contentShown(false)
{
	//Vision::Callbacks.OnUpdateSceneBegin += this;
}

DefeatVictoryMenu::~DefeatVictoryMenu() 
{
	//Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void DefeatVictoryMenu::init(bool victory)
{
	this->victory = victory;
}

void DefeatVictoryMenu::show()
{
	this->showBackground();

	CallFuncObject* showContent = new CallFuncObject(this, (CallFunc)(&DefeatVictoryMenu::showContent), 2.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showContent);
	showContent->executeFunc();
}

void DefeatVictoryMenu::showBackground()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	//Background
	bg = new Veil();
	VColorRef bgColor = V_RGBA_BLACK;
	bg->init(bgColor, 0.0f);
	bg->fadeTo(1.0f, 2.0f, 0.0f);
	bg->SetOrder(END_MENU_ORDER);
}

void DefeatVictoryMenu::showContent()
{
	this->showBanners();
	//this->showInfosBg();
	this->showInfos();
	this->showButtons();

	this->contentShown = true;
}

void DefeatVictoryMenu::showBanners()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	//top banner
	topBanner = new ScreenMaskPanel();
	hkvVec2 topBannerPosition = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y - 87.0f * scale);
	
	topBanner->init(topBannerPosition);

	VTextureObject* topBannerTopLineTexture = Vision::TextureManager.Load2DTexture(this->victory ? 
																				   "GUI\\Textures\\hdpi\\defeat_victory\\victory_top_banner_top_line.png" :
																				   "GUI\\Textures\\hdpi\\defeat_victory\\defeat_top_banner_top_line.png");
	VTextureObject* topBannerBottomLineTexture = Vision::TextureManager.Load2DTexture(this->victory ? 
																				   "GUI\\Textures\\hdpi\\defeat_victory\\victory_top_banner_bottom_line.png" :
																				   "GUI\\Textures\\hdpi\\defeat_victory\\defeat_top_banner_bottom_line.png");

	float topBannerTopLineWidth = 2048.0f * scale;
	float topBannerTopLineHeight = 64.0f * scale;
	hkvVec2 topBannerTopLineSize = hkvVec2(topBannerTopLineWidth, topBannerTopLineHeight);
	SpriteScreenMask* topBannerTopLine = new SpriteScreenMask();
	topBannerTopLine->AddRef();
	hkvVec2 topBannerTopLinePosition = hkvVec2(0, -55.0f * scale);
	topBannerTopLine->init(topBannerTopLinePosition, topBannerTopLineSize, topBannerTopLineTexture);
	topBannerTopLine->SetOrder(END_MENU_ORDER - 2);
	topBanner->addScreenMask(topBannerTopLine);

	float topBannerBottomLineWidth = 2048.0f * scale;
	float topBannerBottomLineHeight = 64.0f * scale;
	hkvVec2 topBannerBottomLineSize = hkvVec2(topBannerBottomLineWidth, topBannerBottomLineHeight);
	SpriteScreenMask* topBannerBottomLine = new SpriteScreenMask();
	topBannerBottomLine->AddRef();
	hkvVec2 topBannerBottomLinePosition = hkvVec2(0, 55.0f * scale);
	topBannerBottomLine->init(topBannerBottomLinePosition, topBannerBottomLineSize, topBannerBottomLineTexture);
	topBannerBottomLine->SetOrder(END_MENU_ORDER - 2);
	topBanner->addScreenMask(topBannerBottomLine);

	PrintTextLabel* resultTextLabel = new PrintTextLabel();
	float fontHeight = 95 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fontHeight);
	resultTextLabel->AddRef();
	string resultText = StringsXmlHelper::sharedInstance().getStringForTag(this->victory ? "victory" : "defeat");
	resultTextLabel->init(hkvVec2(0, -13.0f * scale), resultText, neuropolFont, fontHeight, V_RGBA_WHITE);
	topBanner->addTextLabel(resultTextLabel);

	//bottom banner
	bottomBanner = new ScreenMaskPanel();
	hkvVec2 bottomBannerPosition = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 76.0f * scale);
	bottomBanner->init(bottomBannerPosition);

	VTextureObject* bottomBannerTopLineTexture = Vision::TextureManager.Load2DTexture(this->victory ? 
																					  "GUI\\Textures\\hdpi\\defeat_victory\\victory_bottom_banner_line.png" :
																				      "GUI\\Textures\\hdpi\\defeat_victory\\defeat_bottom_banner_line.png");

	float bottomBannerLineWidth = 2048.0f * scale;
	float bottomBannerLineHeight = 64.0f * scale;
	hkvVec2 bottomBannerLineSize = hkvVec2(bottomBannerLineWidth, bottomBannerLineHeight);
	SpriteScreenMask* bottomBannerTopLine = new SpriteScreenMask();
	bottomBannerTopLine->AddRef();
	hkvVec2 bottomBannerTopLinePosition = hkvVec2(0, -44.0f * scale);
	bottomBannerTopLine->init(bottomBannerTopLinePosition, bottomBannerLineSize, bottomBannerTopLineTexture);
	bottomBannerTopLine->SetOrder(END_MENU_ORDER - 2);
	bottomBanner->addScreenMask(bottomBannerTopLine);

	SpriteScreenMask* bottomBannerBottomLine = new SpriteScreenMask();
	bottomBannerBottomLine->AddRef();
	hkvVec2 bottomBannerBottomLinePosition = hkvVec2(0, 44.0f * scale);
	bottomBannerBottomLine->init(bottomBannerBottomLinePosition, bottomBannerLineSize, bottomBannerTopLineTexture, hkvVec2(1.0f, 1.0f), 180);
	bottomBannerBottomLine->SetOrder(END_MENU_ORDER - 2);
	bottomBanner->addScreenMask(bottomBannerBottomLine);

	//PrintTextLabel* levelTextLabel = new PrintTextLabel();
	//fontHeight = 85 * scale;
	//levelTextLabel->AddRef();
	//string levelText = StringsXmlHelper::sharedInstance().getStringForTag("short_level") + stringFromInt(12);
	//levelTextLabel->init(hkvVec2(0, -10.0f /1536.0f * screenSize.y), levelText, neuropolFont, fontHeight, V_RGBA_WHITE);
	//bottomBanner->addTextLabel(levelTextLabel);

	//xp textlabel
	//this->showXP();
	bottomBanner->SetOpacity(0.0f);
	topBanner->SetOpacity(0.0f);

	//translate banners
	hkvVec2 topBannerToPosition = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y - 256.0f * scale - 66.0f * scale);
	hkvVec2 bottomBannerToPosition = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 256.0f * scale + 52.0f * scale);
	topBanner->fadeTo(1.0f, 0.3f, 0.0f);
	bottomBanner->fadeTo(1.0f, 0.3f, 0.0f);
	topBanner->translateTo(topBannerToPosition, 0.4f, 0.0f);
	bottomBanner->translateTo(bottomBannerToPosition, 0.4f, 0.0f);
}

void DefeatVictoryMenu::showInfosBg()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* infosBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\hexaorb_bg.png");

	infosBg = new SpriteScreenMask();
	hkvVec2 infosBgPosition = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y);
	hkvVec2 infosBgSize = hkvVec2(1024.0f * scale, 512.0f * scale);
	hkvVec4 infosBgTextureRange = hkvVec4(0, 0, 1024.0f, 0);
	infosBg->init(infosBgPosition, infosBgSize, infosBgTextureRange, infosBgTexture, false);
	infosBg->SetOrder(END_MENU_ORDER - 3);
}

void DefeatVictoryMenu::showInfos()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	infosPanel = new ScreenMaskPanel();
	infosPanel->init(hkvVec2(0.5f * screenSize.x, 741.0f / 1536.0f * screenSize.y));

	float fontHeight = 45 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);

	//secondary objectives
	int secondaryObjectivesCount = LevelManager::sharedInstance().getSecondaryObjectivesCount();
	if (secondaryObjectivesCount > 0)
	{
		ScreenMaskPanel* secondaryObjectiveLine = new ScreenMaskPanel();
		secondaryObjectiveLine->AddRef();
		secondaryObjectiveLine->init(hkvVec2(0,0));

		VTextureObject* triangleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\triangle_results.png");
		SpriteScreenMask* triangle = new SpriteScreenMask();
		triangle->AddRef();
		hkvVec2 trianglePosition = hkvVec2(-555.0f * scale, 0);
		hkvVec2 triangleSize = hkvVec2(64.0f * scale, 64.0f * scale);
		triangle->init(trianglePosition, triangleSize, triangleTexture);
		triangle->SetOrder(END_MENU_ORDER - 4);
		secondaryObjectiveLine->addScreenMask(triangle);

		//VTextureObject* blackBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\black_bg_results.png");
		//SpriteScreenMask* blackBg = new SpriteScreenMask();
		//blackBg->AddRef();
		//hkvVec2 blackBgPosition = hkvVec2(0,0);
		//hkvVec2 blackBgSize = hkvVec2(1072.0f * scale, 64.0f * scale);
		//blackBg->init(blackBgPosition, blackBgSize, blackBgTexture);
		//blackBg->SetOrder(END_MENU_ORDER - 4);
		//secondaryObjectiveLine->addScreenMask(blackBg);
		//blackBg->SetOpacity(0.7f);

		PrintTextLabel* secondaryObjectivesTextLabel = new PrintTextLabel();
		secondaryObjectivesTextLabel->AddRef();
		hkvVec2 secondaryObjectivesTextLabelPosition = hkvVec2(0, -5.0f * scale);
		int completedSecondaryObjectivesCount = LevelManager::sharedInstance().getCompletedSecondaryObjectivesCount();

		string secondaryObjectivesText = StringsXmlHelper::sharedInstance().getStringForTag("secondary_obj") + 
			string(" ") + 
			stringFromInt(completedSecondaryObjectivesCount) + 
			string("/") +
			stringFromInt(secondaryObjectivesCount);
		secondaryObjectivesTextLabel->init(secondaryObjectivesTextLabelPosition, secondaryObjectivesText, neuropolFont, fontHeight, V_RGBA_WHITE);
		secondaryObjectiveLine->addTextLabel(secondaryObjectivesTextLabel);
		
		//secondaryObjectivesTextLabel->SetOpacity(0.0f);
		//secondaryObjectivesTextLabel->fadeTo(1.0f, 0.3f, 2.8f);

		infosPanel->addPanel(secondaryObjectiveLine);
	}

	////primary objectives
	ScreenMaskPanel* primaryObjectiveLine = new ScreenMaskPanel();
	primaryObjectiveLine->AddRef();
	hkvVec2 primaryObjectivesLinePosition = (secondaryObjectivesCount > 0) ? hkvVec2(0, -86.0f / 1536.0f * screenSize.y) : hkvVec2(0, -43.0f / 1536.0f * screenSize.y);
	primaryObjectiveLine->init(primaryObjectivesLinePosition);

	VTextureObject* triangleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\triangle_results.png");
	SpriteScreenMask* triangle = new SpriteScreenMask();
	triangle->AddRef();
	hkvVec2 trianglePosition = hkvVec2(-555.0f * scale, 0);
	hkvVec2 triangleSize = hkvVec2(64.0f * scale, 64.0f * scale);
	triangle->init(trianglePosition, triangleSize, triangleTexture);
	triangle->SetOrder(END_MENU_ORDER - 4);
	primaryObjectiveLine->addScreenMask(triangle);

	//VTextureObject* blackBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\black_bg_results.png");
	//SpriteScreenMask* blackBg = new SpriteScreenMask();
	//blackBg->AddRef();
	//hkvVec2 blackBgPosition = hkvVec2(0,0);
	//hkvVec2 blackBgSize = hkvVec2(1072.0f * scale, 64.0f * scale);
	//blackBg->init(blackBgPosition, blackBgSize, blackBgTexture);
	//blackBg->SetOrder(END_MENU_ORDER - 4);
	//primaryObjectiveLine->addScreenMask(blackBg);
	//blackBg->SetOpacity(0.7f);

	PrintTextLabel* primaryObjectivesTextLabel = new PrintTextLabel();
	primaryObjectivesTextLabel->AddRef();
	hkvVec2 primaryObjectivesTextLabelPosition = hkvVec2(0, -5.0f * scale);
	int completedPrimaryObjectivesCount = LevelManager::sharedInstance().getCompletedPrimaryObjectivesCount();
	int primaryObjectivesCount = LevelManager::sharedInstance().getPrimaryObjectivesCount();
	string primaryObjectivesText = StringsXmlHelper::sharedInstance().getStringForTag("primary_obj") + 
								   string(" ") + 
								   stringFromInt(completedPrimaryObjectivesCount) + 
								   string("/") +
								   stringFromInt(primaryObjectivesCount);
	primaryObjectivesTextLabel->init(primaryObjectivesTextLabelPosition, primaryObjectivesText, neuropolFont, fontHeight, V_RGBA_WHITE);
	primaryObjectiveLine->addTextLabel(primaryObjectivesTextLabel);

	infosPanel->addPanel(primaryObjectiveLine);

	////killed enemy orbs
	ScreenMaskPanel* killedOrbsLine = new ScreenMaskPanel();
	killedOrbsLine->AddRef();
	hkvVec2 killedOrbsLinePosition = (secondaryObjectivesCount > 0) ? hkvVec2(0, 86.0f / 1536.0f * screenSize.y): hkvVec2(0, 43.0f / 1536.0f * screenSize.y);
	killedOrbsLine->init(killedOrbsLinePosition);

	triangleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\triangle_results.png");
	triangle = new SpriteScreenMask();
	triangle->AddRef();
	trianglePosition = hkvVec2(-555.0f * scale, 0);
	triangleSize = hkvVec2(64.0f * scale, 64.0f * scale);
	triangle->init(trianglePosition, triangleSize, triangleTexture);
	triangle->SetOrder(END_MENU_ORDER - 4);
	killedOrbsLine->addScreenMask(triangle);

	//blackBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\black_bg_results.png");
	//blackBg = new SpriteScreenMask();
	//blackBg->AddRef();
	//blackBgPosition = hkvVec2(0,0);
	//blackBgSize = hkvVec2(1072.0f * scale, 64.0f * scale);
	//blackBg->init(blackBgPosition, blackBgSize, blackBgTexture);
	//blackBg->SetOrder(END_MENU_ORDER - 4);
	//killedOrbsLine->addScreenMask(blackBg);
	//blackBg->SetOpacity(0.7f);

	PrintTextLabel* killedOrbsTextLabel = new PrintTextLabel();
	killedOrbsTextLabel->AddRef();
	hkvVec2 killedOrbsTextLabelPosition = hkvVec2(0,0);
	int killedOrbsCount = LevelManager::sharedInstance().getCurrentLevel()->getKilledEnemyOrbsCount();
	string killedOrbsText = StringsXmlHelper::sharedInstance().getStringForTag("killed_orbs") + stringFromInt(killedOrbsCount);
	killedOrbsTextLabel->init(killedOrbsTextLabelPosition, killedOrbsText, neuropolFont, fontHeight, V_RGBA_WHITE);
	killedOrbsLine->addTextLabel(killedOrbsTextLabel);

	infosPanel->addPanel(killedOrbsLine);

	//hide textLabels
	//primaryObjectivesTextLabel->SetOpacity(0.0f);
	//killedOrbsTextLabel->SetOpacity(0.0f);

	//primaryObjectivesTextLabel->fadeTo(1.0f, 0.3f, 0.0f);
	//killedOrbsTextLabel->fadeTo(1.0f, 0.3f, 0.0f);

	infosPanel->SetOpacity(0.0f);
	infosPanel->fadeTo(1.0f, 0.3f, 0.2f);

	//CallFuncObject* revealPrimaryObjectives = new CallFuncObject(this, (CallFunc)(&DefeatVictoryMenu::revealPrimaryObjectives), 0.15f);
	//CallFuncObject* revealSecondaryObjectives = new CallFuncObject(this, (CallFunc)(&DefeatVictoryMenu::revealSecondaryObjectives), 0.0f);
	//CallFuncObject* revealKiledOrbs = new CallFuncObject(this, (CallFunc)(&DefeatVictoryMenu::revealKilledOrbs), 0.15f);
	//CallFuncHandler::sharedInstance().addCallFuncInstance(revealPrimaryObjectives);
	//CallFuncHandler::sharedInstance().addCallFuncInstance(revealSecondaryObjectives);
	//CallFuncHandler::sharedInstance().addCallFuncInstance(revealKiledOrbs);
	//revealPrimaryObjectives->executeFunc();
	//revealSecondaryObjectives->executeFunc();
	//revealKiledOrbs->executeFunc();
}

void DefeatVictoryMenu::showButtons()
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	//buttons
	buttonsPanel = new ScreenMaskPanel();
	buttonsPanel->init(hkvVec2(0.5f * screenSize.x, screenSize.y - 160.0f * scale));

	//font
	float buttonsFontHeight = 40.0f * scale;
	VisFont_cl* buttonsFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, buttonsFontHeight);
	VColorRef buttonsFontColor = victory ? VColorRef(149,255,251) : V_RGBA_RED;

	//left button (menu)
	DefeatVictoryMenuButton* leftBtn = new DefeatVictoryMenuButton();
	leftBtn->AddRef();
	hkvVec2 leftBtnPosition = hkvVec2(-324.0f / 2048.0f * screenSize.x, 0);
	string leftBtnText = StringsXmlHelper::sharedInstance().getStringForTag("menu");
	leftBtn->init(leftBtnPosition, this->victory);
	buttonsPanel->addScreenMask(leftBtn);
	leftBtn->m_iID = VGUIManager::GetID(ID_BTN_MENU);

	PrintTextLabel* leftBtnTextLabel = new PrintTextLabel();
	leftBtnTextLabel->AddRef();
	hkvVec2 leftBtnTextLabelPosition = leftBtnPosition + hkvVec2(0, 100.0f * scale);
	leftBtnTextLabel->init(leftBtnTextLabelPosition, leftBtnText, buttonsFont, buttonsFontHeight, buttonsFontColor);
	buttonsPanel->addTextLabel(leftBtnTextLabel);

	//right button (continue/retry)
	DefeatVictoryMenuButton* rightBtn = new DefeatVictoryMenuButton();
	rightBtn->AddRef();
	hkvVec2 rightBtnPosition = hkvVec2(324.0f / 2048.0f * screenSize.x, 0);
	string rightBtnText = this->victory ? StringsXmlHelper::sharedInstance().getStringForTag("continue") : StringsXmlHelper::sharedInstance().getStringForTag("retry");
	rightBtn->init(rightBtnPosition, this->victory);
	buttonsPanel->addScreenMask(rightBtn);
	rightBtn->m_iID = VGUIManager::GetID(this->victory ? ID_BTN_CONTINUE : ID_BTN_RETRY);

	PrintTextLabel* rightBtnTextLabel = new PrintTextLabel();
	rightBtnTextLabel->AddRef();
	hkvVec2 rightBtnTextLabelPosition = rightBtnPosition + hkvVec2(0, 100.0f * scale);
	rightBtnTextLabel->init(rightBtnTextLabelPosition, rightBtnText, buttonsFont, buttonsFontHeight, buttonsFontColor);
	buttonsPanel->addTextLabel(rightBtnTextLabel);

	leftBtn->SetOpacity(0.0f);
	rightBtn->SetOpacity(0.0f);
	leftBtnTextLabel->SetOpacity(0.0f);
	rightBtnTextLabel->SetOpacity(0.0f);

	leftBtn->fadeTo(1.0f, 0.3f, 0.0f);
	rightBtn->fadeTo(1.0f, 0.3f, 0.0f);
	leftBtnTextLabel->fadeTo(1.0f, 0.3f, 0.0f);
	rightBtnTextLabel->fadeTo(1.0f, 0.3f, 0.0f);
}

void DefeatVictoryMenu::revealPrimaryObjectives()
{
	PrintTextLabel* primaryObjectivesLabel = infosPanel->getTextLabels()[0];
	primaryObjectivesLabel->SetOpacity(1.0f);
}
void DefeatVictoryMenu::revealSecondaryObjectives()
{
	PrintTextLabel* secondaryObjectivesLabel = infosPanel->getTextLabels()[1];
	secondaryObjectivesLabel->SetOpacity(1.0f);
}

void DefeatVictoryMenu::revealKilledOrbs()
{
	PrintTextLabel* killedOrbsLabel = infosPanel->getTextLabels()[2];
	killedOrbsLabel->SetOpacity(1.0f);
}

void DefeatVictoryMenu::showXP()
{
	int totalXP = 0;
	if (this->victory)
	{
		int completedPrimaryObjectives = LevelManager::sharedInstance().getPrimaryObjectivesCount();
		int completedSecondaryObjectives = LevelManager::sharedInstance().getCompletedSecondaryObjectivesCount();
		totalXP += (completedPrimaryObjectives * XP_EARNED_PER_PRIMARY_OBJECTIVE);
		totalXP += (completedPrimaryObjectives * XP_EARNED_PER_SECONDARY_OBJECTIVE);
	}
	int killedOrbs = LevelManager::sharedInstance().getCurrentLevel()->getKilledEnemyOrbsCount();
	totalXP += (killedOrbs * XP_EARNED_PER_ORB);

	//xp text panel
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	ScreenMaskPanel* xpTextPanel = new ScreenMaskPanel();
	xpTextPanel->AddRef();
	hkvVec2 xpTextPanelPosition = hkvVec2(0, -75.0f  / 1536.0f * screenSize.y);
	xpTextPanel->init(xpTextPanelPosition);

	VTextureObject* lineTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	float lineWidth = 77 * scale;
	float lineHeight = 4 * scale;
	hkvVec2 lineSize = hkvVec2(lineWidth, lineHeight);

	SpriteScreenMask* leftLine = new SpriteScreenMask();
	leftLine->AddRef();
	hkvVec2 leftLinePosition = hkvVec2(-140.0f / 2048.0f * screenSize.x, 0);
	leftLine->init(leftLinePosition, lineSize, hkvVec4(0, 0, lineWidth, lineHeight), lineTexture, true);
	xpTextPanel->addScreenMask(leftLine);

	SpriteScreenMask* rightLine = new SpriteScreenMask();
	rightLine->AddRef();
	hkvVec2 rightLinePosition = hkvVec2(140.0f / 2048.0f * screenSize.x, 0);
	rightLine->init(rightLinePosition, lineSize, hkvVec4(0, 0, lineWidth, lineHeight), lineTexture, true);
	xpTextPanel->addScreenMask(rightLine);

	PrintTextLabel* xpTextLabel = new PrintTextLabel();
	xpTextLabel->AddRef();
	float fontHeight = 40 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	string xpText = string("XP +") + stringFromInt(totalXP);
	xpTextLabel->init(hkvVec2(0, -5.0f / 1536.0f * screenSize.y), xpText, neuropolFont, fontHeight, V_RGBA_WHITE);
	xpTextPanel->addTextLabel(xpTextLabel);

	bottomBanner->addPanel(xpTextPanel);

	//xp progress bar
}

bool DefeatVictoryMenu::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	if (buttonsPanel)
	{
		vector<SpriteScreenMask*> buttons = buttonsPanel->getScreenMasks();
		for (int buttonIndex = 0; buttonIndex != buttons.size(); buttonIndex++)
		{
			DefeatVictoryMenuButton* button = (DefeatVictoryMenuButton*) buttons[buttonIndex];
			if (button->trigger(touchLocation, eventType))
				return true;
		}
	}
	return false;
}

void DefeatVictoryMenu::removeSelf()
{
	topBanner = NULL;
	bottomBanner = NULL;
	infosBg = NULL;
	infosPanel = NULL;
	buttonsPanel = NULL;
}

//void DefeatVictoryMenu::OnHandleCallback(IVisCallbackDataObject_cl *pData)
//{
//	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
//	{
//		updateInfosBgSize();
//	}
//}

//void DefeatVictoryMenu::updateInfosBgSize()
//{
//	if (!this->contentShown)
//		return;
//
//	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
//	float scale = GameDialogManager::sharedInstance()->getScale();
//
//	hkvVec2 topBannerPosition = this->topBanner->GetPosition();
//	hkvVec2 bottomBannerPosition = this->bottomBanner->GetPosition();
//	float infosBgTopContourPosition = topBannerPosition.y + 66.0f * scale;
//	float infosBgBottomContourPosition = bottomBannerPosition.y - 52.0f * scale;
//	hkvVec2 infosBgSize = hkvVec2(1024.0f * scale, infosBgBottomContourPosition - infosBgTopContourPosition);
//	if (infosBgSize.y > 512.0f * scale)
//		infosBgSize.y = 512.0f * scale;
//	hkvVec4 infosBgTextureRange = hkvVec4(0, 256.0f - 0.5f * infosBgSize.y / scale, 1024.0f, 256.0f + 0.5f * infosBgSize.y / scale);
//	this->infosBg->SetTargetSize(infosBgSize.x, infosBgSize.y);
//	this->infosBg->SetTextureRange(infosBgTextureRange.x, infosBgTextureRange.y, infosBgTextureRange.z, infosBgTextureRange.w);
//	this->infosBg->SetPosition(hkvVec2(infosBg->GetPosition().x, infosBg->GetPosition().y));
//
//	hkvVec2 pos = hkvVec2(infosBg->GetPosition().x - 0.5f * infosBgSize.x, infosBg->GetPosition().y - 0.5f * infosBgSize.y); //update the position
//	this->infosBg->SetPos(pos.x, pos.y);
//}

////////////////////////////////////DefeatVictoryMenuButton////////////////////////////////////
void DefeatVictoryMenuButton::init(hkvVec2 position, bool victory)
{
	VTextureObject* buttonTexture = victory ? Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\blue_circle_button.png") : 
											  Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\red_circle_button.png");


	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 buttonSize = hkvVec2(128.0f * scale, 128.0f * scale);

	ButtonScreenMask::init(position, buttonSize, buttonTexture);
}

void DefeatVictoryMenuButton::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_BTN_MENU))
	{
		hkvLog::Error("ID_BTN_MENU onClick");
		SceneManager::sharedInstance().loadMainMenu();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BTN_CONTINUE))
	{
		hkvLog::Error("ID_BTN_CONTINUE onClick");
		int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
		int levelsCount = LevelManager::sharedInstance().getLevels().size();
		if (++currentLevelNumber <= levelsCount)
		{
			hkvLog::Error("LOADING LEVEL %i", currentLevelNumber);
			SceneManager::sharedInstance().loadLevel(currentLevelNumber);
		}
		else
			hkvLog::Error("END GAME show Credits");
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BTN_RETRY))
	{
		hkvLog::Error("ID_BTN_RETRY onClick");
		int currentLevelNumber = LevelManager::sharedInstance().getCurrentLevel()->getNumber();
		SceneManager::sharedInstance().loadLevel(currentLevelNumber); //reload same level
	}
}

////////////////////////////////////ActionPanel////////////////////////////////////
void ActionPanel::init(hkvVec2 position, NodeSphere* parentNodeSphere)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ScreenMaskPanel::init(position);
	this->parentNodeSphere = parentNodeSphere;
	this->actionMode = ACTION_NORMAL_MODE;

	//textures
	VTextureObject* moveBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\move_btn.png");
	VTextureObject* rallyPointBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\rally_point_btn.png");
	VTextureObject* buildBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\build_btn.png");

	//action buttons
	float actionBtnWidth = 156.0f * scale;
	float actionBtnHeight = 156.0f * scale;
	hkvVec2 actionBtnSize = hkvVec2(actionBtnWidth, actionBtnHeight);

	//move
	moveBtn = new ActionButton();
	moveBtn->AddRef();
	hkvVec2 moveBtnPosition = hkvVec2(0.5f * actionBtnWidth, -180.0f * scale);
	moveBtn->init(moveBtnPosition, actionBtnSize, moveBtnTexture);
	moveBtn->m_iID = VGUIManager::GetID(ID_MOVE_BTN);
	moveBtn->SetColor(TRAVEL_MODE_COLOR);
	moveBtn->setEnabledColor(TRAVEL_MODE_COLOR);
	moveBtn->setSelectedColor(TRAVEL_MODE_COLOR);
	this->addScreenMask(moveBtn);

	Team* defaultTeam = TeamManager::sharedInstance().getDefaultTeam();
	//rally
	rallyBtn = new ActionButton();
	rallyBtn->AddRef();
	hkvVec2 rallyPointBtnPosition = hkvVec2(0.5f * actionBtnWidth, 0);
	rallyBtn->init(rallyPointBtnPosition, actionBtnSize, rallyPointBtnTexture);
	rallyBtn->m_iID = VGUIManager::GetID(ID_RALLY_POINT_BTN);
	rallyBtn->SetColor(RALLY_POINT_MODE_COLOR);
	rallyBtn->setEnabledColor(RALLY_POINT_MODE_COLOR);
	rallyBtn->setSelectedColor(RALLY_POINT_MODE_COLOR);
	this->addScreenMask(rallyBtn);

	//build
	buildBtn = new ActionButton();
	buildBtn->AddRef();
	hkvVec2 buildBtnPosition = hkvVec2(0.5f * actionBtnWidth, 180.0f * scale);		
	buildBtn->init(buildBtnPosition, actionBtnSize, buildBtnTexture);		
	buildBtn->m_iID = VGUIManager::GetID(ID_BUILD_BTN);
	buildBtn->SetColor(BUILD_MODE_COLOR);
	buildBtn->setEnabledColor(BUILD_MODE_COLOR);
	buildBtn->setSelectedColor(BUILD_MODE_COLOR);
	this->addScreenMask(buildBtn);

	this->dismissMoveBtn(false, 0.0f);
	this->dismissRallyBtn(false, 0.0f);
	this->dismissBuildBtn(false, 0.0f);

	//move sub panel
	GameDialogManager::sharedInstance()->getHUD()->buildMoveSubActionPanel();

	//rally sub panel
	GameDialogManager::sharedInstance()->getHUD()->buildRallySubActionPanel();

	//build subPanel
	BuildSubMenuPanel* buildSubMenuPanel = new BuildSubMenuPanel();
	buildSubMenuPanel->AddRef();
	buildSubMenuPanel->init(hkvVec2(0, 0));
	GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->addPanel(buildSubMenuPanel);

	this->SetOrder(ACTION_PANEL_ORDER);
}

void ActionPanel::updateActionMode(int newActionMode)
{
	this->actionMode = newActionMode;

	//update selection halo
	ScreenMaskPanel* selectionHaloPanel = GameDialogManager::sharedInstance()->getHUD()->getSelectionHaloPanel();
	if (selectionHaloPanel)
	{
		SelectionHalo* selectionHalo = (SelectionHalo*) selectionHaloPanel->getScreenMasks()[0];
		selectionHalo->updateForActionMode(newActionMode);
	}
}

void ActionPanel::showMoveBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay)
{
	if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
		&&
		!TutorialsHandler::sharedInstance().isMoveBtnRevealed())
		return;

	moveBtn->enable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	if (animated)
	{
		float animationDuration;
		if (duration <= 0)
		{
			float absDeltaPositionX = abs(toPosition.x - moveBtn->GetPosition().x);
			float maxAmplitude = 100.0f * scale - toPosition.x;
			float maxAnimationDuration = 0.2f;
			animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		}
		else
			animationDuration = duration;
		moveBtn->translateTo(toPosition, animationDuration, delay);
		moveBtn->scaleTo(toScale, animationDuration, delay);
	}
	else
	{
		moveBtn->SetPosition(toPosition);
		moveBtn->SetScale(toScale);
	}
}

void ActionPanel::showRallyBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay)
{
	if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
		&&
		!TutorialsHandler::sharedInstance().isRallyBtnRevealed())
		return;

	rallyBtn->enable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	if (animated)
	{
		float animationDuration;
		if (duration <= 0)
		{
			float absDeltaPositionX = abs(toPosition.x - rallyBtn->GetPosition().x);
			float maxAmplitude = 100.0f * scale - toPosition.x;
			float maxAnimationDuration = 0.2f;
			animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		}
		else
			animationDuration = duration;

		rallyBtn->translateTo(toPosition, animationDuration, delay);
		rallyBtn->scaleTo(toScale, animationDuration, delay);
	}
	else
	{
		rallyBtn->SetPosition(toPosition);
		rallyBtn->SetScale(toScale);
	}
}

void ActionPanel::showBuildBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay)
{
	if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1
		&&
		!TutorialsHandler::sharedInstance().isBuildBtnRevealed())
		return;

	if (GameManager::GlobalManager().getPickedNodeSphere()->findPotentialBridges().size() == 0)
		return;

	buildBtn->enable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	if (animated)
	{
		float animationDuration;
		if (duration <= 0)
		{
			float absDeltaPositionX = abs(toPosition.x - buildBtn->GetPosition().x);
			float maxAmplitude = 100.0f * scale - toPosition.x; //from -78.0f to 100.0f (* scale)
			float maxAnimationDuration = 0.2f;
			animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		}
		else
			animationDuration = duration;

		buildBtn->translateTo(toPosition, animationDuration, delay);
		buildBtn->scaleTo(toScale, animationDuration, delay);
	}
	else
	{
		buildBtn->SetPosition(toPosition);
		buildBtn->SetScale(toScale);
	}
}

void ActionPanel::dismissMoveBtn(bool animated, float delay)
{
	moveBtn->disable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 moveBtnToPosition = hkvVec2(100.0f * scale, moveBtn->GetPosition().y);
	if (animated)
	{
		float absDeltaPositionX = abs(moveBtnToPosition.x - moveBtn->GetPosition().x);
		float maxAmplitude = 178.0f * scale; //from -78.0f to 100.0f (* scale)
		float maxAnimationDuration = 0.2f;
		float animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		moveBtn->translateTo(moveBtnToPosition, animationDuration, delay);
		moveBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, delay);
	}
	else
		moveBtn->SetPosition(moveBtnToPosition);
}

void ActionPanel::dismissRallyBtn(bool animated, float delay)
{
	rallyBtn->disable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 rallyBtnToPosition = hkvVec2(100.0f * scale, rallyBtn->GetPosition().y);
	if (animated)
	{
		float absDeltaPositionX = abs(rallyBtnToPosition.x - rallyBtn->GetPosition().x);
		float maxAmplitude = 178.0f * scale; //from -78.0f to 100.0f (* scale)
		float maxAnimationDuration = 0.2f;
		float animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		rallyBtn->translateTo(rallyBtnToPosition, animationDuration, delay);
		rallyBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, delay);
	}
	else
		rallyBtn->SetPosition(rallyBtnToPosition);
}

void ActionPanel::dismissBuildBtn(bool animated, float delay)
{
	buildBtn->disable();

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 buildBtnToPosition = hkvVec2(100.0f * scale, buildBtn->GetPosition().y);
	if (animated)
	{
		float absDeltaPositionX = abs(buildBtnToPosition.x - buildBtn->GetPosition().x);
		float maxAmplitude = 178.0f * scale; //from -78.0f to 100.0f (* scale)
		float maxAnimationDuration = 0.2f;
		float animationDuration = maxAnimationDuration * absDeltaPositionX / maxAmplitude;
		buildBtn->translateTo(buildBtnToPosition, animationDuration, delay);
		buildBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, delay);
	}
	else
		buildBtn->SetPosition(buildBtnToPosition);
}

//////////////////////MoveSubMenuPanel//////////////////////
void MoveSubMenuPanel::init(hkvVec2 position)
{
	ScreenMaskPanel::init(position);

	cacheMaxOrbsToMove();
	this->m_iCurrentMovingOrbsCount = m_iMaxOrbsToMove;
	m_bOrbsCountMax = true;
}

void MoveSubMenuPanel::cacheMaxOrbsToMove()
{
	m_iMaxOrbsToMove = 0;

	NodeSphere* actionSphere = GameDialogManager::sharedInstance()->getHUD()->getNodeSphere();
	if (actionSphere)
	{
		vector<Orb*> &alliedOrbs = actionSphere->getOrbsForTeam(TeamManager::sharedInstance().getDefaultTeam());
		for (int orbIndex = 0; orbIndex != alliedOrbs.size(); orbIndex++)
		{
			Orb* alliedOrb = alliedOrbs[orbIndex];
			if (alliedOrb->getStatus() == ORB_STATUS_ORBITING)
				m_iMaxOrbsToMove++;
		}
	}
}

void MoveSubMenuPanel::update(float dt)
{
	ScreenMaskPanel::update(dt);

	cacheMaxOrbsToMove();
	if (m_iMaxOrbsToMove == 0)
		this->m_iCurrentMovingOrbsCount = 0;
	else
	{
		if (m_bOrbsCountMax)
			this->m_iCurrentMovingOrbsCount = m_iMaxOrbsToMove;
		else if (m_bOrbsCountMin)
			this->m_iCurrentMovingOrbsCount = 1;
	}

	updateOrbsCountTextLabel();
}

void MoveSubMenuPanel::updateOrbsCountTextLabel()
{
	PrintTextLabel* orbsCountTextLabel = (PrintTextLabel*) this->findElementById(ID_MOVE_SUB_PANEL_ORBS_COUNT);
	//string text = stringFromInt(m_iCurrentMovingOrbsCount) + string("/") + stringFromInt(m_iMaxOrbsToMove);
	string text = stringFromInt(m_iCurrentMovingOrbsCount);
	orbsCountTextLabel->setText(text);
}

void MoveSubMenuPanel::onSliderMove(float percentage)
{
	if (percentage >= 1)
	{
		m_bOrbsCountMax = true;
		m_bOrbsCountMin = false;
		this->m_iCurrentMovingOrbsCount = m_iMaxOrbsToMove;
	}
	else if (percentage <= 0)
	{
		m_bOrbsCountMax = false;
		m_bOrbsCountMin = true;
		this->m_iCurrentMovingOrbsCount = (m_iMaxOrbsToMove == 0) ? 0 : 1;
	}
	else
	{
		m_bOrbsCountMax = false;
		m_bOrbsCountMin = false;
		this->m_iCurrentMovingOrbsCount = GeometryUtils::round(percentage * m_iMaxOrbsToMove);
		if (m_iMaxOrbsToMove > 0 && this->m_iCurrentMovingOrbsCount == 0)
			this->m_iCurrentMovingOrbsCount = 1;
	}
}

//////////////////////MoveSubMenuSlider//////////////////////
V_IMPLEMENT_DYNCREATE(MoveSubMenuSlider, ScreenMaskPanel, Vision::GetEngineModule());

MoveSubMenuSlider::~MoveSubMenuSlider()
{
	sliderBg = NULL;
	sliderFill = NULL;
	sliderKnob = NULL;
}

void MoveSubMenuSlider::init(hkvVec2 position)
{
	ScreenMaskPanel::init(position);

	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* sliderBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\move_slider_bg.png");
	VTextureObject* sliderFillTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\move_slider_fill.png");
	VTextureObject* sliderKnobTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\move_slider_knob.png");

	float sliderWidth = 64.0f * scale;
	float sliderHeight = 512.0f * scale;
	hkvVec2 sliderSize = hkvVec2(sliderWidth, sliderHeight);

	//slider bg
	sliderBg = new SpriteScreenMask();
	sliderBg->AddRef();
	sliderBg->init(hkvVec2(0,0), sliderSize, sliderBgTexture);
	this->addScreenMask(sliderBg);
	sliderBg->SetOrder(ACTION_PANEL_ORDER + 2);

	//slider fill
	sliderFill = new SpriteScreenMask();
	sliderFill->AddRef();
	hkvVec2 sliderFillPosition = hkvVec2(0, 0.5f * sliderHeight);
	hkvVec4 sliderFillTextureRange = hkvVec4(0, 0, sliderSize.x / scale, sliderSize.y / scale);
	sliderFill->init(sliderFillPosition, sliderSize, sliderFillTextureRange, sliderFillTexture, false);
	sliderFill->m_iID = VGUIManager::GetID(ID_MOVE_SUB_PANEL_SLIDER_FILL);
	sliderFill->SetAnchorPoint(hkvVec2(0.5f, 1.0f));
	this->addScreenMask(sliderFill);
	sliderFill->SetOrder(ACTION_PANEL_ORDER + 1);

	//slider knob
	float sliderKnobWidth = 96.0f * scale;
	float sliderKnobHeight = 96.0f * scale;
	hkvVec2 sliderKnobSize = hkvVec2(sliderKnobWidth, sliderKnobHeight);

	sliderKnob = new SpriteScreenMask();
	sliderKnob->AddRef();
	hkvVec2 sliderKnobPosition = hkvVec2(0, -0.5f * sliderHeight + 25.0f * scale); 
	sliderKnob->init(sliderKnobPosition, sliderKnobSize, sliderKnobTexture);
	this->addScreenMask(sliderKnob);
	sliderKnob->SetOrder(ACTION_PANEL_ORDER);
}

void MoveSubMenuSlider::setKnobPositionForPercentage(float fPercentage)
{
	float fScale = GameDialogManager::sharedInstance()->getScale();

	float sliderFillHeight = 512.0f * fScale;

	hkvVec2 knobMinPosition = this->sliderFill->GetPosition() - hkvVec2(0, 25.0f * fScale);
	hkvVec2 knobMaxPosition = this->sliderFill->GetPosition() - hkvVec2(0, sliderFillHeight) + hkvVec2(0, 25.0f * fScale);

	hkvVec2 knobNewPosition = sliderKnob->GetPosition();
	knobNewPosition.y = fPercentage * (knobMaxPosition.y - knobMinPosition.y) + knobMinPosition.y;
	this->sliderKnob->SetPosition(knobNewPosition);

	float sliderFillTextureWidth = 64.0f;
	float sliderFillTextureHeight = 512.0f;
	sliderFill->SetUnscaledTargetSize(hkvVec2(sliderFillTextureWidth * fScale, fPercentage * sliderFillTextureHeight * fScale));
	sliderFill->IncPosition(hkvVec2(0,0));
	sliderFill->SetTextureRange(0, (1 - fPercentage) * sliderFillTextureHeight, sliderFillTextureWidth, sliderFillTextureHeight);

	MoveSubMenuPanel* moveSubMenu = (MoveSubMenuPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->findElementById(ID_MOVE_SUB_PANEL);
	moveSubMenu->onSliderMove(fPercentage);
}

bool MoveSubMenuSlider::containsTouch(hkvVec2 touchLocation)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	float sliderWidth = 96.0f * scale; //max of slider width and knob width
	float sliderHeight = 654.0f * scale; //512 + 2 * 71
	hkvVec2 sliderTouchArea = hkvVec2(sliderWidth, sliderHeight);

	hkvVec2 absPosition = this->GetAbsPosition();
	hkvVec2 bboxMin = absPosition - anchorPoint.compMul(sliderTouchArea);
	hkvVec2 bboxMax = absPosition + anchorPoint.compMul(sliderTouchArea);
	VRectanglef bbox = VRectanglef(bboxMin, bboxMax);

	return bbox.IsInside(touchLocation);
}

void MoveSubMenuSlider::onPointerDown(hkvVec2 touchLocation)
{
	Touchable::onPointerDown(touchLocation);

	float scale = GameDialogManager::sharedInstance()->getScale();

	hkvVec2 knobAbsPosition = sliderKnob->GetAbsPosition();
	hkvVec2 knobTouchArea = hkvVec2(96.0f * scale, 96.0f * scale);
	hkvVec2 bboxMin = knobAbsPosition - anchorPoint.compMul(knobTouchArea);
	hkvVec2 bboxMax = knobAbsPosition + anchorPoint.compMul(knobTouchArea);
	VRectanglef bbox = VRectanglef(bboxMin, bboxMax);

	if (bbox.IsInside(touchLocation))
		this->knobSelected = true;
}

bool MoveSubMenuSlider::onPointerMove(hkvVec2 touchLocation)
{
	if (!Touchable::onPointerMove(touchLocation))
		return false;

	if (!this->knobSelected)
		return false;

	float scale = GameDialogManager::sharedInstance()->getScale();

	//find min and max values the cursor can reach
	float sliderFillWidth = 64.0f * scale;
	float sliderFillHeight = 512.0f * scale;
	hkvVec2 knobMinPosition = this->sliderFill->GetPosition() - hkvVec2(0, 25.0f * scale);
	hkvVec2 knobMaxPosition = this->sliderFill->GetPosition() - hkvVec2(0, sliderFillHeight) + hkvVec2(0, 25.0f * scale);

	float deltaTouchLocationY = 1.0f * this->deltaTouchLocation.y;
	hkvVec2 knobNewPosition = sliderKnob->GetPosition() + hkvVec2(0, deltaTouchLocationY);
	if (knobNewPosition.y > knobMinPosition.y)
		knobNewPosition.y = knobMinPosition.y;
	else if (knobNewPosition.y < knobMaxPosition.y)
		knobNewPosition.y = knobMaxPosition.y;

	sliderKnob->SetPosition(knobNewPosition);

	float percentage = (knobNewPosition.y - knobMinPosition.y) / (float) (knobMaxPosition.y - knobMinPosition.y);
	MoveSubMenuPanel* moveSubMenu = (MoveSubMenuPanel*) GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->findElementById(ID_MOVE_SUB_PANEL);
	moveSubMenu->onSliderMove(percentage);

	float sliderFillTextureWidth = 64.0f;
	float sliderFillTextureHeight = 512.0f;
	sliderFill->SetUnscaledTargetSize(hkvVec2(sliderFillTextureWidth * scale, percentage * sliderFillTextureHeight * scale));
	sliderFill->IncPosition(hkvVec2(0,0));
	sliderFill->SetTextureRange(0, (1 - percentage) * sliderFillTextureHeight, sliderFillTextureWidth, sliderFillTextureHeight);

	return true;
}

void MoveSubMenuSlider::onPointerUp(hkvVec2 touchLocation)
{
	Touchable::onPointerUp(touchLocation);
	this->knobSelected = false;
}

//////////////////////RallySubMenuPanel//////////////////////
void RallySubMenuPanel::showBtn(const char* id, float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ButtonScreenMask* button = (ButtonScreenMask*) this->findElementById(id);
	button->translateTo(hkvVec2(-64.0f * scale, button->GetPosition().y), 0.2f, delay);
}

void RallySubMenuPanel::hideBtn(const char* id, float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ButtonScreenMask* button = (ButtonScreenMask*) this->findElementById(id);
	button->translateTo(hkvVec2(64.0f * scale, button->GetPosition().y), 0.2f, delay);
}

void RallySubMenuPanel::hideAll(float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ActionButton* validateBtn = (ActionButton*) this->findElementById(ID_RALLY_SUB_PANEL_VALIDATE);
	ActionButton* cancelBtn = (ActionButton*) this->findElementById(ID_RALLY_SUB_PANEL_CANCEL);
	ActionButton* deleteBtn = (ActionButton*) this->findElementById(ID_RALLY_SUB_PANEL_DELETE);
	validateBtn->translateTo(hkvVec2(64.0f * scale, validateBtn->GetPosition().y), 0.2f, delay);
	cancelBtn->translateTo(hkvVec2(64.0f * scale, cancelBtn->GetPosition().y), 0.2f, delay);
	deleteBtn->translateTo(hkvVec2(64.0f * scale, deleteBtn->GetPosition().y), 0.2f, delay);
}

//////////////////////BuildSubMenuPanel//////////////////////
void BuildSubMenuPanel::init(hkvVec2 position)
{
	float scale = GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();

	//position of the build button
	ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
	ActionButton* buildBtn = (ActionButton*) actionPanel->findElementById(ID_BUILD_BTN);
	hkvVec2 buildBtnPosition = buildBtn->GetPosition();
	float buildBtnWidth, buildBtnHeight;
	buildBtn->GetTargetSize(buildBtnWidth, buildBtnHeight);

	//validate and cancel buttons
	hkvVec2 resultBtnSize = hkvVec2(128.0f * scale, 128.0f * scale);
	VTextureObject* validateBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_validate_btn.png");
	VTextureObject* cancelBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_undo_btn.png");
	VTextureObject* deleteBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_delete_btn.png");

	ActionButton* validateBtn = new ActionButton();
	validateBtn->AddRef();
	hkvVec2 validateBtnPosition = hkvVec2(0.5f * (resultBtnSize.x),  buildBtn->GetPosition().y - 368.0f * scale);
	validateBtn->init(validateBtnPosition, resultBtnSize, validateBtnTexture);
	validateBtn->SetColor(V_RGBA_GREEN);
	validateBtn->setEnabledColor(V_RGBA_GREEN);
	validateBtn->setSelectedColor(V_RGBA_GREEN);
	validateBtn->m_iID = VGUIManager::GetID(ID_BUILD_SUB_PANEL_VALIDATE);

	ActionButton* cancelBtn = new ActionButton();
	cancelBtn->AddRef();
	hkvVec2 cancelBtnPosition = hkvVec2(0.5f * resultBtnSize.x, buildBtn->GetPosition().y - 200.0f * scale);
	cancelBtn->init(cancelBtnPosition, resultBtnSize, cancelBtnTexture);
	cancelBtn->SetColor(V_RGBA_RED);
	cancelBtn->setEnabledColor(V_RGBA_RED);
	cancelBtn->setSelectedColor(V_RGBA_RED);
	cancelBtn->m_iID = VGUIManager::GetID(ID_BUILD_SUB_PANEL_CANCEL);

	ScreenMaskPanel::init(hkvVec2(0, 0));
	this->m_iID = VGUIManager::GetID(ID_BUILD_SUB_PANEL);

	this->addScreenMask(validateBtn);
	this->addScreenMask(cancelBtn);
}

void BuildSubMenuPanel::showBtn(const char* id, float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ButtonScreenMask* button = (ButtonScreenMask*) this->findElementById(id);
	button->translateTo(hkvVec2(-64.0f * scale, button->GetPosition().y), 0.2f, delay);
}

void BuildSubMenuPanel::hideBtn(const char* id, float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ButtonScreenMask* button = (ButtonScreenMask*) this->findElementById(id);
	button->translateTo(hkvVec2(64.0f * scale, button->GetPosition().y), 0.2f, delay);
}


void BuildSubMenuPanel::hideAll(float delay)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ActionButton* validateBtn = (ActionButton*) this->findElementById(ID_BUILD_SUB_PANEL_VALIDATE);
	ActionButton* cancelBtn = (ActionButton*) this->findElementById(ID_BUILD_SUB_PANEL_CANCEL);
	validateBtn->translateTo(hkvVec2(64.0f * scale, validateBtn->GetPosition().y), 0.2f, delay);
	cancelBtn->translateTo(hkvVec2(64.0f * scale, cancelBtn->GetPosition().y), 0.2f, delay);
}

//////////////////////GameSpeedSlider//////////////////////
V_IMPLEMENT_DYNCREATE(GameSpeedSlider, Slider, Vision::GetEngineModule());

void GameSpeedSlider::init(hkvVec2 position)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	VTextureObject* trackTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_bg.png");
	VTextureObject* knobTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_knob.png");

	hkvVec2 trackSize = hkvVec2(512.0f * scale, 32.0f * scale);
	hkvVec2 knobSize = hkvVec2(64.0f * scale, 64.0f * scale);

	Slider::init(true, position, trackSize, knobSize, trackTexture, knobTexture);

	//slider bg
	this->sliderTrack->SetOrder(INTERFACE_MENU_ORDER - 1);

	//slider knob
	this->percentage = (GameTimer::sharedInstance().getTimeScaleFactor(false) - 1.0f) / TIME_SCALE_AMPLITUDE;
	float sliderAmplitude = trackSize.x;
	hkvVec2 sliderKnobPosition = hkvVec2((percentage - 0.5f) * sliderAmplitude, 0); 
	this->sliderKnob->SetPosition(sliderKnobPosition);
	this->sliderKnob->SetOrder(INTERFACE_MENU_ORDER - 2);

	//text label
	PrintTextLabel* gameSpeedTextLabel = new PrintTextLabel();
	gameSpeedTextLabel->AddRef();
	hkvVec2 gameSpeedTextPosition = hkvVec2(0, 30.0f * scale);
	string gameSpeedText = StringsXmlHelper::sharedInstance().getStringForTag("pause_menu_game_speed");
	float gameSpeedFontHeight = 35.0f * scale;
	VisFont_cl* gameSpeedFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, gameSpeedFontHeight);
	VColorRef gameSpeedFontColor = VColorRef(183, 255, 247);
	gameSpeedTextLabel->init(gameSpeedTextPosition, gameSpeedText, gameSpeedFont, gameSpeedFontHeight, gameSpeedFontColor);
	this->addTextLabel(gameSpeedTextLabel);
}

bool GameSpeedSlider::containsTouch(hkvVec2 touchLocation)
{
	return Slider::containsTouch(touchLocation);
}

void GameSpeedSlider::onPointerDown(hkvVec2 touchLocation)
{
	return Slider::onPointerDown(touchLocation);
}

bool GameSpeedSlider::onPointerMove(hkvVec2 touchLocation)
{
	//if (!Touchable::onPointerMove(touchLocation))
	//	return false;

	//if (!this->knobSelected)
	//	return false;

	//float scale = GameDialogManager::sharedInstance()->getScale();

	////find min and max values the cursor can reach
	//float sliderWidth = 512.0f * scale;
	//float sliderHeight = 32.0f * scale;
	//hkvVec2 knobMinPosition = this->sliderBg->GetPosition() - hkvVec2(0.5f * sliderWidth, 0) + hkvVec2(25.0f * scale, 0);
	//hkvVec2 knobMaxPosition = this->sliderBg->GetPosition() + hkvVec2(0.5f * sliderWidth, 0) - hkvVec2(0, 25.0f * scale);

	//float deltaTouchLocationX = this->deltaTouchLocation.x;
	//hkvVec2 knobNewPosition = sliderKnob->GetPosition() + hkvVec2(deltaTouchLocationX, 0);
	//if (knobNewPosition.x < knobMinPosition.x)
	//	knobNewPosition.x = knobMinPosition.x;
	//else if (knobNewPosition.x > knobMaxPosition.x)
	//	knobNewPosition.x = knobMaxPosition.x;

	//sliderKnob->SetPosition(knobNewPosition);

	Slider::onPointerMove(touchLocation);
	GameTimer::sharedInstance().setTimeScaleFactor(percentage * TIME_SCALE_AMPLITUDE + 1.0f);

	return true;
}

void GameSpeedSlider::onPointerUp(hkvVec2 touchLocation)
{
	Touchable::onPointerUp(touchLocation);
	this->knobSelected = false;
}

//////////////////////QuitDialogPanel//////////////////////
void QuitDialogPanel::init()
{
	YesNoDialogPanel::init(StringsXmlHelper::sharedInstance().getStringForTag("quit_popup_message"));
}

void QuitDialogPanel::onClickYes()
{
	hkvLog::Error("BACK TO MENU WITH SAVING");
	SceneManager::sharedInstance().loadMainMenu();
}

void QuitDialogPanel::onClickNo()
{
	hkvLog::Error("BACK TO MENU WITHOUT SAVING");
	SceneManager::sharedInstance().loadMainMenu();
}

//////////////////////RetryDialogPanel//////////////////////
void RetryDialogPanel::init()
{
	YesNoDialogPanel::init(StringsXmlHelper::sharedInstance().getStringForTag("retry_popup_message"));
}

void RetryDialogPanel::onClickYes()
{
	SceneManager::sharedInstance().loadLevel(LevelManager::sharedInstance().getCurrentLevel()->getNumber());
}

void RetryDialogPanel::onClickNo()
{
	GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->dismissSecondaryContent();
	GameDialogManager::sharedInstance()->getHUD()->getPauseMenu()->showMainContent(true);
}
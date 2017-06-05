#include "OrbsPluginPCH.h"
#include "MenuManager.h"
#include "GameManager.h"

////////////////////////////////////MenuManager////////////////////////////////////
MenuManager* MenuManager::instance = NULL;

MenuManager::MenuManager(void) : companySplash(NULL),
	headphonesSplash(NULL),
	background(NULL),
	introMenu(NULL),
	levelsMenu(NULL),
	veil(NULL),
	transitioning(false)
{

}

MenuManager::~MenuManager(void)
{
	
}

MenuManager* MenuManager::sharedInstance()
{
	if (!instance)
		instance = new MenuManager();
	return instance;
}

void MenuManager::deInitGUIContext()
{
	SceneGUIManager::deInitGUIContext();

	if (companySplash || headphonesSplash)
		removeSplashScreens();
	if (this->background)
		removeBackground();
	if (this->introMenu)
		removeIntroMenu();
	if (this->levelsMenu)
		removeLevelsMenu();
	veil = NULL;
	m_GUIContext->SetActivate(false);
	V_SAFE_DELETE(instance);
}

void MenuManager::defineScale()
{
	hkvVec2 screenSize = getScreenSize();

	float designScreenWidth = 2048.0f; //width as it was designed for an ipad4 retina screen
	float designScreenHeight = 1536.0f; //height as it was designed for an ipad4 retina screen
	float scaleX = screenSize.x / designScreenWidth;
	float scaleY = screenSize.y / designScreenHeight;
	scale = min(scaleX, scaleY); //take the smaller scale
}

vector<PrintTextLabel*> MenuManager::retrieveAllTextLabels()
{
	vector<PrintTextLabel*> allTextLabels;
	if (introMenu)
	{
		allTextLabels = introMenu->retrieveAllTextLabels();
	}
	else if (levelsMenu)
	{
		allTextLabels = levelsMenu->retrieveAllTextLabels();
	}
	return allTextLabels;
}

bool MenuManager::clickContainedInGUI(hkvVec2 clickCoords)
{
	return true;
}

bool MenuManager::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	if (introMenu)
		this->introMenu->processPointerEvent(touchLocation, eventType);
	else if (levelsMenu)
		this->levelsMenu->processPointerEvent(touchLocation, eventType);
	return true;
}

void MenuManager::performToggleOptionsAction(MidPanelButton* sender)
{
	this->introMenu->performToggleOptionsAction(sender);
}

void MenuManager::performToggleCreditsAction(MidPanelButton* sender)
{
	this->introMenu->performToggleCreditsAction(sender);
}

void MenuManager::performShowAchievementsAcion(MidPanelButton* sender)
{
	this->introMenu->performShowAchievementsAction(sender);
}

void MenuManager::showSplashScreens()
{
	//textures
	VTextureObject* companyLogoTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\paralyze_logo.png");
	VTextureObject* companyTextTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\paralyze_title.png");
	VTextureObject* headphonesTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\headphones_required_512.png");

	//dimensions
	float companyLogoTextureWidth = 700 * scale;
	float companyLogoTextureHeight = 700 * scale;
	hkvVec2 companyLogoTextureSize = hkvVec2(companyLogoTextureWidth, companyLogoTextureHeight);
	float companyTextTextureWidth = 700 * scale;
	float companyTextTextureHeight = 175 * scale;
	hkvVec2 companyTextTextureSize = hkvVec2(companyTextTextureWidth, companyTextTextureHeight);
	float headphonesTextureWidth = 768 * scale;
	float headphonesTextureHeight = 768 * scale;
	hkvVec2 headphonesTextureSize = hkvVec2(headphonesTextureWidth, headphonesTextureHeight);

	//masks
	companySplash =  new SplashScreen();
	headphonesSplash =  new SplashScreen();
	companySplash->init(4.0f, headphonesSplash);
	headphonesSplash->init(4.0f, NULL);

	SpriteScreenMask* companyLogo = new SpriteScreenMask();
	companyLogo->AddRef();
	companyLogo->init(hkvVec2(0, -96.0f * scale), companyLogoTextureSize, companyLogoTexture);
	companySplash->addScreenMask(companyLogo);

	SpriteScreenMask* companyText = new SpriteScreenMask();
	companyText->AddRef();
	companyText->init(hkvVec2(0, 320.0f * scale), companyTextTextureSize, companyTextTexture);
	companySplash->addScreenMask(companyText);

	SpriteScreenMask* headphones = new SpriteScreenMask();
	headphones->AddRef();
	headphones->init(hkvVec2(0,0), headphonesTextureSize, headphonesTexture);
	headphonesSplash->addScreenMask(headphones);

	companySplash->launch();	
}

void MenuManager::removeSplashScreens()
{
	companySplash = NULL;
	headphonesSplash = NULL;
}

void MenuManager::removeSplashScreen(SplashScreen* splashScreen)
{
	if (splashScreen == this->companySplash)
		this->companySplash = NULL;
	else if (splashScreen == this->headphonesSplash)
		this->headphonesSplash = NULL;
}

void MenuManager::removeVeil()
{
	veil = NULL;
	this->transitioning = false;
}

void MenuManager::showBackground()
{
	return;
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	VTextureObject* backgroundTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	background = new SpriteScreenMask();
	hkvVec4 textureRange = hkvVec4(0, 0, (float) screenSize.x / (scale * MID_PANEL_SCALE), (float) screenSize.y / (scale * MID_PANEL_SCALE));
	background->init(0.5f * screenSize, screenSize, textureRange, backgroundTexture, true);
	background->SetColor(VColorRef(6, 12, 18));
	background->SetOrder(BACKGROUND_ORDER);
}

void MenuManager::removeBackground()
{
	background = NULL;
}

void MenuManager::showIntroMenu()
{
	if (!background)
		showBackground();
	introMenu = new IntroMenu();
	introMenu->init();
}

void MenuManager::removeIntroMenu()
{
	introMenu->removeSelf();
	introMenu = NULL;
}

void MenuManager::showLevelsMenu()
{
	if (!background)
		showBackground();

	levelsMenu = new LevelsMenu();
	levelsMenu->init();
}

void MenuManager::removeLevelsMenu()
{
	levelsMenu->removeSelf();
	levelsMenu = NULL;
}

void MenuManager::showResetDialog()
{
	float scale = MenuManager::sharedInstance()->getScale();

	dialogHolder = new InterfaceMenu();
	dialogHolder->init();
	
	ResetDialogPanel* resetDialogPanel = new ResetDialogPanel();
	resetDialogPanel->AddRef();
	resetDialogPanel->init();
	resetDialogPanel->SetPosition(hkvVec2(0,0));
	//resetDialogPanel->SetOrder(-10);
	dialogHolder->addPanel(resetDialogPanel);

	dialogHolder->show(hkvVec2(768.0f * scale, 384.0f *scale), false, false);

	resetDialogPanel->SetOpacity(0);
	resetDialogPanel->fadeTo(1.0f, 0.3f, 0.5f);
}

void MenuManager::dismissDialogHolder()
{
	dialogHolder = NULL;
}

void MenuManager::introToLevels()
{
	this->removeIntroMenu();
	this->showLevelsMenu();
}

void MenuManager::levelsToIntro()
{
	this->removeLevelsMenu();
	this->showIntroMenu();
}

void MenuManager::menuTransition(bool introToLevels)
{
	this->transitioning = true;

	veil = new TransitionVeil();

	CallFuncObject* callFuncObject;
	if (introToLevels)
		callFuncObject = new CallFuncObject(MenuManager::sharedInstance(), (CallFunc)(&MenuManager::introToLevels), 0.0f);
	else
		callFuncObject = new CallFuncObject(MenuManager::sharedInstance(), (CallFunc)(&MenuManager::levelsToIntro), 0.0f);

	veil->init(V_RGBA_BLACK, callFuncObject);
	veil->SetOrder(VEIL_ORDER);
	veil->SetOpacity(0);
	veil->launch();

	//fade TextLabels as they 
	if (introToLevels)
	{
		//fadeOutIntroMenu();
		//delay the fade in of the next screen
		/*CallFuncObject* callFuncObject = new CallFuncObject(this, (CallFunc)(&MenuManager::fadeInLevelsMenu), 1.0f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(callFuncObject);
		callFuncObject->executeFunc();*/
	}
	else
	{
		//fadeOutLevelsMenu();
		//delay the fade in of the next screen
		/*CallFuncObject* callFuncObject = new CallFuncObject(this, (CallFunc)(&MenuManager::fadeInIntroMenu), 1.0f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(callFuncObject);
		callFuncObject->executeFunc();*/
	}
}

void MenuManager::startLevel()
{
	this->levelsMenu->startLevel();
}

////////////////////////////////////IntroMenu////////////////////////////////////
IntroMenu::~IntroMenu()
{
	
}

void IntroMenu::init()
{
	this->showMidPanel();	
	//this->showOptionsPanel(); //tmp debug
	this->showTitle();
}

void IntroMenu::showMidPanel()
{
	midPanel = new ScreenMaskPanel();

	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	midPanel->init(0.5f * screenSize);

	//textures
	VTextureObject* bubbleBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\bubble_bg.png");
	VTextureObject* bubbleHeartTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\bubble_heart.png");
	VTextureObject* circle1Texture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\circle1.png");
	VTextureObject* circle2Texture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\circle2.png");

	//texture dimensions
	float circleWidth = 512 * scale * MID_PANEL_SCALE;
	float circleHeight = 512 * scale * MID_PANEL_SCALE;
	hkvVec2 circleTextureSize = hkvVec2((float) circleWidth, (float) circleHeight);
	float bubbleBgWidth = 512 * scale * MID_PANEL_SCALE;
	float bubbleBgHeight = 512 * scale * MID_PANEL_SCALE;
	hkvVec2 bubbleBgTextureSize = hkvVec2(bubbleBgWidth, bubbleBgHeight);
	float bubbleHeartWidth = 512 * scale * MID_PANEL_SCALE;
	float bubbleHeartHeight = 512 * scale * MID_PANEL_SCALE;
	hkvVec2 bubbleHeartTextureSize = hkvVec2(bubbleHeartWidth, bubbleHeartHeight);

	//create masks and add them to panel
	SpriteScreenMask* bubbleBg = new SpriteScreenMask();
	SpriteScreenMask* bubbleHeart = new SpriteScreenMask();
	RotatingSpriteScreenMask* circle1 = new RotatingSpriteScreenMask();
	RotatingSpriteScreenMask* circle2 = new RotatingSpriteScreenMask();
	bubbleBg->AddRef();
	bubbleHeart->AddRef();
	circle1->AddRef();
	circle2->AddRef();

	//all circles have same texture size and same position
	hkvVec2 circlePos = hkvVec2(0, 0);
	
	bubbleBg->init(circlePos, bubbleBgTextureSize, bubbleBgTexture);
	bubbleHeart->init(circlePos, bubbleHeartTextureSize, bubbleHeartTexture);
	circle1->init(circlePos, circleTextureSize, circle1Texture, 10, hkvVec2(0,0));
	circle2->init(circlePos, circleTextureSize, circle2Texture, -10, hkvVec2(0,0));
	circle1->SetOrder(MENU_CIRCLES_ORDER);
	circle2->SetOrder(MENU_CIRCLES_ORDER);
	bubbleBg->SetOrder(BUBBLE_BG_ORDER);
	bubbleHeart->SetOrder(BUBBLE_HEART_ORDER);

	midPanel->addScreenMask(bubbleBg);
	midPanel->addScreenMask(bubbleHeart);
	midPanel->addScreenMask(circle1);
	midPanel->addScreenMask(circle2);

	bubbleBg->SetOpacity(0.0f);
	bubbleHeart->SetOpacity(0.0f);
	circle1->SetOpacity(0.0f);
	circle2->SetOpacity(0.0f);

	bubbleBg->SetScale(hkvVec2(0,0));
	bubbleHeart->SetScale(hkvVec2(0,0));
	circle1->SetScale(hkvVec2(0,0));
	circle2->SetScale(hkvVec2(0,0));

	bubbleBg->fadeTo(1.0f, 0.2f, 1.00f);
	circle1->fadeTo(1.0f, 0.2f, 1.09f);
	circle2->fadeTo(1.0f, 0.2f, 1.18f);
	bubbleHeart->fadeTo(1.0f, 0.2f, 1.27f);

	bubbleBg->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 1.00f);
	circle1->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 1.09f);
	circle2->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 1.18f);
	bubbleHeart->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 1.27f);

	//set these masks with fixed rotation
	bubbleBg->setFixedRotation(false);
	bubbleHeart->setFixedRotation(true);
	circle1->setFixedRotation(false);
	circle2->setFixedRotation(false);

	//Buttons
	ScreenMaskPanel* optionsBtnPanel = new ScreenMaskPanel();
	ScreenMaskPanel* trophyBtnPanel = new ScreenMaskPanel();
	ScreenMaskPanel* creditsBtnPanel = new ScreenMaskPanel();
	MidPanelButton* playBtn = new MidPanelButton();
	MidPanelButton* optionsBtn = new MidPanelButton();
	MidPanelButton* creditsBtn = new MidPanelButton();
	MidPanelButton* achievementsBtn = new MidPanelButton();
	SpriteScreenMask* optionsOverlay = new SpriteScreenMask();
	SpriteScreenMask* trophyOverlay = new SpriteScreenMask();
	SpriteScreenMask* creditsOverlay = new SpriteScreenMask();
	optionsBtnPanel->AddRef();
	trophyBtnPanel->AddRef();
	creditsBtnPanel->AddRef();
	playBtn->AddRef();
	optionsBtn->AddRef();
	creditsBtn->AddRef();
	achievementsBtn->AddRef();
	optionsOverlay->AddRef();
	trophyOverlay->AddRef();
	creditsOverlay->AddRef();

	//Load buttons textures
	VTextureObject* sideButtonDisabledTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\disabled_button.png");
	VTextureObject* sideButtonEnabledTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\enabled_button.png");
	VTextureObject* optionsOverlayTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_options.png");
	VTextureObject* trophyOverlayTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_trophy.png");
	VTextureObject* creditsOverlayTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_credits.png");
	
	//Calculate the buttons position, size and rotation
	float midPanelBtnWidth = 256.0f * scale * MID_PANEL_BUTTONS_SCALE;
	float midPanelBtnHeight = 256.0f * scale * MID_PANEL_BUTTONS_SCALE;
	hkvVec2 midPanelBtnSize = hkvVec2(midPanelBtnWidth, midPanelBtnHeight);
	float overlayWidth = 128.0f * scale * MID_PANEL_BUTTONS_SCALE;
	float overlayHeight = 128.0f * scale * MID_PANEL_BUTTONS_SCALE;
	hkvVec2 overlaySize = hkvVec2(overlayWidth, overlayHeight);

	float optionsBtnAngle = 60.0f;
	hkvVec2 optionsPanelPosition = hkvVec2(0, -0.5f * bubbleBgHeight + 110.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(optionsPanelPosition, hkvVec2(0,0), optionsBtnAngle);

	float creditsBtnAngle = -60.0f;
	hkvVec2 creditsPanelPosition = hkvVec2(0, -0.5f * bubbleBgHeight + 110.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(creditsPanelPosition, hkvVec2(0,0), creditsBtnAngle);

	float achievementsBtnAngle = 180.0f;
	hkvVec2 trophyPanelPosition = hkvVec2(0, -0.5f * bubbleBgHeight + 110.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(trophyPanelPosition, hkvVec2(0,0), achievementsBtnAngle);

	//init the buttons content, set correct order and id
	playBtn->init(hkvVec2(0, 0), hkvVec2(200,200), NULL);
	optionsBtnPanel->init(optionsPanelPosition);
	trophyBtnPanel->init(trophyPanelPosition);
	creditsBtnPanel->init(creditsPanelPosition);
	optionsBtn->init(hkvVec2(0, 0), midPanelBtnSize, sideButtonDisabledTexture, hkvVec2(1.0f, 1.0f), optionsBtnAngle);
	creditsBtn->init(hkvVec2(0, 0), midPanelBtnSize, sideButtonDisabledTexture, hkvVec2(1.0f, 1.0f), creditsBtnAngle);
	achievementsBtn->init(hkvVec2(0, 0), midPanelBtnSize, sideButtonDisabledTexture, hkvVec2(1.0f, 1.0f), achievementsBtnAngle);
	hkvVec2 optionsOverlayPosition = hkvVec2(0, 18.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(optionsOverlayPosition, hkvVec2(0,0), optionsBtnAngle);
	hkvVec2 trophyOverlayPosition = hkvVec2(0, 10.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(trophyOverlayPosition, hkvVec2(0,0), achievementsBtnAngle);
	hkvVec2 creditsOverlayPosition = hkvVec2(0, 15.0f / 256.0f * midPanelBtnHeight);
	GeometryUtils::rotate2DPoint(creditsOverlayPosition, hkvVec2(0,0), creditsBtnAngle);
	optionsOverlay->init(optionsOverlayPosition, overlaySize, optionsOverlayTexture);
	trophyOverlay->init(trophyOverlayPosition, overlaySize, trophyOverlayTexture);
	creditsOverlay->init(creditsOverlayPosition, overlaySize, creditsOverlayTexture);
	
	midPanel->addScreenMask(playBtn);
	optionsBtnPanel->addScreenMask(optionsBtn);
	optionsBtnPanel->addScreenMask(optionsOverlay);
	trophyBtnPanel->addScreenMask(achievementsBtn);
	trophyBtnPanel->addScreenMask(trophyOverlay);
	creditsBtnPanel->addScreenMask(creditsBtn);
	creditsBtnPanel->addScreenMask(creditsOverlay);
	midPanel->addPanel(optionsBtnPanel);
	midPanel->addPanel(trophyBtnPanel);
	midPanel->addPanel(creditsBtnPanel);

	optionsBtnPanel->SetOpacity(0.0f);
	creditsBtnPanel->SetOpacity(0.0f);
	trophyBtnPanel->SetOpacity(0.0f);

	playBtn->SetOrder(PLAY_BUTTON_ORDER);
	optionsBtn->SetOrder(SIDE_BUTTONS_ORDER);
	creditsBtn->SetOrder(SIDE_BUTTONS_ORDER);
	achievementsBtn->SetOrder(SIDE_BUTTONS_ORDER);
	optionsOverlay->SetOrder(SIDE_BUTTONS_ORDER - 1);
	trophyOverlay->SetOrder(SIDE_BUTTONS_ORDER - 1);
	creditsOverlay->SetOrder(SIDE_BUTTONS_ORDER - 1);

	optionsOverlay->setFixedRotation(true);
	creditsOverlay->setFixedRotation(true);
	trophyOverlay->setFixedRotation(true);

	playBtn->m_iID = VGUIManager::GetID(ID_PLAY);
	optionsBtn->m_iID = VGUIManager::GetID(ID_OPTIONS);
	creditsBtn->m_iID = VGUIManager::GetID(ID_CREDITS);
	achievementsBtn->m_iID = VGUIManager::GetID(ID_ACHIEVEMENTS);

	//fade in and translate buttons
	optionsBtnPanel->fadeTo(1.0f, 0.05f, 1.5f);
	creditsBtnPanel->fadeTo(1.0f, 0.05f, 1.5f);
	trophyBtnPanel->fadeTo(1.0f, 0.05f, 1.5f);

	float translationLength = 200.0f / 256.0f * midPanelBtnHeight;

	hkvVec2 optionsPanelTranslationDirection = hkvVec2(-sqrt(3.0f) / 2.0f, -0.5f);
	hkvVec2 optionsPanelToPosition = optionsPanelPosition + optionsPanelTranslationDirection * translationLength;
	optionsBtnPanel->translateTo(optionsPanelToPosition, 0.3f, 1.5f);

	hkvVec2 creditsPanelTranslationDirection = hkvVec2(sqrt(3.0f) / 2.0f, -0.5f);
	hkvVec2 creditsPanelToPosition = creditsPanelPosition + creditsPanelTranslationDirection * translationLength;
	creditsBtnPanel->translateTo(creditsPanelToPosition, 0.3f, 1.6f);

	hkvVec2 trophyPanelTranslationDirection = hkvVec2(0, 1);
	hkvVec2 trophyPanelToPosition = trophyPanelPosition + trophyPanelTranslationDirection * translationLength;
	trophyBtnPanel->translateTo(trophyPanelToPosition, 0.3f, 1.7f);

	//build a fountain of orbs in the center of the mid panel
	//OrbFountain* fountain = new OrbFountain();
	//fountain->AddRef();
	//float fountainRadius = floorf(150.0f * scale);
	//fountain->init(hkvVec2(0,0), fountainRadius, 50);
	//midPanel->addFountain(fountain);

	//CallFuncObject* showOrbFountainAction = new CallFuncObject(fountain, (CallFunc)(&OrbFountain::startGeneratingOrbs), 2.0f);
	//CallFuncHandler::sharedInstance().addCallFuncInstance(showOrbFountainAction);
	//showOrbFountainAction->executeFunc();

	//Finally the play text label
	//VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fontHeight);;
	//float fontHeight = 80 * scale;
	//VColorRef fontColor = V_RGBA_WHITE;

	//PlayTextLabel* playTextLabel = new PlayTextLabel();
	//playTextLabel->AddRef();
	//playTextLabel->init(hkvVec2(0,0), "PLAY", neuropolFont, fontHeight, fontColor);
	//midPanel->addTextLabel(playTextLabel);
}

void IntroMenu::removeMidPanel(bool animated)
{
	if (!animated)
	{
		midPanel = NULL;
	}
	else
	{
		//TODO scale animation and release on callback
	}
}

void IntroMenu::showTitle()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	titlePanel = new ScreenMaskPanel();
	titlePanel->init(hkvVec2(0.5f * screenSize.x, 188.0f / 1536.0f * screenSize.y));

	VTextureObject* bracketTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	//brackets
	float bracketHeight = 168.0f * scale;
	float bracketWidth = 16.0f * scale;
	float lineThickness = 5.0f * scale;
	VColorRef bracketColor = VColorRef(216, 251, 255);
	//left
	ScreenMaskPanel* leftBracket = new ScreenMaskPanel();
	leftBracket->AddRef();
	leftBracket->init(hkvVec2(0,0));
	
	SpriteScreenMask* verticalLine = new SpriteScreenMask();
	verticalLine->AddRef();
	verticalLine->init(hkvVec2(-0.5f * (bracketWidth - lineThickness), 0), hkvVec2(lineThickness, bracketHeight), bracketTexture);
	SpriteScreenMask* horizontalLine1 = new SpriteScreenMask();
	horizontalLine1->AddRef();
	horizontalLine1->init(hkvVec2(0, -0.5f * (bracketHeight - lineThickness)), hkvVec2(bracketWidth, lineThickness), bracketTexture);
	SpriteScreenMask* horizontalLine2 = new SpriteScreenMask();
	horizontalLine2->AddRef();
	horizontalLine2->init(hkvVec2(0, 0.5f * (bracketHeight - lineThickness)), hkvVec2(bracketWidth, lineThickness), bracketTexture);
	
	verticalLine->SetColor(bracketColor);
	horizontalLine1->SetColor(bracketColor);
	horizontalLine2->SetColor(bracketColor);

	leftBracket->addScreenMask(verticalLine);
	leftBracket->addScreenMask(horizontalLine1);
	leftBracket->addScreenMask(horizontalLine2);

	//right
	ScreenMaskPanel* rightBracket = new ScreenMaskPanel();
	rightBracket->AddRef();
	rightBracket->init(hkvVec2(0,0));
	
	verticalLine = new SpriteScreenMask();
	verticalLine->AddRef();
	verticalLine->init(hkvVec2(0.5f * (bracketWidth - lineThickness), 0), hkvVec2(lineThickness, bracketHeight), bracketTexture);
	horizontalLine1 = new SpriteScreenMask();
	horizontalLine1->AddRef();
	horizontalLine1->init(hkvVec2(0, -0.5f * (bracketHeight - lineThickness)), hkvVec2(bracketWidth, lineThickness), bracketTexture);
	horizontalLine2 = new SpriteScreenMask();
	horizontalLine2->AddRef();
	horizontalLine2->init(hkvVec2(0, 0.5f * (bracketHeight - lineThickness)), hkvVec2(bracketWidth, lineThickness), bracketTexture);
	
	verticalLine->SetColor(bracketColor);
	horizontalLine1->SetColor(bracketColor);
	horizontalLine2->SetColor(bracketColor);

	rightBracket->addScreenMask(verticalLine);
	rightBracket->addScreenMask(horizontalLine1);
	rightBracket->addScreenMask(horizontalLine2);

	//add them to parent title panel
	titlePanel->addPanel(leftBracket);
	titlePanel->addPanel(rightBracket);

	//initial opacity and scale
	leftBracket->SetOpacity(0.0f);
	rightBracket->SetOpacity(0.0f);
	leftBracket->SetScale(hkvVec2(1.0f, 0.0f));
	rightBracket->SetScale(hkvVec2(1.0f, 0.0f));

	//scale and translate
	leftBracket->fadeTo(1.0f, 0.05f, 2.0f);
	rightBracket->fadeTo(1.0f, 0.05f, 2.0f);
	leftBracket->scaleTo(hkvVec2(1.0f, 1.0f), 0.05f, 2.0f);
	rightBracket->scaleTo(hkvVec2(1.0f, 1.0f), 0.05f, 2.0f);
	
	hkvVec2 leftBracketTranslatePos = leftBracket->GetPosition() + hkvVec2(-270.0f * scale, 0);
	leftBracket->translateTo(leftBracketTranslatePos, 0.2f, 2.05f);

	hkvVec2 rightBracketTranslatePos = rightBracket->GetPosition() + hkvVec2(270.0f * scale, 0);
	rightBracket->translateTo(rightBracketTranslatePos, 0.2f, 2.05f);
	
	//Actual title
	VTextureObject* titleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\title\\title.png");
	float titleWidth = 512.0f * scale;
	float titleHeight = 128.0f * scale;
	hkvVec2 titleSize = hkvVec2(titleWidth, titleHeight);

	SpriteScreenMask* title = new SpriteScreenMask();
	title->AddRef();
	title->init(hkvVec2(0,0), titleSize, titleTexture);

	titlePanel->addScreenMask(title);

	title->SetOpacity(0.0f);
	title->fadeTo(1.0f, 1.0f, 2.05f);
}

void IntroMenu::removeTitle(bool animated)
{
	if (!animated)
		this->titlePanel = NULL;
}

void IntroMenu::showOptionsPanel()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	//sound
	AudioManager::sharedInstance().playWindowOpeningSound();

	//OPTIONS
	optionsPanel = new ScreenMaskPanel();
	optionsPanel->init(hkvVec2(250.0f  / 2048.0f * screenSize.x , 0.5f * screenSize.y));

	//textures
	VTextureObject* bracketTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\options_menu_bracket.png");
	VTextureObject* bgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\options_menu_bg_slice.png");
	VTextureObject* musicOnOverlay = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png");
	VTextureObject* musicOffOverlay = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png");
	VTextureObject* soundOnOverlay = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png");
	VTextureObject* soundOffOverlay = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png");
	VTextureObject* resetOverlay = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_reset.png");

	//size and position
	float bracketWidth = 256.0f * scale * OPTIONS_PANEL_SCALE;
	float bracketHeight = 128.0f * scale * OPTIONS_PANEL_SCALE;
	hkvVec2 bracketSize = hkvVec2(bracketWidth, bracketHeight);

	SpriteScreenMask* topBracket = new SpriteScreenMask();
	SpriteScreenMask* bottomBracket = new SpriteScreenMask();
	SpriteScreenMask* bgColor = new SpriteScreenMask();
	topBracket->AddRef();
	bottomBracket->AddRef();
	bgColor->AddRef();

	hkvVec2 topBracketPosition = hkvVec2(0 , -0.5f *  bracketHeight);
	hkvVec2 bottomBracketPosition = hkvVec2(0 , 0.5f * bracketHeight);
	topBracket->init(topBracketPosition, bracketSize, bracketTexture);
	bottomBracket->init(bottomBracketPosition, bracketSize, bracketTexture, hkvVec2(1.0f, 1.0f), 180);

	hkvVec2 bgColorPosition = hkvVec2(0, 0);
	hkvVec2 bgColorUnscaledSize = hkvVec2(bracketSize.x, 604.0f / 1536.0f * screenSize.y);
	hkvVec4 creditsBgTextureRange = hkvVec4(0, 0, bgColorUnscaledSize.x / (scale * OPTIONS_PANEL_SCALE), bgColorUnscaledSize.y / (scale * OPTIONS_PANEL_SCALE));
	bgColor->init(bgColorPosition, bgColorUnscaledSize, creditsBgTextureRange, bgTexture, true);

	optionsPanel->addScreenMask(topBracket);
	optionsPanel->addScreenMask(bottomBracket);
	optionsPanel->addScreenMask(bgColor);

	bgColor->SetScale(hkvVec2(1.0f, 0.0f));

	bottomBracket->SetOrder(OPTIONS_BG_ORDER);
	topBracket->SetOrder(OPTIONS_BG_ORDER);
	bgColor->SetOrder(OPTIONS_BG_ORDER);

	topBracket->m_iID = VGUIManager::GetID(ID_OPTIONS_TOP_BRACKET);
	bottomBracket->m_iID = VGUIManager::GetID(ID_OPTIONS_BOTTOM_BRACKET);
	bgColor->m_iID = VGUIManager::GetID(ID_OPTIONS_BG);

	optionsPanel->SetOpacity(0.0f);

	//animations
	optionsPanel->fadeTo(1.0f, 0.1f, 0.0f);

	hkvVec2 topBracketToPosition = topBracket->GetPosition() - hkvVec2(0, 302.0f / 1536.0f * screenSize.y);
	hkvVec2 bottomBracketToPosition = bottomBracket->GetPosition() + hkvVec2(0, 302.0f / 1536.0f * screenSize.y);
	topBracket->translateTo(topBracketToPosition, 0.3f, 0.1f);
	bottomBracket->translateTo(bottomBracketToPosition, 0.3f, 0.1f);
	bgColor->scaleTo(hkvVec2(1.0f, 1.0f), 0.3f, 0.1f);

	//options buttons
	float overlayBtnWidth = 140.0f * scale * OPTIONS_PANEL_SCALE;
	float overlayBtnHeight = 140.0f * scale * OPTIONS_PANEL_SCALE;
	hkvVec2 overlayBtnSize = hkvVec2(overlayBtnWidth, overlayBtnHeight);

	OptionsButton* musicOverlayBtn = new OptionsButton();
	OptionsButton* soundOverlayBtn = new OptionsButton();
	OptionsButton* resetOverlayBtn = new OptionsButton();
	musicOverlayBtn->AddRef();
	soundOverlayBtn->AddRef();
	resetOverlayBtn->AddRef();

	float bgColorFinalHeight = bgColorUnscaledSize.y;
	hkvVec2 musicOverlayPosition = hkvVec2(0, -300.0f / 712.0f * bgColorFinalHeight);
	hkvVec2 soundOverlayPosition = hkvVec2(0, -22.0f / 712.0f * bgColorFinalHeight);
	hkvVec2 resetOverlayPosition = hkvVec2(0, 267.0f / 712.0f * bgColorFinalHeight);
	bool musicActive = PersistentDataManager::sharedInstance().isMusicActive();
	bool soundActive = PersistentDataManager::sharedInstance().isSoundActive();
	musicOverlayBtn->init(musicOverlayPosition, overlayBtnSize, musicActive ? musicOnOverlay : musicOffOverlay);
	soundOverlayBtn->init(soundOverlayPosition, overlayBtnSize, soundActive ? soundOnOverlay : soundOffOverlay);
	resetOverlayBtn->init(resetOverlayPosition, overlayBtnSize, resetOverlay);

	optionsPanel->addScreenMask(musicOverlayBtn);
	optionsPanel->addScreenMask(soundOverlayBtn);
	optionsPanel->addScreenMask(resetOverlayBtn);

	musicOverlayBtn->SetScale(hkvVec2(0,0));
	soundOverlayBtn->SetScale(hkvVec2(0,0));
	resetOverlayBtn->SetScale(hkvVec2(0,0));
	musicOverlayBtn->SetOpacity(0);
	soundOverlayBtn->SetOpacity(0);
	resetOverlayBtn->SetOpacity(0);

	musicOverlayBtn->SetOrder(OPTIONS_OVERLAYS_ORDER);
	soundOverlayBtn->SetOrder(OPTIONS_OVERLAYS_ORDER);
	resetOverlayBtn->SetOrder(OPTIONS_OVERLAYS_ORDER);

	musicOverlayBtn->m_iID = VGUIManager::GetID(ID_OPTIONS_MUSIC);
	soundOverlayBtn->m_iID = VGUIManager::GetID(ID_OPTIONS_SOUND);
	resetOverlayBtn->m_iID = VGUIManager::GetID(ID_OPTIONS_RESET);

	musicOverlayBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.25f);
	soundOverlayBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.1f);
	resetOverlayBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.25f);
	musicOverlayBtn->fadeTo(1.0f, 0.15f, 0.25f);
	soundOverlayBtn->fadeTo(1.0f, 0.15f, 0.1f);
	resetOverlayBtn->fadeTo(1.0f, 0.15f, 0.25f);

	//options buttons texts
	PrintTextLabel* musicTextLabel = new PrintTextLabel();
	PrintTextLabel* soundTextLabel = new PrintTextLabel();
	PrintTextLabel* resetTextLabel = new PrintTextLabel();
	musicTextLabel->AddRef();
	soundTextLabel->AddRef();
	resetTextLabel->AddRef();

	string musicText = StringsXmlHelper::sharedInstance().getStringForTag("options_music");
	string soundText = StringsXmlHelper::sharedInstance().getStringForTag("options_sound");
	string resetText = StringsXmlHelper::sharedInstance().getStringForTag("options_reset");
	float verticalOffset = 0.0f;
	float fontHeight = 36 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	VRectanglef musicTextLabelDim;
	VRectanglef soundTextLabelDim;
	VRectanglef resetTextLabelDim;
	neuropolFont->GetTextDimension(musicText.c_str(), musicTextLabelDim);
	neuropolFont->GetTextDimension(soundText.c_str(), soundTextLabelDim);
	neuropolFont->GetTextDimension(resetText.c_str(), resetTextLabelDim);
	float fontScale = fontHeight / neuropolFont->GetFontHeight();
	musicTextLabelDim.operator*=(scale);
	soundTextLabelDim.operator*=(scale);
	resetTextLabelDim.operator*=(scale);
	hkvVec2 musicTextLabelPosition = hkvVec2(0, musicOverlayBtn->GetPosition().y + 0.5f * (overlayBtnSize.y + musicTextLabelDim.GetSizeY()) +  verticalOffset);
	hkvVec2 soundTextLabelPosition = hkvVec2(0, soundOverlayBtn->GetPosition().y + 0.5f * (overlayBtnSize.y + soundTextLabelDim.GetSizeY()) +  verticalOffset);
	hkvVec2 resetTextLabelPosition = hkvVec2(0, resetOverlayBtn->GetPosition().y + 0.5f * (overlayBtnSize.y + resetTextLabelDim.GetSizeY()) +  verticalOffset);
	musicTextLabel->init(musicTextLabelPosition, musicText.c_str(), neuropolFont, fontHeight, VColorRef(120, 236, 232));
	soundTextLabel->init(soundTextLabelPosition, soundText.c_str(), neuropolFont, fontHeight, VColorRef(120, 236, 232));
	resetTextLabel->init(resetTextLabelPosition, resetText.c_str(), neuropolFont, fontHeight, VColorRef(120, 236, 232));

	musicTextLabel->setRenderedBehindVeils(true);
	soundTextLabel->setRenderedBehindVeils(true);
	resetTextLabel->setRenderedBehindVeils(true);

	optionsPanel->addTextLabel(musicTextLabel);
	optionsPanel->addTextLabel(soundTextLabel);
	optionsPanel->addTextLabel(resetTextLabel);

	musicTextLabel->SetScale(hkvVec2(0.0f, 0.0f));
	soundTextLabel->SetScale(hkvVec2(0.0f, 0.0f));
	resetTextLabel->SetScale(hkvVec2(0.0f, 0.0f));

	musicTextLabel->m_iID = VGUIManager::GetID(ID_OPTIONS_MUSIC_TEXT);
	soundTextLabel->m_iID = VGUIManager::GetID(ID_OPTIONS_SOUND_TEXT);
	resetTextLabel->m_iID = VGUIManager::GetID(ID_OPTIONS_RESET_TEXT);

	musicTextLabel->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.25f);
	soundTextLabel->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.1f);
	resetTextLabel->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.25f);
	
	////LINES
	//VTextureObject* lineTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	//float optionsBtnWidth = floorf(256 * MID_PANEL_SCALE);
	//float optionsBtnHeight = floorf(256 * MID_PANEL_SCALE);
	//MidPanelButton* optionsButton = (MidPanelButton*) midPanel->findElementById(ID_OPTIONS);

	//hkvVec2 optionsButtonPosition = optionsButton->GetAbsPosition();
	//hkvVec2 point1 = optionsButtonPosition - hkvVec2(0.5f * optionsBtnWidth, 0.3f * optionsBtnHeight);
	//hkvVec2 point2 = hkvVec2(0.5f * (optionsButtonPosition.x + optionsPanel->GetAbsPosition().x), point1.y);
	//hkvVec2 point3 = hkvVec2(optionsPanel->GetAbsPosition().x + bracketWidth, optionsPanel->GetAbsPosition().y);

	//SpriteScreenMask* line1 = new SpriteScreenMask();
	//line1->AddRef();
	//hkvVec2 line1Size = hkvVec2((point2 - point1).getLength(), 4.0f * scale);
	//line1->init(point1, line1Size, lineTexture, hkvVec2(0.0f, 1.0f), 0, 1.0f, hkvVec2(1.0f, 0.5f));
	//line1->SetColor(V_RGBA_WHITE);
	//lines.push_back(line1);

	//line1->scaleTo(hkvVec2(1.0f, 1.0f), 0.4f, 0.0f);
}

void IntroMenu::removeOptions()
{
	this->optionsPanel = NULL;
}

void IntroMenu::dismissOptionsPanel()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	//bg
	SpriteScreenMask* topBracket = (SpriteScreenMask*) optionsPanel->findElementById(ID_OPTIONS_TOP_BRACKET);
	SpriteScreenMask* bottomBracket = (SpriteScreenMask*) optionsPanel->findElementById(ID_OPTIONS_BOTTOM_BRACKET);
	SpriteScreenMask* bgColor = (SpriteScreenMask*) optionsPanel->findElementById(ID_OPTIONS_BG);

	hkvVec2 topBracketToPosition = topBracket->GetPosition() + hkvVec2(0, 302.0f / 1536.0f * screenSize.y);
	hkvVec2 bottomBracketToPosition = bottomBracket->GetPosition() - hkvVec2(0, 302.0f / 1536.0f * screenSize.y);
	topBracket->translateTo(topBracketToPosition, 0.3f, 0.0f);
	bottomBracket->translateTo(bottomBracketToPosition, 0.3f, 0.0f);
	bgColor->scaleTo(hkvVec2(1.0f, 0.0f), 0.3f, 0.0f);
	topBracket->fadeTo(0.0f, 0.3f, 0.0f);
	bottomBracket->fadeTo(0.0f, 0.3f, 0.0f);
	bgColor->fadeTo(0.0f, 0.3f, 0.0f);

	//buttons
	OptionsButton* musicBtn = (OptionsButton*) optionsPanel->findElementById(ID_OPTIONS_MUSIC);
	OptionsButton* soundBtn = (OptionsButton*) optionsPanel->findElementById(ID_OPTIONS_SOUND);
	OptionsButton* resetBtn = (OptionsButton*) optionsPanel->findElementById(ID_OPTIONS_RESET);

	musicBtn->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.0f);
	soundBtn->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.10f);
	resetBtn->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.0f);
	musicBtn->fadeTo(0.0f, 0.15f, 0.0f);
	soundBtn->fadeTo(0.0f, 0.15f, 0.10f);
	resetBtn->fadeTo(0.0f, 0.15f, 0.0f);

	optionsPanel->fadeTo(0.0f, 0.1f, 0.3f);

	//texts
	PrintTextLabel* musicTextLabel = (PrintTextLabel*) optionsPanel->findElementById(ID_OPTIONS_MUSIC_TEXT);
	PrintTextLabel* soundTextLabel = (PrintTextLabel*) optionsPanel->findElementById(ID_OPTIONS_SOUND_TEXT);
	PrintTextLabel* resetTextLabel = (PrintTextLabel*) optionsPanel->findElementById(ID_OPTIONS_RESET_TEXT);

	musicTextLabel->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.0f);
	soundTextLabel->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.10f);
	resetTextLabel->scaleTo(hkvVec2(0.0f, 0.0f), 0.15f, 0.0f);
	musicTextLabel->fadeTo(0.0f, 0.15f, 0.0f);
	soundTextLabel->fadeTo(0.0f, 0.15f, 0.10f);
	resetTextLabel->fadeTo(0.0f, 0.15f, 0.0f);

	//remove the options panel from scene
	CallFuncObject* removeOptionsPanelAction = new CallFuncObject(this, (CallFunc)(&IntroMenu::removeOptions), 0.4f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeOptionsPanelAction);
	removeOptionsPanelAction->executeFunc();
}

void IntroMenu::showCredits()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	//move the mid panel
	float toAngle = midPanel->GetAngle() - 30;
	hkvVec2 toPosition = hkvVec2(538.0f / 2048.0f * screenSize.x, 0.5f * screenSize.y);
	midPanel->rotateTo(toAngle, 0.3f, 0.0f);
	midPanel->translateTo(toPosition, 0.3f, 0.0f);

	//load textures
	VTextureObject* creditsBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\credits_bg_color.png");
	VTextureObject* creditsContourTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\credits_bg_contour.png");

	//size and position
	float contourWidth = 8.0f * scale;
	float contourHeight = 64.0f * scale;
	hkvVec2 contourSize = hkvVec2(contourWidth, contourHeight);

	creditsPanel = new ScreenMaskPanel();
	hkvVec2 creditsPosition = hkvVec2(1293.0f / 2048.0f * screenSize.x, 0.5f * screenSize.y);
	creditsPanel->init(creditsPosition);

	SpriteScreenMask* creditsBg = new SpriteScreenMask();
	SpriteScreenMask* creditsContourTop = new SpriteScreenMask();
	SpriteScreenMask* creditsContourBottom = new SpriteScreenMask();
	creditsBg->AddRef();
	creditsContourTop->AddRef();
	creditsContourBottom->AddRef();

	hkvVec2 creditsBgPosition = hkvVec2(0, 0);
	hkvVec2 creditsBgUnscaledTargetSize = hkvVec2(1510.0f / 2048.0f * screenSize.x, 502.0f * scale * MID_PANEL_SCALE);
	hkvVec4 creditsBgTextureRange = hkvVec4(0, 0, creditsBgUnscaledTargetSize.x, creditsBgUnscaledTargetSize.y);
	creditsBg->init(creditsBgPosition, creditsBgUnscaledTargetSize, creditsBgTextureRange, creditsBgTexture, true);
	creditsBg->SetOrder(BUBBLE_BG_ORDER + 2);
	creditsBg->m_iID = VGUIManager::GetID(ID_CREDITS_BG);

	hkvVec2 creditsContourPosition = hkvVec2(0, 0);
	hkvVec2 creditsContourUnscaledTargetSize = hkvVec2(1510.0f / 2048.0f * screenSize.x, contourSize.y);
	hkvVec4 creditsContourTextureRange = hkvVec4(0, 0, creditsContourUnscaledTargetSize.x, 64);
	creditsContourTop->init(creditsContourPosition, creditsContourUnscaledTargetSize, creditsContourTextureRange, creditsContourTexture, true);
	creditsContourBottom->init(creditsContourPosition, creditsContourUnscaledTargetSize, creditsContourTextureRange, creditsContourTexture, true, hkvVec2(1.0f, 1.0f), 180);
	creditsContourTop->SetOrder(BUBBLE_BG_ORDER + 1);
	creditsContourBottom->SetOrder(BUBBLE_BG_ORDER + 1);
	creditsContourTop->m_iID = VGUIManager::GetID(ID_CREDITS_CONTOUR_TOP);
	creditsContourBottom->m_iID = VGUIManager::GetID(ID_CREDITS_CONTOUR_BOTTOM);

	creditsPanel->addScreenMask(creditsBg);
	creditsPanel->addScreenMask(creditsContourTop);
	creditsPanel->addScreenMask(creditsContourBottom);

	creditsBg->SetScale(hkvVec2(1.0f,0.0f));
	creditsContourTop->SetOpacity(0);
	creditsContourBottom->SetOpacity(0);
	
	hkvVec2 creditsContourTopToPosition = hkvVec2(0, -0.5f * creditsBgUnscaledTargetSize.y + 26.0f / 64.0f * contourSize.y);
	hkvVec2 creditsContourBottomToPosition = hkvVec2(0, 0.5f * creditsBgUnscaledTargetSize.y - 26.0f / 64.0f * contourSize.y);
	creditsContourTop->translateTo(creditsContourTopToPosition, 0.3f, 0.3f);
	creditsContourBottom->translateTo(creditsContourBottomToPosition, 0.3f, 0.3f);
	creditsContourTop->fadeTo(1.0f, 0.2f, 0.3f);
	creditsContourBottom->fadeTo(1.0f, 0.2f, 0.3f);
	creditsBg->scaleTo(hkvVec2(1.0f, 1.0f), 0.3f, 0.3f);

	//build scrolling list
	EndCredits* endCredits = new EndCredits();
	endCredits->AddRef();
	hkvVec2 endCreditsPosition = hkvVec2(260.0f / 2048.0f * screenSize.x, 0);
	float endCreditsWidth = 900.0f * scale;
	float endCreditsHeight = 500.0f * scale;
	endCredits->init(endCreditsPosition, endCreditsWidth, endCreditsHeight, true, 50);
	endCredits->m_iID = VGUIManager::GetID(ID_CREDITS_LIST);
	creditsPanel->addList(endCredits);

	//lines
	vector<CreditsLine> creditsStrings = StringsXmlHelper::sharedInstance().getCredits();

	for (int creditsIndex = 0; creditsIndex != creditsStrings.size(); creditsIndex++)
	{
		CreditsLine creditsLine = creditsStrings[creditsIndex];
		string type = creditsLine.type;
		if (!type.compare(CREDITS_LINE_TYPE_IMG))
		{
			string imgPath = creditsLine.path;
			float imgWidth = creditsLine.width * scale;
			float imgHeight = creditsLine.height * scale;
			hkvVec2 imgSize = hkvVec2(imgWidth, imgHeight);

			VTextureObject* imgTexture = Vision::TextureManager.Load2DTexture(imgPath.c_str());
			SpriteScreenMask* img = new SpriteScreenMask();
			img->init(hkvVec2(0,0), imgSize, imgTexture);
			endCredits->addMaskEntry(img, hkvVec2(0, 60.0f * scale));
		}
		else if (!type.compare(CREDITS_LINE_TYPE_TEXT))
		{
			string text = creditsLine.text;
			int dropLines = creditsLine.dropLines;
			float offset = (creditsIndex == 0) ? endCredits->getSize().y : 60.0f * scale * dropLines;
			float fontHeight = creditsLine.fontSize * scale;
			VisFont_cl* font = FontManager::getFontForNameAndSize(creditsLine.fontID, fontHeight);
			VColorRef fontColor = creditsLine.color;

			PrintTextLabel* textEntry = new PrintTextLabel();
			textEntry->init(hkvVec2(0,0), text.c_str(), font, fontHeight, fontColor);
			endCredits->addTextEntry(textEntry, hkvVec2(0, offset));
		}
	}	
}

void IntroMenu::removeCredits(bool animated)
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	//move the mid panel
	if (this->midPanel)
	{
		hkvVec2 toPosition = 0.5f * screenSize;
		midPanel->rotateTo(360, 0.3f, 0.0f);
		midPanel->translateTo(toPosition, 0.3f, 0.0f);
	}

	if (!animated)
		creditsPanel = NULL;
	else
	{
		SpriteScreenMask* creditsBg = (SpriteScreenMask*) creditsPanel->findElementById(ID_CREDITS_BG);
		SpriteScreenMask* creditsContourTop = (SpriteScreenMask*) creditsPanel->findElementById(ID_CREDITS_CONTOUR_TOP);
		SpriteScreenMask* creditsContourBottom = (SpriteScreenMask*) creditsPanel->findElementById(ID_CREDITS_CONTOUR_BOTTOM);
		EndCredits* endCredits = (EndCredits*) creditsPanel->findElementById(ID_CREDITS_LIST);

		creditsBg->scaleTo(hkvVec2(1.0f, 0.0f), 0.3f, 0.0f);
		hkvVec2 creditsContourToPosition = hkvVec2(0, 0);
		creditsContourTop->translateTo(creditsContourToPosition, 0.3f, 0.0f);
		creditsContourBottom->translateTo(creditsContourToPosition, 0.3f, 0.0f);
		creditsContourTop->fadeTo(0.0f, 0.2f, 0.0f);
		creditsContourBottom->fadeTo(0.0f, 0.2f, 0.0f);
		creditsPanel->removeList(endCredits);

		CallFuncObject* callFuncObject = new CallFuncObject(this, (CallFunc)(&IntroMenu::removeCreditsMasksFromScene), 0.3f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(callFuncObject);
		callFuncObject->executeFunc();
	}
}

void IntroMenu::removeCreditsMasksFromScene()
{
	creditsPanel = NULL;
}

void IntroMenu::buildAchievementsPanel()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	achievementsPanel = new ScreenMaskPanel();
	achievementsPanel->init(hkvVec2(0.5f * screenSize.x, 1.5f * screenSize.y));

	VTextureObject* backBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_back_btn.png");
	
	float backBtnWidth = 256.0f * scale;
	float backBtnHeight = 256.0f * scale;
	hkvVec2 backBtnSize = hkvVec2(backBtnWidth, backBtnHeight);

	//back buttons
	BackButton* backBtn1 = new BackButton();
	backBtn1->AddRef();
	hkvVec2 backBtn1Position = hkvVec2(-0.5f * screenSize.x + 200.0f * scale, -0.5f * screenSize.y + 200.0f * scale);
	backBtn1->init(backBtn1Position, backBtnSize, backBtnTexture, hkvVec2(1.0f, 1.0f), -90);

	BackButton* backBtn2 = new BackButton();
	backBtn2->AddRef();
	hkvVec2 backBtn2Position = hkvVec2(0.5f * screenSize.x - 200.0f * scale, -0.5f * screenSize.y + 200.0f * scale);
	backBtn2->init(backBtn2Position, backBtnSize, backBtnTexture, hkvVec2(1.0f, 1.0f), -90);

	achievementsPanel->addScreenMask(backBtn1);
	achievementsPanel->addScreenMask(backBtn2);

	backBtn1->m_iID = VGUIManager::GetID(ID_BACK_FROM_ACHIEVEMENTS_1);
	backBtn2->m_iID = VGUIManager::GetID(ID_BACK_FROM_ACHIEVEMENTS_1);

	//achievements
	AchievementList* achievementsList = new AchievementList();
	achievementsList->AddRef();
	float achievementLineWidth = screenSize.x;
	float achievementLineHeight = 256 * scale;
	hkvVec2 achievementsLineSize = hkvVec2(achievementLineWidth, achievementLineHeight);
	hkvVec2 achievementsListPosition = hkvVec2(0, 0.1f * screenSize.y);
	achievementsList->init(achievementsListPosition, screenSize.x, 4 * achievementsLineSize.y, achievementsLineSize);
	
	achievementsPanel->addList(achievementsList);

	AchievementManager::sharedInstance().buildAchievementsFromXML();
	vector<Achievement*> achievements = AchievementManager::sharedInstance().getAchievements();
	ScreenMaskPanel* achievementLine = NULL;
	for (int achievementIndex = 0; achievementIndex != achievements.size(); achievementIndex++)
	{
		Achievement* achievement = achievements[achievementIndex];
		AchievementPanel* achievementPanel = new AchievementPanel();
		VTextureObject* achievementIcon = AchievementManager::sharedInstance().getAchievementIconForTag(achievement->getTag());
		achievementPanel->init(achievement->getTag(), hkvVec2(0,0), achievement->getTitle(), achievement->getDescription(), achievementIcon);

		int page = achievementIndex / 4;
		achievementsList->addPanelEntry(achievementPanel, achievementsLineSize, page, hkvVec2(0,0));
	}

	//arrows
	ItemListArrows* rightArrows = new ItemListArrows(); 
	ItemListArrows* leftArrows = new ItemListArrows();
	rightArrows->AddRef();
	leftArrows->AddRef();

	hkvVec2 leftArrowsPosition = hkvVec2(-86.0f / 1536.0f * screenSize.x, achievementsListPosition.y - 2.25f * 256.0f * scale);
	hkvVec2 rightArrowsPosition = hkvVec2(86.0f / 1536.0f * screenSize.x, achievementsListPosition.y - 2.25f * 256.0f * scale);
	leftArrows->init(achievementsList, ARROWS_DIRECTION_LEFT, leftArrowsPosition);
	rightArrows->init(achievementsList, ARROWS_DIRECTION_RIGHT, rightArrowsPosition);

	achievementsPanel->addPanel(leftArrows);
	achievementsPanel->addPanel(rightArrows);

	leftArrows->disable();
	
	rightArrows->m_iID = VGUIManager::GetID(ID_ACHIEVEMENTS_ARROWS_RIGHT);
	leftArrows->m_iID = VGUIManager::GetID(ID_ACHIEVEMENTS_ARROWS_LEFT);

	//page count
	PrintTextLabel* pageCountTextLabel = new PrintTextLabel();
	pageCountTextLabel->AddRef();
	string pageCountText = string("1/2");
	float arrowWidth = 64 * scale; //hardcode this, correct value is set in ItemListArrows::init()
	hkvVec2 pageCountPosition = 0.5f * (leftArrowsPosition + rightArrowsPosition) - hkvVec2(0, 0.1f * arrowWidth);
	float fontHeight = 50 *scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	pageCountTextLabel->init(pageCountPosition, pageCountText, neuropolFont, fontHeight, V_RGBA_WHITE);
	pageCountTextLabel->m_iID = VGUIManager::GetID(ID_ACHIEVEMENTS_PAGE_COUNT);

	achievementsPanel->addTextLabel(pageCountTextLabel);
}

void IntroMenu::removeAchievements()
{
	achievementsPanel = NULL;
}

void IntroMenu::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	//first check click on eventual dialog (reset/exit)
	InterfaceMenu* dialogHolder = MenuManager::sharedInstance()->getDialogHolder();
	if (dialogHolder)
	{
		panelProcessPointerEvent(dialogHolder, touchLocation, eventType);
		return;
	}

	//check click on mid panel buttons
	if (midPanel)
	{
		if (panelProcessPointerEvent(midPanel, touchLocation, eventType))
			return;
	}

	//check click on options panel
	if (optionsPanel)
	{
		if (panelProcessPointerEvent(optionsPanel, touchLocation, eventType))
			return;
	}

	//check click on achievementsPanel
	if (achievementsPanel)
	{
		if (panelProcessPointerEvent(achievementsPanel, touchLocation, eventType))
			return;
	}

	//check click on endCredits
	if (creditsPanel)
	{
		EndCredits* endCredits = (EndCredits*) creditsPanel->findElementById(ID_CREDITS_LIST);
		if (endCredits)
			endCredits->trigger(touchLocation, eventType);
	}
}

bool IntroMenu::panelProcessPointerEvent(ScreenMaskPanel* panel, hkvVec2 touchLocation, int eventType)
{
	if (eventType == POINTER_MOVE)
		return false;
	//check direct child masks
	vector<SpriteScreenMask*> screenMasks = panel->getScreenMasks();
	for (int maskIndex = 0; maskIndex != screenMasks.size(); maskIndex++)
	{
		SpriteScreenMask* mask = screenMasks[maskIndex];
		if (vdynamic_cast<ButtonScreenMask*>(mask))
		//if (mask->isClickable())
		{
			if (((ButtonScreenMask*)mask)->trigger(touchLocation, eventType))
				return true;
		}
	}

	//check child panels
	vector<ScreenMaskPanel*> panels = panel->getPanels();
	for (int panelIndex = 0; panelIndex != panels.size(); panelIndex++)
	{
		ScreenMaskPanel* childPanel = panels[panelIndex];
		if (vdynamic_cast<ItemListArrows*>(childPanel))
		//if (childPanel->isClickable())
		{
			if (((ItemListArrows*)childPanel)->trigger(touchLocation, eventType))
				return true;
		}
		if (panelProcessPointerEvent(childPanel, touchLocation, eventType))
			return true;
	}

	return false;
}

void IntroMenu::performToggleOptionsAction(MidPanelButton* sender)
{
	if (!sender->isActive())
	{
		sender->setActive(true);

		//show options menu
		this->showOptionsPanel();

		//remove credits
		if (this->creditsPanel)
		{
			removeCredits(true);

			MidPanelButton* creditsBtn = (MidPanelButton*) midPanel->findElementById(ID_CREDITS);
			creditsBtn->setActive(false);

			//move the mid panel
			hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
			hkvVec2 toPosition = 0.5f * screenSize;
			midPanel->rotateTo(360, 0.3f, 0.0f);
			midPanel->translateTo(toPosition, 0.3f, 0.0f);
		}
	}
	else
	{
		sender->setActive(false);

		//remove options menu
		this->dismissOptionsPanel();
	}
}

void IntroMenu::performToggleCreditsAction(MidPanelButton* sender)
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	if (!sender->isActive())
	{
		sender->setActive(true);

		//close options if open
		if (this->optionsPanel)
		{
			dismissOptionsPanel();
			MidPanelButton* optionsBtn = (MidPanelButton*) midPanel->findElementById(ID_OPTIONS);
			optionsBtn->setActive(false);

			CallFuncObject* showCreditsAction = new CallFuncObject(this, (CallFunc)(&IntroMenu::showCredits), 0.25f);
			CallFuncHandler::sharedInstance().addCallFuncInstance(showCreditsAction);
			showCreditsAction->executeFunc();
		}
		else 
			this->showCredits();
	}
	else
	{
		sender->setActive(false);

		//close credits
		this->removeCredits(true);
	}
}

void IntroMenu::performShowAchievementsAction(MidPanelButton* sender)
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	//create achievements Panel
	buildAchievementsPanel();

	//translate all content
	if (titlePanel)
	{
		hkvVec2 translatePosition = titlePanel->GetAbsPosition() + hkvVec2(0, -screenSize.y);
		titlePanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (midPanel)
	{
		hkvVec2 translatePosition = midPanel->GetAbsPosition() + hkvVec2(0, -screenSize.y);
		midPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (optionsPanel)
	{
		hkvVec2 translatePosition = optionsPanel->GetAbsPosition() + hkvVec2(0, -screenSize.y);
		optionsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (creditsPanel)
	{
		hkvVec2 translatePosition = creditsPanel->GetAbsPosition() + hkvVec2(0, -screenSize.y);
		creditsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (achievementsPanel)
	{
		hkvVec2 translatePosition = achievementsPanel->GetAbsPosition() + hkvVec2(0, -screenSize.y);
		achievementsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}
}

void IntroMenu::performCloseAchievementsAction()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	if (titlePanel)
	{
		hkvVec2 translatePosition = titlePanel->GetAbsPosition() + hkvVec2(0, screenSize.y);
		titlePanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (midPanel)
	{
		hkvVec2 translatePosition = midPanel->GetAbsPosition() + hkvVec2(0, screenSize.y);
		midPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (optionsPanel)
	{
		hkvVec2 translatePosition = optionsPanel->GetAbsPosition() + hkvVec2(0, screenSize.y);
		optionsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (creditsPanel)
	{
		hkvVec2 translatePosition = creditsPanel->GetAbsPosition() + hkvVec2(0, screenSize.y);
		creditsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	if (achievementsPanel)
	{
		hkvVec2 translatePosition = achievementsPanel->GetAbsPosition() + hkvVec2(0, screenSize.y);
		achievementsPanel->translateTo(translatePosition, 0.5f, 0.0f); 
	}

	CallFuncObject* removeAchievementsAction = new CallFuncObject(this, (CallFunc)(&IntroMenu::removeAchievements), 0.5f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeAchievementsAction);
	removeAchievementsAction->executeFunc();
}

vector<PrintTextLabel*> IntroMenu::retrieveAllTextLabels()
{
	vector<PrintTextLabel*> allTextLabels;
	if (midPanel)
	{
		vector<PrintTextLabel*> midPanelTextLabel = midPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), midPanelTextLabel.begin(), midPanelTextLabel.end());
	}
	if (titlePanel)
	{
		vector<PrintTextLabel*> titlePanelTextLabel = titlePanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), titlePanelTextLabel.begin(), titlePanelTextLabel.end());
	}
	if (optionsPanel)
	{
		vector<PrintTextLabel*> optionsPanelTextLabel = optionsPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), optionsPanelTextLabel.begin(), optionsPanelTextLabel.end());
	}
	if (achievementsPanel)
	{
		vector<PrintTextLabel*> achievementsPanelTextLabel = achievementsPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), achievementsPanelTextLabel.begin(), achievementsPanelTextLabel.end());
	}
	if (creditsPanel)
	{
		vector<PrintTextLabel*> creditsPanelTextLabel = creditsPanel->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), creditsPanelTextLabel.begin(), creditsPanelTextLabel.end());
	}

	return allTextLabels;
}

void IntroMenu::removeSelf()
{
	this->removeMidPanel(false);
	this->removeOptions();
	this->removeTitle(false);
	this->removeCredits(false);
	this->removeAchievements();
}

////////////////////////////////////MidPanelButton////////////////////////////////////
void MidPanelButton::setActive(bool active)
{
	this->active = active;

	VTextureObject* newTexture;
	if (active)
		newTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\enabled_button.png");
	else
		newTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\disabled_button.png");

	updateTexture(newTexture);
}

void MidPanelButton::onClick()
{
	//switch over the id and perform the relevant action
	if (this->m_iID == VGUIManager::GetID(ID_PLAY))
	{
		MenuManager::sharedInstance()->menuTransition(true);
	}
	else if (this->m_iID == VGUIManager::GetID(ID_OPTIONS))
	{
		hkvLog::Error(">>>>>>>>>>>>>>onClick OPTIONS_BTN_CLICKED");
		MenuManager::sharedInstance()->performToggleOptionsAction(this);
	}
	else if (this->m_iID == VGUIManager::GetID(ID_CREDITS))
	{
		hkvLog::Error(">>>>>>>>>>>>>>onClick CREDITS_BTN_CLICKED");
		MenuManager::sharedInstance()->performToggleCreditsAction(this);
	}
	else if (this->m_iID == VGUIManager::GetID(ID_ACHIEVEMENTS))
	{
		hkvLog::Error(">>>>>>>>>>>>>>onClick ACHIEVEMENTS_BTN_CLICKED");
		MenuManager::sharedInstance()->performShowAchievementsAcion(this);
	}
}

////////////////////////////////////OptionsButton////////////////////////////////////
void OptionsButton::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_OPTIONS_MUSIC))
	{
		bool music = PersistentDataManager::sharedInstance().toggleMusic();
		const char* texturePath = music ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png";
		VTextureObject* musicTexture = Vision::TextureManager.Load2DTexture(texturePath);
		this->updateTexture(musicTexture);
		AudioManager::sharedInstance().toggleMusic();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_OPTIONS_SOUND))
	{
		bool sound = PersistentDataManager::sharedInstance().toggleSound();
		const char* texturePath = sound ? "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png" : "GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png";
		VTextureObject* soundTexture = Vision::TextureManager.Load2DTexture(texturePath);
		this->updateTexture(soundTexture);
		AudioManager::sharedInstance().toggleSound();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_OPTIONS_RESET))
	{
		MenuManager::sharedInstance()->showResetDialog();
	}
}

////////////////////////////////////EndCredits////////////////////////////////////
void EndCredits::init(hkvVec2 position, float width, float height, bool autoscroll, float autoscrollSpeed)
{
	ScrollingList::init(position, width, height, autoscroll, autoscrollSpeed);
}

void EndCredits::adjustOpacity(PrintTextLabel* label)
{
	if (MenuManager::sharedInstance()->isTransitioning())
		return;

	hkvVec2 labelPos = label->getAbsPos();
	float distanceToCenter = labelPos.y - this->GetAbsPosition().y;
	float opacity = abs(distanceToCenter) / (0.5f * this->getSize().y);
	if (opacity > 1)
		opacity = 0;
	else
		opacity = (float) cos(2 * M_PI / 4.0f * opacity);
	hkvVec4 labelColor = label->getFontColor().getAsVec4();
	labelColor.w = opacity;
	label->setFontColor(VColorRef::Float_To_RGBA(labelColor));
}

void EndCredits::adjustOpacity(SpriteScreenMask* mask)
{
	if (MenuManager::sharedInstance()->isTransitioning())
		return;

	float maskPositionY = mask->GetPosition().y;
	float distanceToCenter = maskPositionY - this->GetAbsPosition().y;
	float opacity = abs(distanceToCenter) / (0.5f * this->getSize().y);
	if (opacity > 1)
		opacity = 0;
	else
		opacity = (float) cos(2 * M_PI / 4.0f * opacity);
	hkvVec4 maskColor = mask->GetColor().getAsVec4();
	maskColor.w = opacity;
	mask->SetColor(VColorRef::Float_To_RGBA(maskColor));
}

void EndCredits::scroll(float dx)
{
	PrintTextLabel* firstEntry = texts[0]; //first entry is a text, not a mask
	hkvVec2 parentRelativePosition = firstEntry->getAbsPos() - this->getPos();
	float distanceFromTop = parentRelativePosition.y + dx;
	float scrollingListLength = this->height;
	float contentLength = this->totalLength - scrollingListLength;
	if (distanceFromTop > scrollingListLength)
	{
		distanceFromTop = scrollingListLength;
		dx = distanceFromTop - parentRelativePosition.y;
	}
	else if (distanceFromTop < -contentLength)
	{
		distanceFromTop = -contentLength;
		dx = distanceFromTop - parentRelativePosition.y;
	}	

	if (distanceFromTop >= -contentLength && distanceFromTop <= scrollingListLength) //limit the scroll
		IncChildsPosition(hkvVec2(0, dx));
}



////////////////////////////////////LevelsMenu////////////////////////////////////
void LevelsMenu::init()
{
	this->showFountainAndBrackets();
	this->showLevelsList();

	CallFuncObject*  showListArrowsAction = new CallFuncObject(this, (CallFunc)(&LevelsMenu::showListArrows), 0.85f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showListArrowsAction);
	showListArrowsAction->executeFunc();

	CallFuncObject* showButtonsAction  = new CallFuncObject(this, (CallFunc)(&LevelsMenu::showButtons), 0.85f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showButtonsAction);
	showButtonsAction->executeFunc();
}

void LevelsMenu::showFountainAndBrackets()
{
	AudioManager::sharedInstance().playLevelsMenuIntroSound();

	float scale = MenuManager::sharedInstance()->getScale();
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	VTextureObject* fountainCircleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_fountain_circle.png");
	VTextureObject* fountainBubbleTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_fountain_bubble.png");

	float firstCircleWidth = 256 * scale;
	float firstCircleHeight = 256 * scale;
	float secondCircleWidth = 220 * scale;
	float secondCircleHeight = 220 * scale;
	float bubbleWidth = 220 * scale;
	float bubbleHeight = 220 * scale;

	fountainPanel = new ScreenMaskPanel();
	hkvVec2 fountainPanelPosition = hkvVec2(250.0f * scale, 0.75f * screenSize.y);
	fountainPanel->init(fountainPanelPosition);

	RotatingSpriteScreenMask* fountainCircle1 = new RotatingSpriteScreenMask();
	fountainCircle1->AddRef();
	fountainCircle1->init(hkvVec2(0,0), hkvVec2(firstCircleWidth, firstCircleHeight), fountainCircleTexture, 30);
	fountainPanel->addScreenMask(fountainCircle1);
	fountainCircle1->SetScale(hkvVec2(0,0));

	RotatingSpriteScreenMask* fountainCircle2 = new RotatingSpriteScreenMask();
	fountainCircle2->AddRef();
	fountainCircle2->init(hkvVec2(0,0), hkvVec2(secondCircleWidth, secondCircleHeight), fountainCircleTexture, -30);
	fountainPanel->addScreenMask(fountainCircle2);
	fountainCircle2->SetScale(hkvVec2(0,0));

	SpriteScreenMask* bubble = new SpriteScreenMask();
	bubble->AddRef();
	bubble->init(hkvVec2(0,0), hkvVec2(bubbleWidth, bubbleHeight), fountainBubbleTexture);
	fountainPanel->addScreenMask(bubble);
	bubble->SetScale(hkvVec2(0,0));

	fountainCircle1->SetOrder(BUBBLE_BG_ORDER);
	fountainCircle2->SetOrder(BUBBLE_BG_ORDER);
	bubble->SetOrder(BUBBLE_BG_ORDER);

	OrbFountain* orbFountain = new OrbFountain();
	orbFountain->AddRef();
	orbFountain->init(hkvVec2(0,0), 50.0f * scale, 50);
	fountainPanel->addFountain(orbFountain);

	CallFuncObject* showOrbFountainAction = new CallFuncObject(orbFountain, (CallFunc)(&OrbFountain::startGeneratingOrbs), 2.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(showOrbFountainAction);
	showOrbFountainAction->executeFunc();

	//open previous created circles
	fountainCircle1->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.05f);
	fountainCircle2->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.05f);
	bubble->scaleTo(hkvVec2(1.0f, 1.0f), 0.2f, 0.05f);

	//LINES (6 lines)
	lines.reserve(5); //6 lines in total
	VTextureObject* lineTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	SpriteScreenMask* line1 = new SpriteScreenMask();
	line1->AddRef();
	hkvVec2 line1Position = hkvVec2(0, 0.75f * screenSize.y);
	hkvVec2 line1Size = hkvVec2(fountainPanelPosition.x - 0.65f * firstCircleWidth, 4.0f * scale);
	line1->init(line1Position, line1Size, lineTexture, hkvVec2(0.0f, 1.0f), 0, 1.0f, hkvVec2(0.0f, 0.5f));
	line1->SetColor(V_RGBA_WHITE);
	lines.push_back(line1);

	SpriteScreenMask* line2 = new SpriteScreenMask();
	line2->AddRef();
	hkvVec2 line2Position = fountainPanelPosition - hkvVec2(0, 0.65f * firstCircleHeight);
	hkvVec2 line2Size = hkvVec2(4.0f * scale, line2Position.y - 0.5f * screenSize.y);
	line2->init(line2Position, line2Size, lineTexture, hkvVec2(1.0f, 0.0f), 0, 1.0f, hkvVec2(0.5f, 1.0f));
	line2->SetColor(V_RGBA_WHITE);
	lines.push_back(line2);

	SpriteScreenMask* line3 = new SpriteScreenMask();
	line3->AddRef();
	hkvVec2 line3Position = hkvVec2(fountainPanelPosition.x, 0.5f * screenSize.y);
	hkvVec2 line3Size = hkvVec2(0.5f * screenSize.x - 0.55f * 1024.0f * scale - fountainPanelPosition.x, 4.0f * scale);
	line3->init(line3Position, line3Size, lineTexture, hkvVec2(0.0f, 1.0f), 0, 1.0f, hkvVec2(0.0f, 0.5f));
	line3->SetColor(V_RGBA_WHITE);
	lines.push_back(line3);

	hkvVec2 startBtnPosition = hkvVec2(screenSize.x - 200.0f * scale, 200.0f * scale);
	SpriteScreenMask* line4 = new SpriteScreenMask();
	line4->AddRef();
	hkvVec2 line4Position = hkvVec2(0.5f * screenSize.x + 0.55f * 1024.0f * scale, 0.5f * screenSize.y);
	hkvVec2 line4Size = hkvVec2(startBtnPosition.x - line4Position.x , 4.0f * scale);
	line4->init(line4Position, line4Size, lineTexture, hkvVec2(0.0f, 1.0f), 0, 1.0f, hkvVec2(0.0f, 0.5f));
	line4->SetColor(V_RGBA_WHITE);
	lines.push_back(line4);

	SpriteScreenMask* line5 = new SpriteScreenMask();
	line5->AddRef();
	hkvVec2 line5Position = hkvVec2(startBtnPosition.x, 0.5f * screenSize.y);
	hkvVec2 line5Size = hkvVec2(4.0f * scale, 0.67f * (startBtnPosition.y - line5Position.y));
	line5->init(line5Position, line5Size, lineTexture, hkvVec2(1.0f, 0.0f), 0, 1.0f, hkvVec2(0.5f, 0.0f));
	line5->SetColor(V_RGBA_WHITE);
	lines.push_back(line5);

	line1->SetOrder(LEVELS_BUTTONS_ORDER);
	line2->SetOrder(LEVELS_BUTTONS_ORDER);
	line3->SetOrder(LEVELS_BUTTONS_ORDER);
	line4->SetOrder(LEVELS_BUTTONS_ORDER);
	line5->SetOrder(LEVELS_BUTTONS_ORDER);

	line1->scaleTo(hkvVec2(1.0f, 1.0f), 0.05f, 0.0f);
	line2->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.25f);
	line3->scaleTo(hkvVec2(1.0f, 1.0f), 0.05f, 0.4f);
	line4->scaleTo(hkvVec2(1.0f, 1.0f), 0.05f, 0.65f);
	line5->scaleTo(hkvVec2(1.0f, 1.0f), 0.15f, 0.7f);
}

void LevelsMenu::showLevelsList()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	levelsList = new LevelsList();

	//textures
	VTextureObject* itemBgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_list_item_bg.png");
	VTextureObject* lockTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_list_item_lock.png");

	//dimensions
	float itemBgWidth = 1024 * scale;
	float itemBgHeight = 256 * scale;
	hkvVec2 itemBgSize = hkvVec2(itemBgWidth, itemBgHeight);
	float lockWidth = 128 * scale;
	float lockHeight = 128 * scale;
	hkvVec2 lockSize = hkvVec2(lockWidth, lockHeight);

	levelsList->init(0.5f * screenSize, screenSize.x, 1.0f * screenSize.y, itemBgSize);

	//find 1st level entry offset for the reached level to be centered in the view
	int reachedLevel = PersistentDataManager::sharedInstance().getReachedLevel();
	float firstLevelOffset = 0.5f * (levelsList->getSize().y - itemBgSize.y) - (reachedLevel - 1) * itemBgSize.y;

	vector<Level*> levels = LevelManager::sharedInstance().getLevels();
	for (int levelIndex = 0; levelIndex != levels.size(); levelIndex++)
	{
		Level* level = levels[levelIndex];

		ScreenMaskPanel* panelEntry = new ScreenMaskPanel();
		panelEntry->init(hkvVec2(0, 0));
		levelsList->addPanelEntry(panelEntry, itemBgSize, 0, (levelIndex == 0) ? hkvVec2(0, firstLevelOffset) : hkvVec2(0, 0));
		

		//add bg
		SpriteScreenMask* itemBg = new SpriteScreenMask();
		itemBg->AddRef();
		hkvVec2 itemBgPosition = hkvVec2(0,0);
		itemBg->init(itemBgPosition, itemBgSize, itemBgTexture);

		panelEntry->addScreenMask(itemBg);

		itemBg->SetOrder(LEVELS_LIST_ITEM_ORDER);
		
		if (level->isLocked())
		{
			//add lock
			SpriteScreenMask* lock = new SpriteScreenMask();
			lock->AddRef();
			hkvVec2 lockPosition = hkvVec2(0,0);
			lock->init(lockPosition, lockSize, lockTexture);
			panelEntry->addScreenMask(lock);
			lock->SetOrder(LEVELS_LIST_ITEM_ORDER - 1);
		}
		else
		{
			PrintTextLabel* levelTitleTextLabel = new PrintTextLabel();
			levelTitleTextLabel->AddRef();
			float fontHeight = 64 * scale;
			VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
			string text = string("Level ") + stringFromInt(level->getNumber()) + string(" - ") + level->getTitle();
			levelTitleTextLabel->init(hkvVec2(0,0), text.c_str(), neuropolFont, fontHeight, V_RGBA_WHITE);
			panelEntry->addTextLabel(levelTitleTextLabel);
		}
		
		//scale and opacity
		panelEntry->SetOpacity(0);
		panelEntry->SetScale(hkvVec2(1.0f, 0.0f));
	}

	levelsList->unveil(0.45f);
}

void LevelsMenu::showListArrows()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	//add arrows
	hkvVec2 topArrowsPosition = hkvVec2(0.5f * screenSize.x, 86.0f / 1536.0f * screenSize.y); 
	hkvVec2 bottomArrowsPosition = hkvVec2(0.5f * screenSize.x, 1450.0f / 1536.0f * screenSize.y);
	topArrows = new ItemListArrows();
	bottomArrows = new ItemListArrows();
	topArrows->init(levelsList, ARROWS_DIRECTION_TOP, topArrowsPosition);
	bottomArrows->init(levelsList, ARROWS_DIRECTION_BOTTOM, bottomArrowsPosition);
	topArrows->SetOrder(LEVELS_BUTTONS_ORDER);
	bottomArrows->SetOrder(LEVELS_BUTTONS_ORDER);

	topArrows->m_iID = VGUIManager::GetID(ID_LEVELS_LIST_ARROWS);
	bottomArrows->m_iID = VGUIManager::GetID(ID_LEVELS_LIST_ARROWS);

	//disable arrows if center element of the list is either first or last element
	if (levelsList->getCurrentItemIndex() == 0)
		MenuManager::sharedInstance()->getLevelsMenu()->getTopArrows()->disable();
	else if (levelsList->getCurrentItemIndex() == levelsList->getPanels().size() - 1)
		MenuManager::sharedInstance()->getLevelsMenu()->getBottomArrows()->disable();
}

void LevelsMenu::showButtons()
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float scale = MenuManager::sharedInstance()->getScale();

	//textures
	VTextureObject* startBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_start_btn.png");
	VTextureObject* backBtnTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_back_btn.png");

	//dimensions
	float btnWidth = 256 * scale;
	float btnHeight = 256 * scale;
	hkvVec2 btnSize = hkvVec2((float) btnWidth, (float) btnHeight);
	
	//buttons
	startBtn = new StartButton();
	hkvVec2 startBtnPosition = hkvVec2(screenSize.x - 200.0f * scale, 200.0f * scale);
	startBtn->init(startBtnPosition, btnSize, startBtnTexture);
	startBtn->SetScale(hkvVec2(0.0f, 0.0f));
	startBtn->SetOpacity(0);
	startBtn->SetOrder(LEVELS_BUTTONS_ORDER);

	backBtn = new BackButton();
	hkvVec2 backBtnPosition = hkvVec2(200.0f * scale, 200.0f * scale);
	backBtn->init(backBtnPosition, btnSize, backBtnTexture);
	backBtn->SetScale(hkvVec2(0.0f, 0.0f));
	startBtn->SetOpacity(0);
	backBtn->SetOrder(LEVELS_BUTTONS_ORDER);
	backBtn->m_iID = VGUIManager::GetID(ID_BACK_FROM_LEVELS);

	//animations
	startBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.1f, 0.0f); 
	backBtn->scaleTo(hkvVec2(1.0f, 1.0f), 0.1f, 0.0f); 
	startBtn->fadeTo(1.0f, 0.1f, 0.0f);
	backBtn->fadeTo(1.0f, 0.1f, 0.0f);
}

void LevelsMenu::startLevel()
{
	int levelNumber = this->levelsList->getCurrentItemIndex() + 1;
	bool levelLoaded = SceneManager::sharedInstance().loadLevel(levelNumber);
}

void LevelsMenu::processPointerEvent(hkvVec2 touchLocation, int eventType)
{
	if (startBtn)
	{
		if (startBtn->trigger(touchLocation, eventType))
			return;
	}

	if (backBtn)
	{
		if (backBtn->trigger(touchLocation, eventType))
			return;
	}

	if (topArrows)
	{
		if (topArrows->trigger(touchLocation, eventType))
			return;
	}

	if (bottomArrows)
	{
		if (bottomArrows->trigger(touchLocation, eventType))
			return;
	}

	if (levelsList)
		levelsList->trigger(touchLocation, eventType);
}

vector<PrintTextLabel*> LevelsMenu::retrieveAllTextLabels()
{
	vector<PrintTextLabel*> allTextLabels;
	vector<ScreenMaskPanel*> levelsItems = levelsList->getPanels();
	for (int levelsItemIndex = 0; levelsItemIndex != levelsItems.size(); levelsItemIndex++)
	{
		vector<PrintTextLabel*> itemTextLabels = levelsItems[levelsItemIndex]->retrieveAllTextLabels();
		allTextLabels.insert(allTextLabels.end(), itemTextLabels.begin(), itemTextLabels.end());
	}

	return allTextLabels;
}

void LevelsMenu::removeSelf()
{
	this->fountainPanel = NULL;
	this->levelsList = NULL;
	this->topArrows = NULL;
	this->bottomArrows = NULL;
	this->backBtn = NULL;
	this->startBtn = NULL;

	//remove lines
	for (int lineIndex = 0; lineIndex != lines.size(); lineIndex++)
	{
		lines[lineIndex]->Release();
	}
}

////////////////////////////////////LevelsList////////////////////////////////////
void LevelsList::init(hkvVec2 position, float width, float height, hkvVec2 itemSize)
{
	ItemList::init(position, width, height, itemSize);
}

void LevelsList::unveil(float delay)
{
	this->unveilDelayElapsedTime = 0;
	this->unveilDelayDuration = delay;
	this->unveiling = true;
	this->unveilDuration = 13 * UNVEIL_TIME_INTERVAL; //unveil 13 elements before and after the centered element
}

void LevelsList::adjustOpacity(ScreenMaskPanel* panel)
{
	//if (MenuManager::sharedInstance()->isTransitioning())
	//	return;

	float distanceToCenter = abs(panel->GetPosition().y - this->GetPosition().y);
	float opacity = 1 - distanceToCenter / (0.5f * this->getSize().y);

	if (opacity < 0)
		opacity = 0;
	else if (opacity > 1)
		opacity = 1;

	panel->SetOpacity(opacity);
}

void LevelsList::adjustScale(ScreenMaskPanel* panel)
{
	float distanceToCenter = abs(panel->GetPosition().y - this->GetPosition().y);
	float scale = 1 - 0.5f * distanceToCenter / (0.5f * this->getSize().y);
	panel->SetScale(hkvVec2(scale, scale));
}

void LevelsList::unveilItem(ScreenMaskPanel* item)
{
	//opacity
	float distanceToCenter = abs(item->GetPosition().y - this->GetPosition().y);
	float targetOpacity = 1 - distanceToCenter / (0.5f * this->getSize().y);

	if (targetOpacity < 0)
		targetOpacity = 0;
	else if (targetOpacity > 1)
		targetOpacity = 1;

	//scale
	float targetUniformScale = 1 - 0.5f * distanceToCenter / (0.5f * this->getSize().y);
	hkvVec2 targetScale = hkvVec2(targetUniformScale, targetUniformScale);

	//animate item
	item->fadeTo(targetOpacity, 0.2f, 0.0f);
	item->scaleTo(targetScale, 0.2f, 0.0f);
}

void LevelsList::scroll(float dx)
{
	if (panels.size() > 0)
	{
		ScreenMaskPanel* firstEntry = panels[0];
		ScreenMaskPanel* lastEntry = panels[panels.size() - 1];

		float firstEntryNextPositionY = firstEntry->GetPosition().y + dx;
		if (firstEntryNextPositionY >= this->GetPosition().y)
		{
			firstEntryNextPositionY = this->GetPosition().y;
			dx = this->GetPosition().y - firstEntry->GetPosition().y;
		}
		float lastEntryNextPositionY = lastEntry->GetPosition().y + dx;
		if (lastEntryNextPositionY <= this->GetPosition().y)
		{
			lastEntryNextPositionY = this->GetPosition().y;
			dx = this->GetPosition().y - lastEntry->GetPosition().y;
		}
		if (firstEntryNextPositionY <= this->GetPosition().y
			&&
			lastEntryNextPositionY >= this->GetPosition().y)
			ItemList::scroll(dx);
	}
}

void LevelsList::onItemSelected()
{
	MenuManager::sharedInstance()->getLevelsMenu()->getTopArrows()->enable();
	MenuManager::sharedInstance()->getLevelsMenu()->getBottomArrows()->enable();
	if (this->currentItemIndex == 0)
		MenuManager::sharedInstance()->getLevelsMenu()->getTopArrows()->disable();
	else if (this->currentItemIndex == this->panels.size() - 1)
		MenuManager::sharedInstance()->getLevelsMenu()->getBottomArrows()->disable();
}

void LevelsList::update(float dt)
{
	ItemList::update(dt);
	if (this->unveilDelayElapsedTime < this->unveilDelayDuration)
	{
		unveilDelayElapsedTime += dt;
		return;
	}
	if (this->unveiling)
	{
		int reachedLevel = PersistentDataManager::sharedInstance().getReachedLevel();
		float newUnveilElapsedTime = unveilElapsedTime + dt;
		if (newUnveilElapsedTime > (unveiledItemsCount * UNVEIL_TIME_INTERVAL))
		{
			if (unveiledItemsCount == 0)
			{
				//this->adjustOpacity(this->panels[reachedLevel - 1]);
				AudioManager::sharedInstance().playLevelsListOpeningSound();
				this->unveilItem(this->panels[reachedLevel - 1]);
			}
			else
			{
				ScreenMaskPanel* topItem = NULL;
				ScreenMaskPanel* bottomItem = NULL;

				int itemsCount = (int) panels.size();

				if (reachedLevel > unveiledItemsCount)
					topItem = this->panels[reachedLevel - 1 - unveiledItemsCount];
				if (reachedLevel < itemsCount + 1 - unveiledItemsCount)
					bottomItem = this->panels[reachedLevel - 1 + unveiledItemsCount];

				if (topItem)
					/*this->adjustOpacity(topItem);*/
					this->unveilItem(topItem);
				if (bottomItem)
					//this->adjustOpacity(bottomItem);
					this->unveilItem(bottomItem);

			}
			
			unveiledItemsCount++;

			if (newUnveilElapsedTime > unveilDuration)
				this->unveiling = false;
		}
		unveilElapsedTime = newUnveilElapsedTime;
	}
}

////////////////////////////////////StartButton////////////////////////////////////
void StartButton::onClick()
{
	MenuManager::sharedInstance()->startLevel();
}

////////////////////////////////////BackButton////////////////////////////////////
void BackButton::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_BACK_FROM_LEVELS))
	{
		MenuManager::sharedInstance()->menuTransition(false);
	}
	else if (this->m_iID == VGUIManager::GetID(ID_BACK_FROM_ACHIEVEMENTS_1) || this->m_iID == VGUIManager::GetID(ID_BACK_FROM_ACHIEVEMENTS_2))
	{
		MenuManager::sharedInstance()->getIntroMenu()->performCloseAchievementsAction();
	}
}

////////////////////////////////////SplashScreen////////////////////////////////////
void SplashScreen::init(float duration, SplashScreen* nextSplashScreen)
{
	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();

	ScreenMaskPanel::init(0.5f * screenSize); //splashscreen is centered
	this->SetOpacity(0);
	this->splashDuration = duration + 2 * SPLASH_FADE_DURATION;
	this->nextSplashScreen = nextSplashScreen;
}

void SplashScreen::launch()
{
	this->running = true;
}

void SplashScreen::update(float dt)
{
	if (VVideo::IsSplashScreenActive()) //anarchy splash screen
		return;

	if (this->running)
	{
		float oldTime = splashElapsedTime;
		splashElapsedTime += dt;
		if (splashElapsedTime < SPLASH_FADE_DURATION)
		{
			float deltaOpacity = 1.0f / SPLASH_FADE_DURATION * dt;
			this->IncOpacity(deltaOpacity);
		}
		else if (oldTime < SPLASH_FADE_DURATION && splashElapsedTime >= SPLASH_FADE_DURATION)
		{
			this->SetOpacity(1.0f);
		}
		else if (splashElapsedTime > (splashDuration - SPLASH_FADE_DURATION) &&  oldTime <= (splashDuration - SPLASH_FADE_DURATION))
		{
			dt = splashElapsedTime - (splashDuration - SPLASH_FADE_DURATION);
			float deltaOpacity = -1.0f / SPLASH_FADE_DURATION * dt;
			this->IncOpacity(deltaOpacity);
		}
		else if (splashElapsedTime > (splashDuration - SPLASH_FADE_DURATION) && splashElapsedTime < splashDuration)
		{
			float deltaOpacity = -1.0f / SPLASH_FADE_DURATION * dt;
			this->IncOpacity(deltaOpacity);
		}
		else if (splashElapsedTime >= splashDuration)
		{
			if (nextSplashScreen)
				nextSplashScreen->launch();
			else
				MenuManager::sharedInstance()->showIntroMenu();
			MenuManager::sharedInstance()->removeSplashScreen(this);
		}
	}
}

////////////////////////////////////AchievementList////////////////////////////////////
void AchievementList::onPageSwipe(int currentPage)
{
	//sound
	AudioManager::sharedInstance().playWindowOpeningSound();

	ItemListArrows* leftArrows = (ItemListArrows*) this->parentPanel->findElementById(ID_ACHIEVEMENTS_ARROWS_LEFT);
	ItemListArrows* rightArrows = (ItemListArrows*) this->parentPanel->findElementById(ID_ACHIEVEMENTS_ARROWS_RIGHT);
	leftArrows->enable();
	rightArrows->enable();

	if (currentPage == 0)
		leftArrows->disable();
	else if (currentPage = this->lastPage)	
		rightArrows->disable();

	PrintTextLabel* pageCountTextLabel = (PrintTextLabel*) this->parentPanel->findElementById(ID_ACHIEVEMENTS_PAGE_COUNT);
	string pageCountText = stringFromInt(currentPage + 1) + string("/") + stringFromInt(this->lastPage + 1);
	pageCountTextLabel->setText(pageCountText.c_str());
}

////////////////////////////////////AchievementPanel////////////////////////////////////
void AchievementPanel::init(string id, hkvVec2 position, string title, string description, VTextureObject* iconTexture)
{
	ScreenMaskPanel::init(position);
	float scale = MenuManager::sharedInstance()->isContextActive() ? MenuManager::sharedInstance()->getScale() : GameDialogManager::sharedInstance()->getScale();
	hkvVec2 screenSize = MenuManager::sharedInstance()->isContextActive() ? MenuManager::sharedInstance()->getScreenSize() : GameDialogManager::sharedInstance()->getScreenSize();

	hkvVec2 achievementPanelSize = hkvVec2(0.94f * screenSize.x, 200.0f * scale);
	float gap = 30.0f * scale; //gap between icon and content holder

	//icon
	SpriteScreenMask* iconMask = new SpriteScreenMask();
	iconMask->AddRef();
	float iconWidth = 200.0f * scale;
	float iconHeight = 200.0f * scale;
	hkvVec2 iconSize = hkvVec2(iconWidth, iconHeight); 
	hkvVec2 iconPosition = hkvVec2(-0.5f * achievementPanelSize.x + 0.5f * iconWidth, 0);
	iconMask->init(iconPosition, iconSize, iconTexture);
	this->addScreenMask(iconMask);

	//panel holding bg, contour, title, description, gauge
	ScreenMaskPanel* contentHolder = new ScreenMaskPanel();
	hkvVec2 contentHolderSize = hkvVec2(achievementPanelSize.x - iconSize.x - gap, 200.0f * scale);
	hkvVec2 contentHolderPosition = hkvVec2(0.5f * (achievementPanelSize.x - contentHolderSize.x), 0);
	contentHolder->AddRef();
	contentHolder->init(contentHolderPosition);
	this->addPanel(contentHolder);

	//bg with angles
	VTextureObject* blankTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");

	float angleHorizontalContourWidth = 20.0f * scale;
	float angleHorizontalContourHeight = 4.0f * scale;
	hkvVec2 angleHorizontalContourSize = hkvVec2(angleHorizontalContourWidth, angleHorizontalContourHeight);
	float angleVerticalContourWidth = 4.0f * scale;
	float angleVerticalContourHeight = 20.0f * scale;
	hkvVec2 angleVerticalContourSize = hkvVec2(angleVerticalContourWidth, angleVerticalContourHeight);

	ScreenMaskPanel* topLeftAngle = new ScreenMaskPanel();
	topLeftAngle->AddRef();
	hkvVec2 topLeftAnglePosition = hkvVec2(-0.5f * (contentHolderSize.x - angleHorizontalContourWidth), -0.5f * (contentHolderSize.y - angleVerticalContourHeight));
	topLeftAngle->init(topLeftAnglePosition);
	SpriteScreenMask* topLeftAngleHorizontalContour = new SpriteScreenMask();
	topLeftAngleHorizontalContour->AddRef();
	topLeftAngleHorizontalContour->init(hkvVec2(0, -0.5f * (angleVerticalContourHeight - angleHorizontalContourHeight)), angleHorizontalContourSize, blankTexture);
	topLeftAngle->addScreenMask(topLeftAngleHorizontalContour);
	SpriteScreenMask* topLeftAngleVerticalContour = new SpriteScreenMask();
	topLeftAngleVerticalContour->AddRef();
	topLeftAngleVerticalContour->init(hkvVec2(-0.5f * (angleHorizontalContourWidth - angleVerticalContourWidth), 0), angleVerticalContourSize, blankTexture);
	topLeftAngle->addScreenMask(topLeftAngleVerticalContour);

	ScreenMaskPanel* topRightAngle = new ScreenMaskPanel();
	topRightAngle->AddRef();
	hkvVec2 topRightAnglePosition = hkvVec2(0.5f * (contentHolderSize.x - angleHorizontalContourWidth), -0.5f * (contentHolderSize.y - angleVerticalContourHeight));
	topRightAngle->init(topRightAnglePosition);
	SpriteScreenMask* topRightAngleHorizontalContour = new SpriteScreenMask();
	topRightAngleHorizontalContour->AddRef();
	topRightAngleHorizontalContour->init(hkvVec2(0, -0.5f * (angleVerticalContourHeight - angleHorizontalContourHeight)), angleHorizontalContourSize, blankTexture);
	topRightAngle->addScreenMask(topRightAngleHorizontalContour);
	SpriteScreenMask* topRightAngleVerticalContour = new SpriteScreenMask();
	topRightAngleVerticalContour->AddRef();
	topRightAngleVerticalContour->init(hkvVec2(0.5f * (angleHorizontalContourWidth - angleVerticalContourWidth), 0), angleVerticalContourSize, blankTexture);
	topRightAngle->addScreenMask(topRightAngleVerticalContour);

	ScreenMaskPanel* bottomRightAngle = new ScreenMaskPanel();
	bottomRightAngle->AddRef();
	hkvVec2 bottomRightAnglePosition = hkvVec2(0.5f * (contentHolderSize.x - angleHorizontalContourWidth), 0.5f * (contentHolderSize.y - angleVerticalContourHeight));
	bottomRightAngle->init(bottomRightAnglePosition);
	SpriteScreenMask* bottomRightAngleHorizontalContour = new SpriteScreenMask();
	bottomRightAngleHorizontalContour->AddRef();
	bottomRightAngleHorizontalContour->init(hkvVec2(0, 0.5f * (angleVerticalContourHeight - angleHorizontalContourHeight)), angleHorizontalContourSize, blankTexture);
	bottomRightAngle->addScreenMask(bottomRightAngleHorizontalContour);
	SpriteScreenMask* bottomRightAngleVerticalContour = new SpriteScreenMask();
	bottomRightAngleVerticalContour->AddRef();
	bottomRightAngleVerticalContour->init(hkvVec2(0.5f * (angleHorizontalContourWidth - angleVerticalContourWidth), 0), angleVerticalContourSize, blankTexture);
	bottomRightAngle->addScreenMask(bottomRightAngleVerticalContour);

	ScreenMaskPanel* bottomLeftAngle = new ScreenMaskPanel();
	bottomLeftAngle->AddRef();
	hkvVec2 bottomLeftAnglePosition = hkvVec2(-0.5f * (contentHolderSize.x - angleHorizontalContourWidth), 0.5f * (contentHolderSize.y - angleVerticalContourHeight));
	bottomLeftAngle->init(bottomLeftAnglePosition);
	SpriteScreenMask* bottomLeftAngleHorizontalContour = new SpriteScreenMask();
	bottomLeftAngleHorizontalContour->AddRef();
	bottomLeftAngleHorizontalContour->init(hkvVec2(0, 0.5f * (angleVerticalContourHeight - angleHorizontalContourHeight)), angleHorizontalContourSize, blankTexture);
	bottomLeftAngle->addScreenMask(bottomLeftAngleHorizontalContour);
	SpriteScreenMask* bottomLeftAngleVerticalContour = new SpriteScreenMask();
	bottomLeftAngleVerticalContour->AddRef();
	bottomLeftAngleVerticalContour->init(hkvVec2(-0.5f * (angleHorizontalContourWidth - angleVerticalContourWidth), 0), angleVerticalContourSize, blankTexture);
	bottomLeftAngle->addScreenMask(bottomLeftAngleVerticalContour);

	contentHolder->addPanel(topLeftAngle);
	contentHolder->addPanel(topRightAngle);
	contentHolder->addPanel(bottomRightAngle);
	contentHolder->addPanel(bottomLeftAngle);

	float innerBorder = 40.0f * scale;
	//title
	PrintTextLabel* titleLabel = new PrintTextLabel();
	titleLabel->AddRef();
	hkvVec2 titleLabelLocalPos = hkvVec2(-0.5f * contentHolderSize.x + innerBorder, -0.5f * contentHolderSize.y + 0.5f * innerBorder);
	float titleLabelFontHeight = 54 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, titleLabelFontHeight);
	titleLabel->init(hkvVec2(0,0), title, neuropolFont, titleLabelFontHeight, V_RGBA_WHITE);
	titleLabel->SetPosition(titleLabelLocalPos + titleLabel->getAnchorPoint().compMul(titleLabel->getSize()));
	contentHolder->addTextLabel(titleLabel);

	//description
	PrintTextLabel* descriptionLabel = new PrintTextLabel();
	descriptionLabel->AddRef();
	hkvVec2 descriptionLabelLocalPos = hkvVec2(-0.5f * contentHolderSize.x + innerBorder, titleLabelLocalPos.y + 50.0f * scale);
	float descriptionLabelFontHeight = 40 * scale;
	neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, titleLabelFontHeight);
	string splittedDescription;
	StringsXmlHelper::sharedInstance().splitStringAndCreateLines(splittedDescription, description, 25);
	descriptionLabel->init(hkvVec2(0,0), splittedDescription, neuropolFont, descriptionLabelFontHeight, V_RGBA_WHITE);
	descriptionLabel->SetPosition(descriptionLabelLocalPos + descriptionLabel->getAnchorPoint().compMul(descriptionLabel->getSize()));
	contentHolder->addTextLabel(descriptionLabel);

	//gauge
	AchievementManager &achievementManager = AchievementManager::sharedInstance();
	int achievementMaxValue = achievementManager.getAchievementMaxValue(id);
	if (achievementMaxValue)
	{
		int achievementCurrentValue = achievementManager.getAchievementCurrentValue(id);
		float completionPercentage = achievementCurrentValue / (float) achievementMaxValue;

		hkvVec2 gaugeBgSize = hkvVec2(contentHolderSize.x - 4 * innerBorder, 30.0f * scale);
		hkvVec2 gaugeFillSize = hkvVec2(completionPercentage * gaugeBgSize.x, 30.0f * scale);
		
		SpriteScreenMask* gaugeBg = new SpriteScreenMask();
		gaugeBg->AddRef();
		hkvVec2 gaugeBgPosition = hkvVec2(-0.5f * (contentHolderSize.x - gaugeBgSize.x) + innerBorder, 0.5f * contentHolderSize.y - 0.5f * innerBorder - 0.5f * gaugeBgSize.y);
		gaugeBg->init(gaugeBgPosition, gaugeBgSize, blankTexture);
		gaugeBg->SetColor(VColorRef(36,36,36));
		contentHolder->addScreenMask(gaugeBg);

		SpriteScreenMask* gaugeFill = new SpriteScreenMask();
		gaugeFill->AddRef();
		hkvVec2 gaugeFillPosition = hkvVec2(-0.5f * (contentHolderSize.x - gaugeFillSize.x) + innerBorder, gaugeBgPosition.y);
		gaugeFill->init(gaugeFillPosition, gaugeFillSize, blankTexture);
		gaugeFill->SetColor(V_RGBA_WHITE);
		contentHolder->addScreenMask(gaugeFill);

		PrintTextLabel* gaugeValueTextLabel = new PrintTextLabel();
		gaugeValueTextLabel->AddRef();
		
		float gaugeValueFontHeight = 40.0f * scale;
		VisFont_cl* gaugeValueFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, gaugeValueFontHeight);
		gaugeValueTextLabel->init(hkvVec2(0,0), stringFromInt(achievementCurrentValue), gaugeValueFont, gaugeValueFontHeight, V_RGBA_WHITE);
		hkvVec2 gaugeValueTextLabelPosition = hkvVec2(gaugeFillPosition.x + 0.5f * gaugeFillSize.x, gaugeFillPosition.y - 0.5f * (gaugeFillSize.y + gaugeValueTextLabel->getSize().y + 10.0f * scale));
		gaugeValueTextLabel->SetPosition(gaugeValueTextLabelPosition);
		contentHolder->addTextLabel(gaugeValueTextLabel);
	}
}


////////////////////////////////////ItemListArrows////////////////////////////////////
V_IMPLEMENT_DYNCREATE(ItemListArrows, ClickableScreenMaskPanel, Vision::GetEngineModule());

void ItemListArrows::init(ItemList* list, int direction, hkvVec2 position, bool drawLine, hkvVec2 scale, float opacity)
{
	ClickableScreenMaskPanel::init(position, scale, opacity);

	hkvVec2 screenSize = MenuManager::sharedInstance()->getScreenSize();
	float globalScale = MenuManager::sharedInstance()->getScale();

	this->list = list;
	this->direction = direction;

	//line
	//if (drawLine)
	//{
	//	VTextureObject* whiteTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
	//	float lineWidth = 200.0f / 2048.0f * screenSize.x;
	//	float lineHeight = 4.0f * globalScale;
	//	hkvVec2 lineSize = hkvVec2(lineWidth, lineHeight);
	//	SpriteScreenMask* lineMask = new SpriteScreenMask();
	//	lineMask->AddRef();
	//	hkvVec2 linePosition = hkvVec2(0, 0.5f * (touchArea.y - lineHeight));
	//	hkvVec4 lineTextureRange = hkvVec4(0, 0, lineSize.x, lineSize.y);
	//	lineMask->init(linePosition, lineSize, lineTextureRange, whiteTexture, true);
	//	this->addScreenMask(lineMask);
	//}

	//arrows
	VTextureObject* arrowTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\itemlist_arrow.png");
	VTextureObject* arrow2Texture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\itemlist_arrow2.png");
	float arrowWidth = 64.0f * globalScale;
	float arrowHeight = 64.0f * globalScale;
	hkvVec2 arrowSize = hkvVec2(arrowWidth, arrowHeight);
	SpriteScreenMask* arrow1 = new SpriteScreenMask();
	SpriteScreenMask* arrow2 = new SpriteScreenMask();
	arrow1->AddRef();
	arrow2->AddRef();
	//hkvVec2 arrow1Position = hkvVec2(0, linePosition.y - 0.40f * arrowHeight - lineHeight);
	//hkvVec2 arrow2Position = hkvVec2(0, arrow1Position.y - 0.55f * arrowHeight);
	hkvVec2 arrow1Position = hkvVec2(0, -0.23f * arrowHeight);
	hkvVec2 arrow2Position = hkvVec2(0, 0.23f * arrowHeight);
	arrow1->init(arrow1Position, arrowSize, arrow2Texture);
	arrow2->init(arrow2Position, arrowSize, arrow2Texture);
	this->addScreenMask(arrow1);
	this->addScreenMask(arrow2);

	if (direction == ARROWS_DIRECTION_LEFT)	
		this->SetAngle(90);
	else if (direction == ARROWS_DIRECTION_TOP)	
		this->SetAngle(0);
	else if (direction == ARROWS_DIRECTION_RIGHT)	
		this->SetAngle(-90);
	else if (direction == ARROWS_DIRECTION_BOTTOM)	
		this->SetAngle(180);


	this->touchArea = hkvVec2(4 * arrowHeight, 4 * arrowWidth);
}

void ItemListArrows::onClick()
{
	if (this->m_iID == VGUIManager::GetID(ID_LEVELS_LIST_ARROWS))
	{
		if (direction == ARROWS_DIRECTION_TOP)
			list->scrollUpOneElement();
		else if (direction == ARROWS_DIRECTION_BOTTOM)
			list->scrollDownOneElement();
	}
	else if (this->m_iID == VGUIManager::GetID(ID_ACHIEVEMENTS_ARROWS_LEFT) || this->m_iID == VGUIManager::GetID(ID_ACHIEVEMENTS_ARROWS_RIGHT))
	{
		if (direction == ARROWS_DIRECTION_RIGHT)
			list->pageSwipe(true);
		else if (direction == ARROWS_DIRECTION_LEFT)
			list->pageSwipe(false);
	}
}


/////////////////////////////////OrbFountain/////////////////////////////////
OrbFountain::OrbFountain() : radius(0), generatingOrbs(false), currentNumberOfOrbs(0), totalNumberOfOrbs(0), generationElapsedTime(0) 
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

OrbFountain::~OrbFountain()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;

	for (int orbIndex = 0; orbIndex != orbs.size(); orbIndex++)
	{
		orbs[orbIndex]->Release();
	}
}

void OrbFountain::init(hkvVec2 position, float radius, int numberOfOrbs)
{
	Anchored2DObject::init(position);
	this->radius = radius;
	this->totalNumberOfOrbs = numberOfOrbs;
}

void OrbFountain::generateFountainOrb()
{
	float scale = MenuManager::sharedInstance()->getScale();

	VTextureObject* orbTexture = Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
	float orbWidth = 32 * scale;
	float orbHeight = 32 * scale;
	hkvVec2 orbSize = hkvVec2(orbWidth, orbHeight);

	Orb2D* orb = new Orb2D();
	orb->AddRef();
	orb->setParentFountain(this);
	orb->init(hkvVec2(0,0), orbSize, orbTexture);
	orb->SetOrder(FOUNTAIN_ORBS_ORDER);

	this->currentNumberOfOrbs++;
	this->orbs.push_back(orb);
}

void OrbFountain::startGeneratingOrbs()
{
	this->generatingOrbs = true;
}

void OrbFountain::update(float dt)
{
	if (!generatingOrbs)
		return;
	if (currentNumberOfOrbs < totalNumberOfOrbs)
	{
		generationElapsedTime += dt;
		if (generationElapsedTime > (currentNumberOfOrbs * FOUNTAIN_ORB_GENERATION_INTERVAL))
		{
			generateFountainOrb();
		}
	}
}

/////////////////////////////////Orb2D/////////////////////////////////
Orb2D::Orb2D() : radius(0),
	theta(0),
	periodLength(0),
	periodElapsedTime(0),
	thetaSignOfVariation(false),
	radiusVariation(0),
	thetaVariation(0),
	linearSpeed(0),
	parentFountain(NULL)
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

Orb2D::~Orb2D()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void Orb2D::init(hkvVec2 position, hkvVec2 size, VTextureObject* textureObj)
{
	SpriteScreenMask::init(position, size, textureObj);
	
	VSimpleRenderState_t renderState;
	renderState.SetTransparency(VIS_TRANSP_ALPHA);
	this->SetRenderState(renderState);

	this->startNewPeriod(true);
}

void Orb2D::startNewPeriod(bool firstPeriod)
{
	float startRadius, targetRadius, startTheta, targetTheta;
	setUpRandomValues(startRadius, targetRadius, startTheta, targetTheta, firstPeriod);

	//radius variation
	radiusVariation = targetRadius - startRadius;

	//theta variation
	if (thetaSignOfVariation)
		thetaVariation = targetTheta > startTheta ? targetTheta - startTheta : 360 - startTheta + targetTheta;
	else
		thetaVariation = targetTheta < startTheta ? targetTheta - startTheta : -(360 - targetTheta + startTheta);

	//the distance the orb has to go over
	float distance = GeometryUtils::arcDistance(0.5f * (startRadius + targetRadius),
												startTheta,
												targetTheta,
												thetaSignOfVariation);

	//set the speed of an orb
	int speedVariancePercentage = 50; //percentage of variation of the speed around the stantdard speed ORB_LINEAR_SPEED
	float speedVariance = (float) (rand() % (2 * speedVariancePercentage + 1));
	speedVariance -= (0.5f * speedVariance); //shift it to the left
	linearSpeed = (1 + speedVariance / 100.0f) * FOUNTAIN_ORB_LINEAR_SPEED;

	//Adjust the length of the period according to the speed of the orb
	periodLength = distance / linearSpeed; //linear case
	
	periodElapsedTime = 0;
}

void Orb2D::setUpRandomValues(float &startRadius, float &targetRadius, float &startTheta, float &targetTheta, bool firstPeriod)
{
	//radius
	float midRadius = 0.5f * parentFountain->getRadius();
	if (firstPeriod)
	{
		startRadius = 0; //orbs are emitted from the center of the fountain
		radius = startRadius;
		targetRadius = midRadius;
	}
	else
	{
		startRadius = radius;
	}

	if (!firstPeriod)
	{		
		int deltaRadius = rand() % ((int) (0.24f * parentFountain->getRadius()));
		bool radiusSignOfVariation;

		float centerDirectionProbability = ((abs(startRadius - midRadius) / parentFountain->getRadius()) + 0.5f) * 50; //probability for radius to increase or decrease towards the center of the nodeSphere
		int randPercentage = rand() % 101; //percentage between 0 and 100
		if (startRadius >= 0 && startRadius < midRadius)
		{
			if (centerDirectionProbability >= randPercentage)
				radiusSignOfVariation = true;
			else
				radiusSignOfVariation = false;
		}
		else
		{
			if (centerDirectionProbability >= randPercentage)
				radiusSignOfVariation = false;
			else 
				radiusSignOfVariation = true;
		}

		if (radiusSignOfVariation)
		{
			if (startRadius + deltaRadius > parentFountain->getRadius())
				targetRadius = startRadius - deltaRadius;
			else
				targetRadius = startRadius + deltaRadius;
		}
		else
		{
			if (startRadius - deltaRadius < 0)
				targetRadius = startRadius + deltaRadius;
			else
				targetRadius = startRadius - deltaRadius;
		}
	}

	//theta	
	if (firstPeriod)
	{
		int signOfVariation = rand() % 2;
		thetaSignOfVariation = (signOfVariation == 1) ? true : false;
		startTheta = (float) (rand() % 360);
		theta = startTheta;
	}
	else
	{
		GeometryUtils::normalizeAngle(theta);
		startTheta = theta;
	}

	if (thetaSignOfVariation)
		targetTheta = startTheta + rand() % 80 + 20;
	else
		targetTheta = startTheta - (rand() % 80 + 20);
	GeometryUtils::normalizeAngle(targetTheta);
}

float Orb2D::radiusSinusoidalVariation(float dt)
{
	return (float) -(radiusVariation / 2 * (cos(periodElapsedTime * M_PI / periodLength) - cos((periodElapsedTime - dt) * M_PI / periodLength)));
}

float Orb2D::thetaLinearVariation(float dt)
{
	return thetaVariation / periodLength * dt;
}


hkvVec2 Orb2D::GetAbsPosition()
{
	return parentFountain->GetAbsPosition() + currentPosition;
}

void Orb2D::update(float dt)
{
	bool lastDt = false;
	if ((periodElapsedTime + dt) > periodLength) //recalculate dt at pro rata
	{
		lastDt = true;
		dt = periodLength - periodElapsedTime;
		periodElapsedTime = periodLength;
	}
	else
		periodElapsedTime += dt;

	float dRadius = radiusSinusoidalVariation(dt);
	float dTheta = thetaLinearVariation(dt);		
	
	radius += dRadius;
	theta += dTheta;

	GeometryUtils::normalizeAngle(theta);

	hkvVec2 newPosition = GeometryUtils::polarToCartesian(radius, GeometryUtils::degreesToRadians(theta));
	this->SetPosition(newPosition);

	if (lastDt)
	{
		startNewPeriod(false);
	}
}

/////////////////////////////////////PlayTextLabel/////////////////////////////////////
void PlayTextLabel::renderItself()
{
	PrintTextLabel::renderItself();
	//IVRender2DInterface* pRI = Vision::RenderLoopHelper.BeginOverlayRendering();

	//float fontScaling = fontHeight / font->GetFontHeight();
	//VSimpleRenderState_t renderState = VisFont_cl::DEFAULT_STATE;
	//renderState.SetFlag(RENDERSTATEFLAG_FILTERING);

	//
	////Vision::Shaders.LoadShaderLibrary("Shaders\\PostProcess.ShaderLib");
	////VCompiledTechnique* pTechnique = Vision::Shaders.CreateTechnique("Glow", "GlowParams=0.1,3,5,5");
	////VCompiledShaderPass** shaderPasses = pTechnique->GetShaderList();
	////VCompiledShaderPass* shaderPass = NULL;
	////if (shaderPasses && *shaderPasses)
	////	shaderPass = *shaderPasses;

	////if (shaderPass)
	////	font->PrintText(pRI, absPos, text.c_str(), iColor, *shaderPass, fontScaling);
	//font->PrintText(pRI, absPos, text.c_str(), iColor, NULL, fontScaling);

	//Vision::RenderLoopHelper.EndOverlayRendering();
}

////////////////////////////////////////ResetDialogPanel////////////////////////////////////////
void ResetDialogPanel::init()
{
	YesNoDialogPanel::init(StringsXmlHelper::sharedInstance().getStringForTag("reset_confirmation"));
}

void ResetDialogPanel::onClickYes()
{
	GameManager::GlobalManager().resetGame();
	MenuManager::sharedInstance()->dismissDialogHolder();
}

void ResetDialogPanel::onClickNo()
{
	MenuManager::sharedInstance()->dismissDialogHolder();
}
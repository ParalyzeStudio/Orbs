#pragma once

class IntroMenu;
class SpriteScreenMask;
class ButtonScreenMask;
class RotatingSpriteScreenMask;
class MidPanelButton;
class EndCredits;
class LevelsMenu;
class InterfaceMenu;
class LevelsList;
class StartButton;
class BackButton;
class SplashScreen;
class ItemListArrows;
class DefeatVictoryMenu;
class DefeatVictoryMenuButton;
class OrbFountain;
class Orb2D;

//class used to handle GUI elements in menu scenes
class MenuManager : public SceneGUIManager
{
public:
	~MenuManager(void);
	static MenuManager* sharedInstance();

	void deInitGUIContext();

	void defineScale();
	vector<PrintTextLabel*> retrieveAllTextLabels();

	bool clickContainedInGUI(hkvVec2 clickCoords);
	bool processPointerEvent(hkvVec2 touchLocation, int eventType);

	void performToggleOptionsAction(MidPanelButton* sender);
	void performToggleCreditsAction(MidPanelButton* sender);
	void performShowAchievementsAcion(MidPanelButton* sender);

	void showSplashScreens();
	void removeSplashScreens();
	void removeSplashScreen(SplashScreen* splashScreen);
	void removeVeil();
	void showBackground();
	void removeBackground();
	void showIntroMenu();
	void removeIntroMenu(); //remove the entire intro menu scene
	void showLevelsMenu();
	void removeLevelsMenu();
	void showResetDialog();
	void dismissDialogHolder();

	void introToLevels();
	void levelsToIntro();

	void menuTransition(bool introToLevels);
	bool isTransitioning(){return this->transitioning;};

	void startLevel();

	IntroMenu* getIntroMenu(){return this->introMenu;};
	LevelsMenu* getLevelsMenu(){return this->levelsMenu;};
	InterfaceMenu* getDialogHolder(){return this->dialogHolder;};

private:
	MenuManager();
	static MenuManager* instance;

	VSmartPtr<SplashScreen> companySplash;
	VSmartPtr<SplashScreen> headphonesSplash;
	VSmartPtr<SpriteScreenMask> background;
	VSmartPtr<IntroMenu> introMenu;
	VSmartPtr<LevelsMenu> levelsMenu;
	VSmartPtr<TransitionVeil> veil;
	VSmartPtr<InterfaceMenu> dialogHolder;
	bool transitioning;
};

//ids of menu buttons
#define	ID_PLAY "ID_PLAY"
#define	ID_OPTIONS "ID_OPTIONS"
#define	ID_ACHIEVEMENTS "ID_ACHIEVEMENTS"
#define	ID_CREDITS "ID_CREDITS"
#define ID_OPTIONS_MUSIC "ID_OPTIONS_MUSIC"
#define ID_OPTIONS_SOUND "ID_OPTIONS_SOUND"
#define ID_OPTIONS_RESET "ID_OPTIONS_RESET"
#define ID_OPTIONS_MUSIC_TEXT "ID_OPTIONS_MUSIC_TEXT"
#define ID_OPTIONS_SOUND_TEXT "ID_OPTIONS_SOUND_TEXT"
#define ID_OPTIONS_RESET_TEXT "ID_OPTIONS_RESET_TEXT"
#define ID_OPTIONS_TOP_BRACKET "ID_OPTIONS_TOP_BRACKET"
#define ID_OPTIONS_BOTTOM_BRACKET "ID_OPTIONS_BOTTOM_BRACKET"
#define ID_OPTIONS_BG "ID_OPTIONS_BG"
#define ID_BACK_FROM_LEVELS "ID_BACK_FROM_LEVELS"
#define ID_BACK_FROM_ACHIEVEMENTS_1 "ID_BACK_FROM_ACHIEVEMENTS_1"
#define ID_BACK_FROM_ACHIEVEMENTS_2 "ID_BACK_FROM_ACHIEVEMENTS_2"
#define ID_LEVELS_LIST_ARROWS "ID_LEVELS_LIST_ARROWS"
#define ID_ACHIEVEMENTS_ARROWS_LEFT "ID_ACHIEVEMENTS_ARROWS_LEFT"
#define ID_ACHIEVEMENTS_ARROWS_RIGHT "ID_ACHIEVEMENTS_ARROWS_RIGHT"
#define ID_ACHIEVEMENTS_PAGE_COUNT "ID_ACHIEVEMENTS_PAGE_COUNT"
#define ID_CREDITS_BG "ID_CREDITS_BG"
#define ID_CREDITS_CONTOUR_TOP "ID_CREDITS_CONTOUR_TOP"
#define ID_CREDITS_CONTOUR_BOTTOM "ID_CREDITS_CONTOUR_BOTTOM"
#define ID_CREDITS_LIST "ID_CREDITS_LIST"

//orders of different menu screenmasks
#define BACKGROUND_ORDER 1000
#define MENU_CIRCLES_ORDER 8
#define BUBBLE_BG_ORDER 9
#define BUBBLE_HEART_ORDER 7
#define SIDE_BUTTONS_ORDER 20
#define PLAY_BUTTON_ORDER 7
#define FOUNTAIN_ORBS_ORDER 6
#define VEIL_ORDER 1
#define OPTIONS_BG_ORDER 10
#define OPTIONS_OVERLAYS_ORDER 9
#define LEVELS_LIST_ITEM_ORDER 10
#define LEVELS_BUTTONS_ORDER 10
#define END_MENU_ORDER 5
#define DEFAULT_ORDER 0

#define MID_PANEL_SCALE 1.3f //scale applied to the 512x512 textures to make them bigger as designed in photoshop
#define MID_PANEL_BUTTONS_SCALE 1.1f
#define OPTIONS_PANEL_SCALE 1.5f //scale applied to the textures to make them bigger as designed in photoshop

class IntroMenu : public VRefCounter
{
public:
	IntroMenu() : VRefCounter(),
		midPanel(NULL),
		titlePanel(NULL),
		optionsPanel(NULL),
		creditsPanel(NULL){};
	~IntroMenu();

	void init();

	void showMidPanel();
	void removeMidPanel(bool animated);
	void showTitle();
	void removeTitle(bool animated);
	void showOptionsPanel();
	void dismissOptionsPanel();
	void removeOptions();
	void showCredits();
	void removeCredits(bool animated);
	void removeCreditsMasksFromScene(); //actually release masks after scale/fadeOut animations have ended
	void buildAchievementsPanel();
	void removeAchievements();

	void processPointerEvent(hkvVec2 touchLocation, int eventType);
	bool panelProcessPointerEvent(ScreenMaskPanel* panel, hkvVec2 touchLocation, int eventType);

	void performToggleOptionsAction(MidPanelButton* sender);
	void performToggleCreditsAction(MidPanelButton* sender);
	void performShowAchievementsAction(MidPanelButton* sender);
	void performCloseAchievementsAction();

	vector<PrintTextLabel*> retrieveAllTextLabels();

	void removeSelf(); //remove the entire intro menu (mid panel and open side panels), leave title and background

private:
	//mid panel	
	VSmartPtr<ScreenMaskPanel> midPanel;

	//title screen masks
	VSmartPtr<ScreenMaskPanel> titlePanel;

	//credits
	VSmartPtr<ScreenMaskPanel> creditsPanel;

	//options
	VSmartPtr<ScreenMaskPanel> optionsPanel;

	//achievements
	VSmartPtr<ScreenMaskPanel> achievementsPanel;
};


class MidPanelButton : public ButtonScreenMask
{
public:
	MidPanelButton() : ButtonScreenMask(), active(false){};

	bool isActive(){return this->active;};
	void setActive(bool active);

	virtual void onClick();

private:
	bool active;
};

class OptionsButton : public ButtonScreenMask
{
public:
	OptionsButton() : ButtonScreenMask(){};

	virtual void onClick();
};

class EndCredits : public ScrollingList
{
public:
	EndCredits() : ScrollingList() {};
	~EndCredits(){};

	void init(hkvVec2 position, float width, float height, bool autoscroll, float autoscrollSpeed);

	virtual void adjustOpacity(PrintTextLabel* label);
	virtual void adjustOpacity(SpriteScreenMask* mask);

	virtual void scroll(float dx);
};



////LEVELS
class LevelsMenu : public VRefCounter
{
public:
	LevelsMenu() : fountainPanel(NULL),
		levelsList(NULL), 
		topArrows(NULL),
		bottomArrows(NULL),
		startBtn(NULL),
		backBtn(NULL){};
	~LevelsMenu() {};

	void init();
	void showFountainAndBrackets();
	void showLevelsList();
	void showListArrows();
	void showButtons();

	void startLevel();
	void processPointerEvent(hkvVec2 touchLocation, int eventType);

	vector<PrintTextLabel*> retrieveAllTextLabels();

	void removeSelf();

	ItemListArrows* getTopArrows(){return this->topArrows;};
	ItemListArrows* getBottomArrows(){return this->bottomArrows;};

private:
	VSmartPtr<ScreenMaskPanel> fountainPanel;
	VSmartPtr<LevelsList> levelsList;
	VSmartPtr<ItemListArrows> topArrows;
	VSmartPtr<ItemListArrows> bottomArrows;
	VSmartPtr<StartButton> startBtn;
	VSmartPtr<BackButton> backBtn;
	vector<SpriteScreenMask*> lines;
};


#define UNVEIL_TIME_INTERVAL 0.1f //time elapsed between the unveiling of 2 consecutive items

//List that scrolls item by item
class LevelsList : public ItemList
{
public:
	LevelsList() : ItemList(),
		unveilDelayElapsedTime(0),
		unveilDelayDuration(0),
		unveiling(false),
		unveilElapsedTime(0),
		unveilDuration(0),
		unveiledItemsCount(0)
	{};

	virtual void init(hkvVec2 position, float width, float height, hkvVec2 itemSize);

	void unveil(float delay);
	void adjustOpacity(ScreenMaskPanel* panel);
	void adjustScale(ScreenMaskPanel* panel);
	void unveilItem(ScreenMaskPanel* item);

	virtual void scroll(float dx);
	virtual void onItemSelected();

	virtual void update(float dt);

private:
	float unveilDelayElapsedTime;
	float unveilDelayDuration;
	bool unveiling;
	float unveilElapsedTime;
	float unveilDuration;
	int unveiledItemsCount;
};

class StartButton : public ButtonScreenMask
{
public:
	virtual void onClick();
};

class BackButton : public ButtonScreenMask
{
public:
	virtual void onClick();
};

#define SPLASH_FADE_DURATION 1.0f

class SplashScreen : public ScreenMaskPanel
{
public:
	SplashScreen() : ScreenMaskPanel(),
	splashDuration(0),
	nextSplashScreen(NULL), 
	running(false),
	splashElapsedTime(0){};
	~SplashScreen() {};

	virtual void init(float duration, SplashScreen* nextSplashScreen);
	virtual void launch();
	virtual void update(float dt);

private:
	float splashDuration;
	SplashScreen* nextSplashScreen;
	bool running;
	float splashElapsedTime;
};

class AchievementList : public ItemList
{
public:
	virtual void onPageSwipe(int currentPage);
};

class AchievementPanel : public ScreenMaskPanel
{
public:
	AchievementPanel() : ScreenMaskPanel(),	size(hkvVec2(0,0)){};

	void init(string id, hkvVec2 position, string title, string description, VTextureObject* iconTexture);

	hkvVec2 getSize(){return this->size;};

private:
	hkvVec2 size;
};

#define ARROWS_DIRECTION_LEFT 1
#define ARROWS_DIRECTION_TOP 2
#define ARROWS_DIRECTION_RIGHT 3
#define ARROWS_DIRECTION_BOTTOM 4

class ItemListArrows : public ClickableScreenMaskPanel
{
public:
	ItemListArrows() : list(NULL), direction(0) {};

	V_DECLARE_DYNCREATE(ItemListArrows);

	void init(ItemList* list, int direction, hkvVec2 position, bool drawLine = true, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float opacity = 1.0f);

	virtual void onClick();

	int getDirection(){return this->direction;};

private:
	ItemList* list;
	int direction;
};


#define FOUNTAIN_ORB_GENERATION_INTERVAL 0.01f //1 orb every 0.05 sec

class OrbFountain : public VRefCounter, public Anchored2DObject
{
public:
	OrbFountain();
	~OrbFountain();

	void init(hkvVec2 position, float radius, int numberOfOrbs);
	void generateFountainOrb();
	void startGeneratingOrbs();

	virtual void update(float dt);

	float getRadius(){return this->radius;};

private:
	float radius;
	vector<Orb2D*> orbs;
	bool generatingOrbs;
	int currentNumberOfOrbs;
	int totalNumberOfOrbs;
	float generationElapsedTime;
};

#define FOUNTAIN_ORB_LINEAR_SPEED 2.0f

class Orb2D : public SpriteScreenMask
{
public:
	Orb2D();
	~Orb2D();

	void init(hkvVec2 position, hkvVec2 size, VTextureObject* textureObj);

	void startNewPeriod(bool firstPeriod);
	void setUpRandomValues(float &startRadius, float &targetRadius, float &startTheta, float &targetTheta, bool firstPeriod);

	float radiusSinusoidalVariation(float dt);
	float thetaLinearVariation(float dt);

	virtual void update(float dt);

	void setParentFountain(OrbFountain* fountain){this->parentFountain = fountain;};
	hkvVec2 GetAbsPosition();

private:
	//polar coordinates
	float radius;
	float theta;

	//period
	float periodLength;
	float periodElapsedTime;

	//variations over a period
	bool thetaSignOfVariation;
	float radiusVariation;
	float thetaVariation;
	float linearSpeed;

	//parent fountain
	OrbFountain* parentFountain;
};

class PlayTextLabel : public PrintTextLabel
{
public:
	virtual void renderItself();
};

class ResetDialogPanel : public YesNoDialogPanel
{
public:
	void init();

	virtual void onClickYes();
	virtual void onClickNo();
};
#pragma once

#define CORE_ITEMS_COUNT 1

enum CORE_ITEM_IDS {
	CORE_ITEM_GENERATION = 0,
	CORE_ITEM_DEFENSE,
	CORE_ITEM_ASSAULT
};

class GameHUD;
class CorePanelButton;
class ActionButton;
class Panel;
class TogglingPanel;
class ObjectivesMenu;
class DefeatVictoryMenu;
class DefeatVictoryMenuButton;
class NodeSphere;
class Bridge;
class CorePanel;
class MiscPanel;
class ActionPanel;
class MoveSubMenuPanel;
class QuitDialogPanel;
class RetryDialogPanel;
class Path;
class MetaOrb;
class Objective;
class ObjectiveLabel;
class EndpointHalo;
class PauseMenu;
class HelpMenu;
class GaugeController;
class CountdownTimer;
class MiscPanelAlert;


#define CORE_PANEL_ORDER 10
#define INFO_PANEL_ORDER 10
#define ACTION_PANEL_ORDER 10
#define MISC_PANEL_ORDER 10
#define SELECTION_HALO_PANEL_ORDER 25
#define SPHERE_BADGE_ORDER 25
#define INTERFACE_MENU_ORDER 2
#define INTERFACE_PANEL_ORDER 15
#define POPUP_ORDER -3

//class used to handle GUI elements in level scenes
class GameDialogManager : public SceneGUIManager
{
public:
	~GameDialogManager(void);
	static GameDialogManager* sharedInstance();

	void initGUIContext();
	void deInitGUIContext();
	void defineScale();
	vector<PrintTextLabel*> retrieveAllTextLabels();

	void showHUD(); //main HUD
	void removeHUD();	
	void showDefeatVictoryMenu(bool victory, float delay);
	void removeDefeatVictoryMenu();

	bool clickContainedInGUI(hkvVec2 touchLocation); //has the click to be intercepted and swallowed by GUI
	bool processPointerEvent(hkvVec2 touchLocation, int eventType);

	virtual void update(float dt);
	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	//void updateActionButtonsStates(int mode);

	GameHUD* getHUD(){return gameHUD;};

private:
	GameDialogManager(void);
	static GameDialogManager* instance;

	VSmartPtr<GameHUD> gameHUD;
	VSmartPtr<DefeatVictoryMenu> endMenu;

	bool onPressProcessedByGUI;
};

//ids of buttons, text labels and eventually screen masks
#define ID_TAKE_CONTROL_PANEL "ID_TAKE_CONTROL_PANEL"
#define ID_TAKE_CONTROL_BTN "ID_TAKE_CONTROL_BTN"
#define ID_GENERATOR_COST "ID_GENERATOR_COST"

#define ID_MOVE_BTN "ID_MOVE_BTN"
#define ID_STOP_BTN "ID_STOP_BTN"
#define ID_SACRIFICE_BTN "ID_SACRIFICE_BTN"
#define ID_RALLY_POINT_BTN "ID_RALLY_POINT_BTN"
#define ID_BUILD_BTN "ID_BUILD_BTN"
#define ID_ALLIED_ORBS_COUNT_LABEL "ID_ALLIED_ORBS_COUNT_LABEL"
#define ID_ENEMY_ORBS_COUNT_LABEL "ID_ENEMY_ORBS_COUNT_LABEL"
#define ID_SPHERE_LIFE_COUNT_LABEL "ID_SPHERE_LIFE_COUNT_LABEL"
#define ID_CORE_PANEL_BG "ID_CORE_PANEL_BG"
#define ID_CORE_PANEL_CONTENT_HOLDER "ID_CORE_PANEL_CONTENT_HOLDER"
#define ID_GENERATION_PANEL "ID_GENERATION_PANEL"
#define ID_DEFENSE_PANEL "ID_DEFENSE_PANEL"
#define ID_ASSAULT_PANEL "ID_ASSAULT_PANEL"
#define ID_CORE_PANEL_GAUGE_CONTROLLER "ID_CORE_PANEL_GAUGE_CONTROLLER"
#define ID_CORE_PANEL_GAUGE_VALIDATE "ID_CORE_PANEL_GAUGE_VALIDATE"
#define ID_CORE_PANEL_GAUGE_BACK "ID_CORE_PANEL_GAUGE_BACK"
#define ID_BTN_GENERATION_META_ORB "ID_BTN_GENERATION_META_ORB"
#define ID_BTN_GENERATION_META_ORB_REFLECTION "ID_BTN_GENERATION_META_ORB_REFLECTION"
#define ID_BTN_DEFENSE_META_ORB "ID_BTN_DEFENSE_META_ORB"
#define ID_BTN_DEFENSE_META_ORB_REFLECTION "ID_BTN_DEFENSE_META_ORB_REFLECTION"
#define ID_BTN_ASSAULT_META_ORB "ID_BTN_ASSAULT_META_ORB"
#define ID_BTN_ASSAULT_META_ORB_REFLECTION "ID_BTN_ASSAULT_META_ORB_REFLECTION"
#define ID_GENERATION_TEXT_LABEL "ID_GENERATION_TEXT_LABEL"
#define ID_DEFENSE_TEXT_LABEL "ID_DEFENSE_TEXT_LABEL"
#define ID_ASSAULT_TEXT_LABEL "ID_ASSAULT_TEXT_LABEL"

#define ID_PAUSE_BTN "ID_PAUSE_BTN"
#define ID_HELP_BTN "ID_HELP_BTN"
#define ID_OBJECTIVES_BTN "ID_OBJECTIVES_BTN"

#define ID_SELECTION_HALO "ID_SELECTION_HALO"

#define ID_TIMER "ID_TIMER"

class GameHUD : public VRefCounter
{
public:
	GameHUD() : VRefCounter(),
		buttonsEnabled(false),
		corePanel(NULL),
		infoPanel(NULL),
		selectionHaloPanel(NULL),
		actionPanel(NULL),
		interfacePanel(NULL),
		miscPanel(NULL),
		timerPanel(NULL),
		objectivesMenu(NULL),
		pauseMenu(NULL),
		helpMenu(NULL),
		achievementUnlockedPanel(NULL),
		rallySphereHalo(NULL),
		team1SacrifiedOrbsCountLabel(NULL),
		team2SacrifiedOrbsCountLabel(NULL),
		team3SacrifiedOrbsCountLabel(NULL),
		team4SacrifiedOrbsCountLabel(NULL),
		corePanelDismissed(false),
		infoPanelDismissed(false),
		activeTravelPath(NULL),
		activeTravelPathEndSphere(NULL),
		activeRallyPath(NULL)
	{};
	~GameHUD();

	void init();

	virtual void update(float dt);

	//update functions related to a node sphere
	void updateHUDForPickedNodeSphere();
	void updateSelectionHaloPanelPosition();
	void updateRallySphereHaloPosition();
	void updateBuildSphereHaloPosition();
	void updateInfoPanel();
	void updatePathsPositions();

	//update functions related globals variables (sacrified orbs, time remaining, total population...)
	void updateGlobalVariables();

	////ALL PANELS
	//construction
	void buildCorePanel();
	void buildInfoPanel();
	void buildActionPanel();
	void buildMoveSubActionPanel();
	void buildRallySubActionPanel();
	void buildSelectionHaloPanel();
	void buildInterfacePanel();
	void buildMiscPanel();
	void buildTimerPanel();
	
	//show/dismiss
	void showCorePanel();
	void removeMetaOrbButton(int id);
	void showInfoPanel();
	void dismissCorePanel();
	void dismissInfoPanel();
	void dismissInterfaceMenu();
	void showObjectivesMenu(bool withVeil, bool autoClose, bool announceNewObjectives, const vector<int> &objectivesNumbers);
	void dismissObjectivesMenu();
	void showPauseMenu(bool withVeil, bool autoClose);
	void dismissPauseMenu();
	void showHelpMenu(bool withVeil, bool autoClose);
	void dismissHelpMenu();
	void showSelectionHaloPanel();
	void dismissSelectionHaloPanel();
	void showActionPanel();
	void dismissActionPanel();
	void showMoveSubActionPanel();
	float dismissMoveSubActionPanel();
	void dismissRallySubActionPanel();
	void dismissBuildSubActionPanel();
	void showMiscPanel();
	void dismissMiscPanel();
	void showAchievementUnlocked(string achievementID);
	void dismissAchievementUnlocked();
	void removeAchievementUnlocked();
	
	//opacity of the selectionHalo
	float updateSelectionHaloPanelOpacity();

	//destroy
	void removeObjectivesMenu();
	void removePauseMenu();
	void removeHelpMenu();

	//objectives
	void completeObjective(int number);

	//click
	bool processPointerEvent(hkvVec2 touchLocation, int eventType);
	bool panelProcessPointerEvent(ScreenMaskPanel* panel, hkvVec2 touchLocation, int eventType);
	bool containsClick(hkvVec2 touchLocation);
	bool panelContainsClick(ScreenMaskPanel* panel, hkvVec2 touchLocation);

	//textlabels
	vector<PrintTextLabel*> retrieveAllTextLabels();

	//paths
	void showTravelPath();
	void drawTravelPath(NodeSphere* startSphere, NodeSphere* endSphere, hkvVec2 endTouchLocation);
	void removeActiveTravelPath();
	void showRallySphereHalo(bool animated);
	void showRallySpherePath();
	void removeRallySphereHalo();
	void removeActiveRallyPath();
	Path* getActiveTravelPath(){return this->activeTravelPath;};
	Path* getActiveRallyPath(){return this->activeRallyPath;};
	void setActiveTravelPath(Path* path){this->activeTravelPath = path;};
	void setActiveRallyPath(Path* path){this->activeRallyPath = path;};

	//bridges
	void removePotentialBridges();
	vector<Bridge*> &getPotentialBridges(){return this->potentialBridges;};
	void setPotentialBridges(const vector<Bridge*> &potentialBridges){this->potentialBridges = potentialBridges;};
	void showBuildSphereHalo();
	void removeBuildSphereHalo();

	//getters and setters
	NodeSphere* getNodeSphere();
	ScreenMaskPanel* getSelectionHaloPanel(){return this->selectionHaloPanel;};
	CorePanel* getCorePanel(){return this->corePanel;};
	ActionPanel* getActionPanel(){return this->actionPanel;};
	MiscPanel* getMiscPanel(){return this->miscPanel;};
	PauseMenu* getPauseMenu(){return this->pauseMenu;};
	ObjectivesMenu* getObjectivesMenu(){return this->objectivesMenu;};
	HelpMenu* getHelpMenu(){return this->helpMenu;};

private:
	bool buttonsEnabled;

	//top panel (core items buttons)
	VSmartPtr<CorePanel> corePanel;

	//left panel
	VSmartPtr<ScreenMaskPanel> infoPanel;

	//selection halo
	VSmartPtr<ScreenMaskPanel> selectionHaloPanel;

	//action panel (move, rally, build...)
	VSmartPtr<ActionPanel> actionPanel;

	//interface panel (help and pause)
	VSmartPtr<ScreenMaskPanel> interfacePanel;

	//panel where miscellaneous infos are displayed about the current level
	VSmartPtr<MiscPanel> miscPanel;

	//timer
	VSmartPtr<ScreenMaskPanel> timerPanel;

	//objective menu
	VSmartPtr<ObjectivesMenu> objectivesMenu;

	//help menu
	VSmartPtr<HelpMenu> helpMenu;

	//pause menu
	VSmartPtr<PauseMenu> pauseMenu;

	//achievement unlocked panel
	VSmartPtr<ScreenMaskPanel> achievementUnlockedPanel;

	//rally sphere halo
	VSmartPtr<EndpointHalo> rallySphereHalo;

	//build sphere halo
	VSmartPtr<EndpointHalo> buildSphereHalo;

	//paths
	Path* activeTravelPath;
	NodeSphere* activeTravelPathEndSphere;
	Path* activeRallyPath;

	//potential bridges
	vector<Bridge*> potentialBridges;

	//text labels
	VSmartPtr<PrintTextLabel> team1SacrifiedOrbsCountLabel;
	VSmartPtr<PrintTextLabel> team2SacrifiedOrbsCountLabel;
	VSmartPtr<PrintTextLabel> team3SacrifiedOrbsCountLabel;
	VSmartPtr<PrintTextLabel> team4SacrifiedOrbsCountLabel;

	//bool knobPressed;
	bool corePanelDismissed;
	bool infoPanelDismissed;
};

class CorePanel : public ScreenMaskPanel
{
public:
	CorePanel() : ScreenMaskPanel(), selectedMetaOrb(0) {};

	void buildBackground();
	void buildAndShowMainContent();
	void showGauge(int id, bool animated);
	void showGaugeUpgradePanel(); //after a click on the '+' upgrade button

	void setSelectedMetaOrb(MetaOrb* metaOrb){this->selectedMetaOrb = metaOrb;};
	MetaOrb* getSelectedMetaOrb(){return this->selectedMetaOrb;};

private:
	MetaOrb* selectedMetaOrb;
};

#define ID_CORE_PANEL_BUTTON_ARTIFACT_ANNIHILATION "ID_CORE_PANEL_BUTTON_ARTIFACT_ANNIHILATION"
#define ID_CORE_PANEL_BUTTON_ARTIFACT_REVELATION "ID_CORE_PANEL_BUTTON_ARTIFACT_REVELATION"
#define ID_CORE_PANEL_BUTTON_ARTIFACT_BERZERK "ID_CORE_PANEL_BUTTON_ARTIFACT_BERZERK"

class CorePanelButton : public ButtonScreenMask
{
public:
	CorePanelButton() : ButtonScreenMask(){};
	~CorePanelButton() {};

	V_DECLARE_DYNCREATE(CorePanelButton);

	virtual void onClick();
};

#define ID_GAUGE_CURSOR_DELTA "ID_GAUGE_CURSOR_DELTA"

class Gauge : public ScreenMaskPanel/*, public Touchable*/
{
public:
	Gauge() : ScreenMaskPanel(),
		m_pParentMetaOrb(NULL),
		m_pController(NULL),
		m_pGaugeFill(NULL),
		m_pGaugeFillTarget(NULL),
		m_pCurrentValueTextLabel(NULL), 
		m_pTargetValueTextLabel(NULL),
		m_bCursorSelected(false),
		m_bGaugeFull(false),
		m_iCurrentOrbsValue(0),
		m_iTargetOrbsValue(0),
		m_iMaxReachableOrbsValue(0),
		m_iMaxOrbsValue(0){};
	
	V_DECLARE_DYNCREATE(Gauge);

	void init(hkvVec2 position, MetaOrb* parentMetaOrb);

	virtual void update(float dt);
	void updateValues();
	void updateFill();

	//virtual bool containsTouch(hkvVec2 touchLocation);
	//virtual void onPointerDown(hkvVec2 touchLocation);
	//virtual bool onPointerMove(hkvVec2 touchLocation);
	//virtual void onPointerUp(hkvVec2 touchLocation);

	void setController(GaugeController* controller){this->m_pController = controller;};
	int getCurrentOrbsValue(){return this->m_iCurrentOrbsValue;};
	int getTargetOrbsValue(){return this->m_iTargetOrbsValue;};
	int getMaxReachableOrbsValue(){return this->m_iMaxReachableOrbsValue;};
	int getMaxOrbsValue(){return this->m_iMaxOrbsValue;};

private:
	MetaOrb* m_pParentMetaOrb;
	GaugeController* m_pController;

	PrintTextLabel* m_pCurrentValueTextLabel;
	PrintTextLabel* m_pTargetValueTextLabel;
	VSmartPtr<SpriteScreenMask> m_pGaugeFill;
	VSmartPtr<SpriteScreenMask> m_pGaugeFillTarget;

	bool m_bCursorSelected;
	bool m_bGaugeFull;

	int m_iCurrentOrbsValue; //the current number of orbs in the meta-orb
	int m_iTargetOrbsValue; //the number of orbs the meta-orb is targeting
	int m_iMaxReachableOrbsValue; //the max number of orbs the nodesphere can deliver to the metaOrb
	int m_iMaxOrbsValue; //meta-orb capacity
};

#define ID_GAUGE_CURRENT_VALUE_TEXT_LABEL "ID_GAUGE_CURRENT_VALUE_TEXT_LABEL"

class GaugeController : public Slider
{
public:
	GaugeController() : m_pParentGauge(NULL),
		m_iCurrentValue(0),
		m_bEnabled(false){};

	V_DECLARE_DYNCREATE(GaugeController);

	void init(Gauge* gauge, hkvVec2 position);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	void updateCurrentValueTextLabel();

	int getCurrentValue(){return this->m_iCurrentValue;};

	virtual void update(float dt);

	void setEnabled(bool enabled){this->m_bEnabled = enabled;};

private:
	Gauge* m_pParentGauge;
	
	int m_iCurrentValue;
	bool m_bEnabled; //gaugecontroller is disabled when user clicks on cancel/validate and the content fades out. It is used to prevent m_iCurrentValue to be updated and textlabel refreshed
};

#define ID_GAUGE_UPDATE_BUTTON_GENERATION "ID_GAUGE_UPDATE_BUTTON_GENERATION"
#define ID_GAUGE_UPDATE_BUTTON_DEFENSE "ID_GAUGE_UPDATE_BUTTON_DEFENSE"
#define ID_GAUGE_UPDATE_BUTTON_ASSAULT "ID_GAUGE_UPDATE_BUTTON_ASSAULT"

class GaugeUpdateButton : public ButtonScreenMask
{
public:
	virtual void onClick();
};

#define ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE "ID_MISC_PANEL_INFO_COUNTDOWN_TIMER_MESSAGE"
#define ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT "ID_MISC_PANEL_INFO_SACRIFIED_ORBS_COUNT"
#define ID_MISC_PANEL_ALERT_NEW_OBJECTIVES "ID_MISC_PANEL_ALERT_NEW_OBJECTIVES"
#define ID_MISC_PANEL_ALERT_ACHIEVEMENT_UNLOCKED "ID_MISC_PANEL_ALERT_ACHIEVEMENT_UNLOCKED"
#define ID_MISC_PANEL_COUNTDOWN_TIMER_VALUE "ID_MISC_PANEL_COUNTDOWN_TIMER_VALUE"

class MiscPanel : public ScreenMaskPanel
{
public:
	MiscPanel() : ScreenMaskPanel(),
		m_iDisplayedInfoID(0),
		m_pTimer(0),
		m_fPendingAlertElapsedTime(0),
		m_pCurrentAlert(NULL)
	{};
	~MiscPanel();

	virtual void init(hkvVec2 position);

	void buildCountdownTimerMessage(string strTitle, float fStartTimeInSecs);
	void buildSacrifiedOrbsCount();
	void buildAndStackNewObjectivesAlert();
	void buildAndStackAchievementUnlockedAlert(string strAchievementID);

	//void showCountdownTimerMessage();
	//void showSacrifiedOrbsCount();
	//void showNewObjectivesAlert();

	void showInfo(const char* pszInfoID);
	void showNextAlert();
	void dismissCurrentDisplayedInfo();
	void dismissAndRemoveAlert(MiscPanelAlert* pAlert);

	virtual void update(float dt);

	void setDisplayedInfoID(int iID){this->m_iDisplayedInfoID = iID;};

private:
	int m_iDisplayedInfoID;
	vector<MiscPanelAlert*> m_pendingAlerts;
	float m_fPendingAlertElapsedTime;
	MiscPanelAlert* m_pCurrentAlert;

	//countdown
	VSmartPtr<CountdownTimer> m_pTimer; //the actual timer
};

#define MISC_PANEL_ALERT_DURATION 3.0f

class MiscPanelAlert : public ClickableScreenMaskPanel
{
public:
	virtual void onClick();

	void removeSelf();
};

#define ID_SACRIFIED_ORBS_COUNT_TEXT_LABEL "ID_SACRIFIED_ORBS_COUNT_TEXT_LABEL"

class MiscPanelSacrifiedOrbsLabel : public ScreenMaskPanel
{
public:
	void init(hkvVec2 position, Team* pTeam);
	void updateOrbsCountValue();

private:
	Team* m_pTeam;
};

#define ID_INTERFACE_MENU_BACKGROUND "ID_INTERFACE_MENU_BACKGROUND"
#define ID_INTERFACE_MENU_TOP_LEFT_ANGLE "ID_INTERFACE_MENU_TOP_LEFT_ANGLE"
#define ID_INTERFACE_MENU_TOP_RIGHT_ANGLE "ID_INTERFACE_MENU_TOP_RIGHT_ANGLE"
#define ID_INTERFACE_MENU_BOTTOM_LEFT_ANGLE "ID_INTERFACE_MENU_BOTTOM_LEFT_ANGLE"
#define ID_INTERFACE_MENU_BOTTOM_RIGHT_ANGLE "ID_INTERFACE_MENU_BOTTOM_RIGHT_ANGLE"
#define ID_INTERFACE_MENU_TOP_CONTOUR "ID_INTERFACE_MENU_TOP_CONTOUR"
#define ID_INTERFACE_MENU_RIGHT_CONTOUR "ID_INTERFACE_MENU_RIGHT_CONTOUR"
#define ID_INTERFACE_MENU_BOTTOM_CONTOUR "ID_INTERFACE_MENU_BOTTOM_CONTOUR"
#define ID_INTERFACE_MENU_LEFT_CONTOUR "ID_INTERFACE_MENU_LEFT_CONTOUR"
#define ID_INTERFACE_MENU_CLOSE_BUTTON "ID_INTERFACE_MENU_CLOSE_BUTTON"

#define INTERFACE_MENU_AUTO_CLOSE_TIME 5.0f

class InterfaceMenu : public ScreenMaskPanel
{
public:
	InterfaceMenu() : ScreenMaskPanel(),
		size(hkvVec2(0,0)),
		blackVeil(NULL),
		autoClose(false),
		autoCloseElapsedTime(0),
		autoCloseTime(0){};
	~InterfaceMenu();

	void init();
	void show(hkvVec2 size, bool withVeil, bool autoClose);
	void buildAndShowMainFrame(bool withVeil, bool autoClose);
	void animateMainFrame(hkvVec2 finalBox);

	void dismiss();
	
	void update(float dt);

	void setAutoClose(bool showing){this->autoClose = autoClose;};
	bool isAutoClose(){return this->autoClose;};

protected:
	hkvVec2 size;
	VSmartPtr<Veil> blackVeil;

	bool autoClose;
	float autoCloseElapsedTime;
	float autoCloseTime;
};

class PauseMenu : public InterfaceMenu
{
public:
	PauseMenu() : InterfaceMenu(), 
	mainContentHolder(NULL),
	secondaryContentHolder(NULL){};

	void init();
	void show(bool withVeil, bool autoClose);
	void buildMainContent();
	void buildAndShowSecondaryContent(int clickedBtnID);
	void showMainContent(bool bAnimateMainFrame);
	void dismissMainContent();
	void dismissSecondaryContent();
	void moveAngles(hkvVec2 finalbox);

	ScreenMaskPanel* getSecondaryContent(){return this->secondaryContentHolder;};

private:
	ScreenMaskPanel* mainContentHolder; //buttons sound, music, game speed...
	ScreenMaskPanel* secondaryContentHolder; //popups (retry, go to menu)
};

#define HELP_MENU_NUM_PAGES 1

class HelpMenu : public InterfaceMenu
{
public:
	HelpMenu() : InterfaceMenu(), m_iCurrentPageNumber(0){};

	void init();
	void show(bool withVeil, bool autoClose);
	void buildArrows();
	void buildPage1(); //controls
	void buildPage2(); //action buttons
	void buildPage3(); //meta orbs

	void showPage(int iPageNumber);

	vector<ScreenMaskPanel*> getPages(){return this->m_pages;};
	int getCurrentPageNumber(){return this->m_iCurrentPageNumber;};
	int getPageCount(){return (int) m_pages.size();};

private:
	vector<ScreenMaskPanel*> m_pages;
	int m_iCurrentPageNumber;
};

class ObjectivesMenu : public InterfaceMenu
{
public:
	void init();
	void show(bool withVeil, bool autoClose, bool announceNewObjectives, const vector<int>& objectivesNumbers);
	void buildAndShow(bool announceNewObjectives, const vector<int>& objectivesNumber);

	ObjectiveLabel* findObjectiveLabelByNumber(int number);
	void completeObjective(int number);
};

#define ID_HELP_MENU_ARROW_LEFT "ID_HELP_MENU_ARROW_LEFT"
#define ID_HELP_MENU_ARROW_RIGHT "ID_HELP_MENU_ARROW_RIGHT"

class HelpMenuButton : public ButtonScreenMask
{
public:
	void updateState(int iCurrentPageNumber);

	virtual void onClick();
};

class InterfaceCloseButton : public ButtonScreenMask
{
public:
	void init(hkvVec2 position);
	
	void fadeIn();
	void fadeOut();

	void setAside();

	virtual void onClick();
};

class ActionButton : public ButtonScreenMask
{
public:
	ActionButton() : ButtonScreenMask(), visible(true){};
	~ActionButton();

	bool trigger(hkvVec2 touchLocation, int eventType);
	virtual void onClick();

	bool isVisible(){return this->visible;};
	void hide(){this->SetOpacity(0.0f); this->visible = false;};
	void show(){this->SetOpacity(1.0f); this->visible = true;};

private:
	bool visible;
};

#define ID_INTERFACE_MENU_BTN_EXIT "ID_INTERFACE_MENU_BTN_EXIT"

class InterfaceButton : public ButtonScreenMask
{
public:
	InterfaceButton() : ButtonScreenMask() {};

	virtual void onClick();
};

#define ID_PAUSE_MENU_BTN_SAVE "ID_PAUSE_MENU_BTN_SAVE"
#define ID_PAUSE_MENU_BTN_RETRY "ID_PAUSE_MENU_BTN_RETRY"
#define ID_PAUSE_MENU_BTN_MENU "ID_PAUSE_MENU_BTN_MENU"
#define ID_PAUSE_MENU_BTN_MUSIC "ID_PAUSE_MENU_BTN_MUSIC"
#define ID_PAUSE_MENU_BTN_SOUND "ID_PAUSE_MENU_BTN_SOUND"

class PauseMenuButton : public ButtonScreenMask
{
public:
	PauseMenuButton() : ButtonScreenMask() {};

	virtual void onClick();
};

class ObjectiveLabel : public ScreenMaskPanel
{
public:
	ObjectiveLabel();

	V_DECLARE_DYNCREATE(ObjectiveLabel);

	void init(hkvVec2 pos, int maxCharsPerLine, Objective* objective, float fontHeight, VColorRef fontColor, int number);

	void complete(); //update the texture to mark this objective as complete

	//void IncPos(hkvVec2 deltaPosition);

	hkvVec2 getPos(){return this->pos;};
	hkvVec2 getSize(){return hkvVec2(this->width, this->height);};
	int getNumber(){return this->number;};

private:
	//content
	hkvVec2 pos;
	float width;
	float height;

	int number; //the objective number
};

#define XP_EARNED_PER_PRIMARY_OBJECTIVE 50
#define XP_EARNED_PER_SECONDARY_OBJECTIVE 50
#define XP_EARNED_PER_ORB 1

//END MENU
class DefeatVictoryMenu : public VRefCounter /*, public IVisCallbackHandler_cl*/
{
public:
	DefeatVictoryMenu();
	~DefeatVictoryMenu();

	void init(bool victory);

	void show();
	void showBackground();
	void showContent();
	void showBanners();
	void showInfosBg();
	void showInfos();
	void showButtons();

	void revealPrimaryObjectives();
	void revealSecondaryObjectives();
	void revealKilledOrbs();
	void showXP();

	bool processPointerEvent(hkvVec2 touchLocation, int eventType);

	void removeSelf();

	//void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	//void updateInfosBgSize();

private:
	VSmartPtr<Veil> bg;
	VSmartPtr<ScreenMaskPanel> topBanner;
	VSmartPtr<ScreenMaskPanel> bottomBanner;
	VSmartPtr<SpriteScreenMask> infosBg;
	VSmartPtr<ScreenMaskPanel> infosPanel;
	VSmartPtr<ScreenMaskPanel> buttonsPanel;

	bool victory; //victory or defeat screen

	bool contentShown;
};


#define ID_BTN_MENU "ID_BTN_MENU"
#define ID_BTN_CONTINUE "ID_BTN_CONTINUE"
#define ID_BTN_RETRY "ID_BTN_RETRY"

class DefeatVictoryMenuButton : public ButtonScreenMask
{
public:
	DefeatVictoryMenuButton() : ButtonScreenMask() {};

	void init(hkvVec2 position, bool victory);

	virtual void onClick();
};

enum
{
	ACTION_NORMAL_MODE = 0,
	ACTION_TRAVEL_MODE,
	ACTION_RALLY_POINT_MODE,
	ACTION_BUILD_BRIDGE_MODE,
	ACTION_SACRIFICE_MODE
}; 

#define NORMAL_MODE_COLOR V_RGBA_WHITE
#define TRAVEL_MODE_COLOR V_RGBA_GREEN
#define RALLY_POINT_MODE_COLOR V_RGBA_YELLOW
#define BUILD_MODE_COLOR VColorRef(0,255,255)
#define SACRIFICE_MODE_COLOR V_RGBA_RED
#define ARTIFACT_MODE_COLOR V_RGBA_RED

class ActionPanel : public ScreenMaskPanel
{
public:
	ActionPanel() : ScreenMaskPanel(), 
		parentNodeSphere(NULL), 
		actionMode(0),
		moveBtn(NULL),
		rallyBtn(NULL),
		buildBtn(NULL)
	{};

	void init(hkvVec2 position, NodeSphere* parentNodeSphere);

	void updateActionMode(int newActionMode); // color of the halo (green:move, blue:build, yellow:rally point, red:sacrificepit)

	NodeSphere* getParentNodeSphere(){return this->parentNodeSphere;};
	int getActionMode(){return this->actionMode;};
	void setActionMode(int actionMode){this->actionMode = actionMode;};

	void showMoveBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay);
	void showRallyBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay);
	void showBuildBtn(hkvVec2 toPosition, hkvVec2 toScale, bool animated, float duration, float delay);
	void dismissMoveBtn(bool animated, float delay);
	void dismissRallyBtn(bool animated, float delay);
	void dismissBuildBtn(bool animated, float delay);

private:
	NodeSphere* parentNodeSphere;
	int actionMode;

	ActionButton* moveBtn;
	ActionButton* rallyBtn;
	ActionButton* buildBtn;
};

#define ID_MOVE_SUB_PANEL "ID_MOVE_SUB_PANEL"
#define ID_MOVE_SUB_PANEL_SLIDER_FILL "ID_MOVE_SUB_PANEL_SLIDER_FILL"
#define ID_MOVE_SUB_PANEL_SLIDER_KNOB "ID_MOVE_SUB_PANEL_SLIDER_KNOB"
#define ID_MOVE_SUB_PANEL_ORBS_COUNT "ID_MOVE_SUB_PANEL_ORBS_COUNT"
#define ID_MOVE_SUB_PANEL_SLIDER_VALIDATE "ID_MOVE_SUB_PANEL_SLIDER_VALIDATE"
#define ID_MOVE_SUB_PANEL_SLIDER_CANCEL "ID_MOVE_SUB_PANEL_SLIDER_CANCEL"
#define ID_MOVE_SUB_PANEL_SLIDER "ID_MOVE_SUB_PANEL_SLIDER"
#define ID_RALLY_SUB_PANEL "ID_RALLY_SUB_PANEL"
#define ID_RALLY_SUB_PANEL_VALIDATE "ID_RALLY_SUB_PANEL_VALIDATE"
#define ID_RALLY_SUB_PANEL_CANCEL "ID_RALLY_SUB_PANEL_CANCEL"
#define ID_RALLY_SUB_PANEL_DELETE "ID_RALLY_SUB_PANEL_DELETE"
#define ID_BUILD_SUB_PANEL "ID_BUILD_SUB_PANEL"
#define ID_BUILD_SUB_PANEL_VALIDATE "ID_BUILD_SUB_PANEL_VALIDATE"
#define ID_BUILD_SUB_PANEL_CANCEL "ID_BUILD_SUB_PANEL_CANCEL"

class MoveSubMenuPanel : public ScreenMaskPanel
{
public:
	MoveSubMenuPanel() : ScreenMaskPanel(), 
		m_iCurrentMovingOrbsCount(0),
		m_iMaxOrbsToMove(0),
		m_bOrbsCountMin(false),
		m_bOrbsCountMax(false),
		dismissedPosition(hkvVec2(0,0)),
		shownPosition(hkvVec2(0,0)),
		dismissed(false){};

	void init(hkvVec2 position);
	void cacheMaxOrbsToMove();

	virtual void update(float dt);
	void updateOrbsCountTextLabel();

	void onSliderMove(float percentage);

	int getCurrentMovingOrbsCount(){return this->m_iCurrentMovingOrbsCount;};
	void setCurrentMovingOrbsCount(int orbsCount){this->m_iCurrentMovingOrbsCount = orbsCount;};

	void setOrbsCountMin(bool min){this->m_bOrbsCountMin = min;};
	void setOrbsCountMax(bool max){this->m_bOrbsCountMax = max;};
	bool isOrbsCountMin(){return this->m_bOrbsCountMin;};
	bool isOrbsCountMax(){return this->m_bOrbsCountMax;};

	bool isDismissed(){return this->dismissed;};
	void setDismissed(bool dismissed){this->dismissed = dismissed;};
	hkvVec2 getDismissedPosition(){return this->dismissedPosition;};
	hkvVec2 getShownPosition(){return this->shownPosition;};
	void setDismissedPosition(hkvVec2 dismissedPosition){this->dismissedPosition = dismissedPosition;};
	void setShownPosition(hkvVec2 shownPosition){this->shownPosition = shownPosition;};

private:
	int m_iCurrentMovingOrbsCount;
	int m_iMaxOrbsToMove;

	bool m_bOrbsCountMin; //have we reached the lower limit
	bool m_bOrbsCountMax; //have we reached the upper limit

	bool dismissed;
	hkvVec2 dismissedPosition;
	hkvVec2 shownPosition;
};

class MoveSubMenuSlider : public ScreenMaskPanel, public Touchable
{
public:
	MoveSubMenuSlider() : sliderBg(NULL), sliderFill(NULL), sliderKnob(NULL), knobSelected(false){};
	~MoveSubMenuSlider();

	V_DECLARE_DYNCREATE(MoveSubMenuSlider);

	void init(hkvVec2 position);
	void setKnobPositionForPercentage(float fPercentage);

	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);

private:
	VSmartPtr<SpriteScreenMask> sliderBg;
	VSmartPtr<SpriteScreenMask> sliderFill;
	VSmartPtr<SpriteScreenMask> sliderKnob;

	bool knobSelected;
};

class RallySubMenuPanel : public ScreenMaskPanel
{
public:
	RallySubMenuPanel() : ScreenMaskPanel(){};

	void showBtn(const char* id, float delay);
	void hideBtn(const char* id, float delay);
	void hideAll(float delay);
};

class BuildSubMenuPanel : public ScreenMaskPanel
{
public:
	BuildSubMenuPanel() : ScreenMaskPanel(){};

	void init(hkvVec2 position);

	void showBtn(const char* id, float delay);
	void hideBtn(const char* id, float delay);
	void hideAll(float delay);
};

class GameSpeedSlider : public Slider
{
public:
	GameSpeedSlider() : Slider(){};
	~GameSpeedSlider() {};

	V_DECLARE_DYNCREATE(GameSpeedSlider);

	void init(hkvVec2 position);

	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);

};

class QuitDialogPanel : public YesNoDialogPanel
{
public:
	void init();

	virtual void onClickYes();
	virtual void onClickNo();
};

class RetryDialogPanel : public YesNoDialogPanel
{
public:
	void init();

	virtual void onClickYes();
	virtual void onClickNo();
};
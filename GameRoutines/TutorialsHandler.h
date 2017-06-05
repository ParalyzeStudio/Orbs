#pragma once

class TutorialMessage;

struct Tutorial
{
	string tag;
	float displayTime;
	float intervalTime;
	vector<string> texts;
};

class TutorialsHandler : public ResourcePreloader
{
public:
	~TutorialsHandler() {};
	
	static TutorialsHandler& sharedInstance(){return instance;};

	void preloadStringsFiles();
	void buildTutorialsFromXML();

	void showTutorialGroup1();
	void showTutorialGroup2();
	void showTutorialGroup3();
	void showTutorialGroup4();
	void showTutorialGroup5();
	void showTutorialGroup6();

	void onFinishDisplayingTutorialMessage(VBaseObject* sender);

	Tutorial* getTutorialForTag(string tutorialTag);

	bool isMoveBtnRevealed(){return this->moveBtnRevealed;};
	bool isRallyBtnRevealed(){return this->rallyBtnRevealed;};
	bool isBuildBtnRevealed(){return this->buildBtnRevealed;};
	bool isCorePanelRevealed(){return this->corePanelRevealed;};

	int getSphereCountInsideCameraBoundaries(){return this->sphereCountInsideCameraBoundaries;};

private:
	TutorialsHandler() : lastTutorialGroupDisplayed(0), currentDisplayedTutorialMessage(NULL){};
	static TutorialsHandler instance;

	TiXmlDocument tutorialsDoc;
	vector<Tutorial> tutorials;

	stack<TutorialMessage*> stackedTutorialsGroups;

	int lastTutorialGroupDisplayed;
	TutorialMessage* currentDisplayedTutorialMessage;

	bool moveBtnRevealed;
	bool rallyBtnRevealed;
	bool buildBtnRevealed;
	bool corePanelRevealed;

	int sphereCountInsideCameraBoundaries;
};

class TutorialMessage : public ScreenMaskPanel
{
public:
	TutorialMessage() : ScreenMaskPanel(), tutorial(NULL), evacuated(false){};
	~TutorialMessage();

	void init(hkvVec2 position, Tutorial* tutorial, float lineOffset);
	void show();
	void dismiss();

	Tutorial* getTutorial(){return this->tutorial;};
	bool isEvacuated(){return this->evacuated;};
	void setEvacuated(bool evacuated){this->evacuated = evacuated;};

private:
	vector<HUDMessage*> hintsMessages;
	Tutorial* tutorial;
	bool evacuated;
};


#pragma once

class HUDMessage;
class IntroMessage;

class MessageHandler : public VBaseObject, public IVisCallbackHandler_cl
{
public:
	void OneTimeInit();
	void OneTimeDeInit();

	static MessageHandler &sharedInstance(){return instance;};

	void showErrorMessage(string text); //top left hand corner message, red color
	void showObjectiveCompletedMessage(); //above objectives panel, remains visible as long as the user doesn't open the objectives panel (can display objective complete, or new objective)
	void showIntroMessage(string text);
	void announceObjectives();
	//void showCountdownTimerMessage(string text, int startTime);

	void queueAndShowIntroMessages(int group, float delay); //centered messages used for introductions, and presenting new objectives
	//void announceObjectives();
	void dismissObjectivesAnnouncer();
	void removeObjectivesAnnouncer();

	void removeActiveMessage(HUDMessage* message);
	void removeAllActiveMessages();

	void stopDisplayingIntroMessage();

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	virtual void update(float dt);

	vector<HUDMessage*>& getActiveMessages(){return this->activeMessages;};

private:
	static MessageHandler instance;
	vector<HUDMessage*> activeMessages;
	stack<string> queuedIntroMessages;
	VSmartPtr<ScreenMaskPanel> objectivesAnnouncer;

	float showIntroMessagesDelay;
	float introMessagesDelayElapsedTime;
	bool displayingIntroMessage;
};

#define ID_HUD_MESSAGE_TEXT_LABEL "ID_HUD_MESSAGE_TEXT_LABEL"

class HUDMessage : public ScreenMaskPanel
{
public:
	HUDMessage() : ScreenMaskPanel(), savedPosition(hkvVec2(0,0)) {};

	virtual void init(hkvVec2 position, string text);
	virtual void removeSelf(){MessageHandler::sharedInstance().removeActiveMessage(this);};

	virtual void hide(); //hide this message temporarily by setting its position out of screen
	virtual void unhide(); //restore this message on the screen

	void fadeIn();
	void fadeOut();

	PrintTextLabel* getTextLabel(){return (PrintTextLabel*) this->findElementById(ID_HUD_MESSAGE_TEXT_LABEL);};

private:
	hkvVec2 savedPosition; //saved position of the message before it was hidden
};

class ErrorMessage : public HUDMessage
{
public:
	ErrorMessage() : HUDMessage() {};

	virtual void init(string text);
};

class ObjectiveMessage : public HUDMessage
{
public:
	ObjectiveMessage() : HUDMessage() {};

	virtual void init(string text);

	float getWidth(){return this->width;};
	float getHeight(){return this->height;};

private:
	float width;
	float height;
};

class IntroMessage : public HUDMessage
{
public:
	IntroMessage() : HUDMessage() {};

	virtual void init(string text);

	void removeSelf();
};

//#define ID_COUNTDOWN_TIMER "ID_COUNTDOWN_TIMER"
//
//class CountdownTimerMessage : public HUDMessage
//{
//public:
//	CountdownTimerMessage();
//	~CountdownTimerMessage();
//
//
//	virtual void init(string text, int startTime);
//	void updateTimer(float dt);
//
//	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
//
//private:
//	float remainingTime;
//};
#include "OrbsPluginPCH.h"
#include "MessageHandler.h"
#include "GameManager.h"

MessageHandler MessageHandler::instance;

///////////////////////////////MessageHandler///////////////////////////////
void MessageHandler::OneTimeInit()
{
	Vision::Callbacks.OnRenderHook += this;
	displayingIntroMessage = false;
}

void MessageHandler::OneTimeDeInit()
{
	Vision::Callbacks.OnRenderHook -= this;
	this->removeAllActiveMessages();
	this->objectivesAnnouncer = NULL;
}

void MessageHandler::showErrorMessage(string text)
{
	ErrorMessage* errorMessage = new ErrorMessage();
	errorMessage->AddRef();
	errorMessage->init(text);
	errorMessage->fadeTo(0.0f, 1.0f, 2.0f);

	CallFuncObject* releaseMessageAction = new CallFuncObject(errorMessage, (CallFunc)(&HUDMessage::removeSelf), 3.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(releaseMessageAction);
	releaseMessageAction->executeFunc();

	activeMessages.push_back(errorMessage);
}

void MessageHandler::showObjectiveCompletedMessage()
{
	//hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	//float scale = GameDialogManager::sharedInstance()->getScale();

	//string objectiveCompleteText = StringsXmlHelper::sharedInstance().getStringForTag("objective_complete");

	//ObjectiveMessage* objectiveMessage = new ObjectiveMessage();
	//objectiveMessage->AddRef();
	//objectiveMessage->init(objectiveCompleteText);

	//float borderOffset = 10.0f * scale;
	//float ObjectivesMenuPositionY = 0.5f * screenSize.y;
	//float ObjectivesMenuHeight = 512.0f * scale;
	//float messagePanelPositionX = screenSize.x - 0.5f * objectiveMessage->getWidth() - borderOffset;
	//float messagePanelPositionY = ObjectivesMenuPositionY -  0.5f * ObjectivesMenuHeight - 0.5f * objectiveMessage->getHeight();
	//objectiveMessage->SetPosition(hkvVec2(messagePanelPositionX, messagePanelPositionY));

	//CallFuncObject* releaseMessageAction = new CallFuncObject(objectiveMessage, (CallFunc)(&HUDMessage::removeSelf), 3.0f);
	//CallFuncHandler::sharedInstance().addCallFuncInstance(releaseMessageAction);
	//releaseMessageAction->executeFunc();

	//activeMessages.push_back(objectiveMessage);
}

void MessageHandler::showIntroMessage(string text)
{
	displayingIntroMessage = true;

	IntroMessage* introMessage = new IntroMessage();
	introMessage->AddRef();
	introMessage->init(text);

	introMessage->SetOpacity(0.0f);
	introMessage->fadeIn();

	float introMessageDuration = 0.2f;

	CallFuncObject* fadeOutAction = new CallFuncObject(introMessage, (CallFunc)(&HUDMessage::fadeOut), introMessageDuration + 1.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(fadeOutAction);
	fadeOutAction->executeFunc();

	CallFuncObject* removeMessageAction = new CallFuncObject(introMessage, (CallFunc)(&IntroMessage::removeSelf), introMessageDuration + 2.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeMessageAction);
	removeMessageAction->executeFunc();

	CallFuncObject* stopDisplayingAction = new CallFuncObject(this, (CallFunc)(&MessageHandler::stopDisplayingIntroMessage), introMessageDuration + 2.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(stopDisplayingAction);
	stopDisplayingAction->executeFunc();

	activeMessages.push_back(introMessage);

	if (queuedIntroMessages.size() == 0)
	{
		CallFuncObject* startLevelAction = new CallFuncObject(&GameManager::GlobalManager(), (CallFunc)(&GameManager::startLevel), introMessageDuration + 2.0f);
		CallFuncHandler::sharedInstance().addCallFuncInstance(startLevelAction);
		startLevelAction->executeFunc();

		if (LevelManager::sharedInstance().getCurrentLevel()->getNumber() == 1) //1st level, tutorials, no objectives to show
		{
			CallFuncObject* showTutoAction = new CallFuncObject(&TutorialsHandler::sharedInstance(), (CallFunc)(&TutorialsHandler::showTutorialGroup1), introMessageDuration + 2.0f);
			CallFuncHandler::sharedInstance().addCallFuncInstance(showTutoAction);
			showTutoAction->executeFunc();
		}
		else 
		{
			CallFuncObject* announceObjectivesAction = new CallFuncObject(this, (CallFunc)(&MessageHandler::announceObjectives), introMessageDuration + 2.0f);
			CallFuncHandler::sharedInstance().addCallFuncInstance(announceObjectivesAction);
			announceObjectivesAction->executeFunc();
		}
	}
}

void MessageHandler::announceObjectives()
{
	vector<int> emptyVector;
	GameDialogManager::sharedInstance()->getHUD()->showObjectivesMenu(false, false, false, emptyVector);
}

//void MessageHandler::showCountdownTimerMessage(string text, int startTime)
//{
//	CountdownTimerMessage* timerMessage = new CountdownTimerMessage();
//	timerMessage->AddRef();
//	timerMessage->init(text, startTime);
//
//	activeMessages.push_back(timerMessage);
//}

void MessageHandler::queueAndShowIntroMessages(int group, float delay)
{
	this->showIntroMessagesDelay = delay;
	vector<IntroText> &introMessages = LevelManager::sharedInstance().getCurrentLevel()->getIntros();

	for (size_t introMessageIndex = introMessages.size() - 1; introMessageIndex != -1; introMessageIndex--)
	{
		IntroText introMessage = introMessages[introMessageIndex];
		if (group == introMessage.group)
		{
			queuedIntroMessages.push(introMessage.text);
		}
	}

	displayingIntroMessage = false;
}

//void MessageHandler::announceObjectives()
//{
//	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
//	float scale = GameDialogManager::sharedInstance()->getScale();
//
//	vector<Objective*> &allObjectives = LevelManager::sharedInstance().getCurrentLevel()->getObjectives();
//	vector<Objective*> revealedPrimaryObjectives;
//	vector<Objective*> revealedSecondaryObjectives;
//	for (size_t objectiveIndex = 0; objectiveIndex != allObjectives.size(); objectiveIndex++)
//	{
//		Objective* objective = allObjectives[objectiveIndex];
//		if (!objective->isHidden() && !objective->hasBeenAlreadyDisplayed())
//		{
//			if (objective->isPrimary())
//				revealedPrimaryObjectives.push_back(objective);
//			else
//				revealedSecondaryObjectives.push_back(objective);
//			objective->setAlreadyDisplayed(true);
//		}
//	}
//
//	int revealedObjectivesCount = (int) revealedPrimaryObjectives.size() + (int) revealedSecondaryObjectives.size();
//	if (revealedObjectivesCount == 0) //nothing to show
//	{
//		GameManager::GlobalManager().startLevel();
//		return;
//	}
//
//	objectivesAnnouncer = new ScreenMaskPanel();
//	objectivesAnnouncer->init(hkvVec2(0,0));
//
//	float contentHeight = 0;
//	//objective panel header
//	ObjectiveMessage* objectiveMessage = new ObjectiveMessage();
//	objectiveMessage->AddRef();
//	string newObjectivesText;
//	if (revealedObjectivesCount == 1) //Only 1 objective
//		newObjectivesText = StringsXmlHelper::sharedInstance().getStringForTag("new_objective");
//	else
//		newObjectivesText = StringsXmlHelper::sharedInstance().getStringForTag("new_objectives");
//	objectiveMessage->init(newObjectivesText);
//
//	objectiveMessage->SetPosition(hkvVec2(0,0));
//	objectivesAnnouncer->addPanel(objectiveMessage);
//	
//	contentHeight += objectiveMessage->getHeight();
//
//	//first line
//	//VTextureObject* lineTexture = Vision::TextureManager.Load2DTexture("Textures\\colors\\White.png");
//	//float lineWidth = 60.0f * scale;
//	//float lineHeight = 4.0f * scale;
//	//hkvVec2 lineSize = hkvVec2(lineWidth, lineHeight);
//	//float offset = 5.0f * scale;
//	//SpriteScreenMask* firstLine = new SpriteScreenMask();
//	//firstLine->AddRef();
//	//firstLine->init(hkvVec2(0, contentHeight + 0.5f * lineHeight + offset), lineSize, lineTexture);
//	//ObjectivesMenu->addScreenMask(firstLine);
//	//contentHeight += (2 * offset + lineHeight);
//
//	//Actual objectives
//	float objectiveFontHeight = 40 * scale;
//	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, objectiveFontHeight);
//	VColorRef objectiveColor = V_RGBA_WHITE;
//
//	if (revealedPrimaryObjectives.size() != 0)
//	{
//		//primary title
//		PrintTextLabel* primaryTitle = new PrintTextLabel();
//		primaryTitle->AddRef();
//		hkvVec2 primaryTitlePosition = hkvVec2(0, contentHeight + 20.0f * scale);
//		string primaryTitleText = StringsXmlHelper::sharedInstance().getStringForTag("primary_obj");
//		float objectiveTitleFontHeight = 45 * scale;
//		VColorRef primaryTitleColor = V_RGBA_WHITE;
//		primaryTitle->init(primaryTitlePosition, primaryTitleText, neuropolFont, objectiveTitleFontHeight, primaryTitleColor);
//		objectivesAnnouncer->addTextLabel(primaryTitle);
//		contentHeight += (primaryTitle->getSize().y);
//
//
//		//primary objectives
//		ObjectiveLabel* previousObjective;
//		for (size_t primaryObjectiveIndex = 0; primaryObjectiveIndex != revealedPrimaryObjectives.size(); primaryObjectiveIndex++)
//		{
//			Objective* primaryObjective = revealedPrimaryObjectives[primaryObjectiveIndex];
//
//			ObjectiveLabel* objectiveLabel = new ObjectiveLabel();
//			objectiveLabel->AddRef();
//
//			float objectiveWidth = 1000.0f * scale;
//			hkvVec2 objectiveLocalPos;
//			if (primaryObjectiveIndex == 0)
//			{
//				objectiveLocalPos =  hkvVec2(-500.0f * scale, contentHeight + 50.0f * scale);
//			}
//			else
//			{
//				hkvVec2 previousObjectivePos = previousObjective->getPos();
//				objectiveLocalPos = hkvVec2(previousObjectivePos.x, previousObjectivePos.y + previousObjective->getSize().y);
//			}
//			previousObjective = objectiveLabel;
//
//			objectiveLabel->init(objectiveLocalPos, 40, primaryObjective, objectiveFontHeight, objectiveColor, primaryObjectiveIndex + 1);
//			objectivesAnnouncer->addPanel(objectiveLabel);
//			contentHeight += objectiveLabel->getSize().y;
//		}
//	}
//
//	if (revealedSecondaryObjectives.size() != 0)
//	{
//		contentHeight += 80.0f * scale;
//
//		//secondary title
//		PrintTextLabel* secondaryTitle = new PrintTextLabel();
//		secondaryTitle->AddRef();
//		hkvVec2 secondaryTitlePosition = hkvVec2(0, contentHeight + 20.0f * scale);
//		string secondaryTitleText = StringsXmlHelper::sharedInstance().getStringForTag("secondary_obj");
//		float objectiveTitleFontHeight = 45 * scale;
//		VColorRef secondaryTitleColor = V_RGBA_WHITE;
//		secondaryTitle->init(secondaryTitlePosition, secondaryTitleText, neuropolFont, objectiveTitleFontHeight, secondaryTitleColor);
//		objectivesAnnouncer->addTextLabel(secondaryTitle);
//		contentHeight += (secondaryTitle->getSize().y);
//
//
//		//secondary objectives
//		ObjectiveLabel* previousObjective;
//		for (size_t secondaryObjectiveIndex = 0; secondaryObjectiveIndex != revealedSecondaryObjectives.size(); secondaryObjectiveIndex++)
//		{
//			Objective* secondaryObjective = revealedSecondaryObjectives[secondaryObjectiveIndex];
//
//			ObjectiveLabel* objectiveLabel = new ObjectiveLabel();
//			objectiveLabel->AddRef();
//
//			float objectiveWidth = 1000.0f * scale;
//			hkvVec2 objectiveLocalPos;
//			if (secondaryObjectiveIndex == 0)
//			{
//				objectiveLocalPos =  hkvVec2(-500.0f * scale, contentHeight + 50.0f * scale);
//			}
//			else
//			{
//				hkvVec2 previousObjectivePos = previousObjective->getPos();
//				objectiveLocalPos = hkvVec2(previousObjectivePos.x, previousObjectivePos.y + previousObjective->getSize().y);
//			}
//			previousObjective = objectiveLabel;
//
//			objectiveLabel->init(objectiveLocalPos, 40, secondaryObjective, objectiveFontHeight, objectiveColor, secondaryObjectiveIndex + 1);
//			objectivesAnnouncer->addPanel(objectiveLabel);
//			contentHeight += objectiveLabel->getSize().y;
//		}
//	}
//
//	//Position of the global panel
//	hkvVec2 objectivesAnnouncerPosition = hkvVec2(0.5f * screenSize.x, 0.5f * (screenSize.y - contentHeight));
//	objectivesAnnouncer->SetPosition(objectivesAnnouncerPosition);
//
//	objectivesAnnouncer->SetOpacity(0.0f);
//	objectivesAnnouncer->fadeTo(1.0f, 1.0f, 0.0f);
//
//	//dismiss
//	CallFuncObject* dismissObjectivesAction = new CallFuncObject(this, (CallFunc)(&MessageHandler::dismissObjectivesAnnouncer), 1.5f);
//	CallFuncHandler::sharedInstance().addCallFuncInstance(dismissObjectivesAction);
//	dismissObjectivesAction->executeFunc();
//}

void MessageHandler::dismissObjectivesAnnouncer()
{
	objectivesAnnouncer->fadeTo(0.0f, 1.0f, 0.0f);

	CallFuncObject* removeObjectivesAction = new CallFuncObject(this, (CallFunc)(&MessageHandler::removeObjectivesAnnouncer), 1.0f);
	CallFuncHandler::sharedInstance().addCallFuncInstance(removeObjectivesAction);
	removeObjectivesAction->executeFunc();
}

void MessageHandler::removeObjectivesAnnouncer()
{
	this->objectivesAnnouncer = NULL;
}

void MessageHandler::removeActiveMessage(HUDMessage* message)
{
	vector<HUDMessage*>::iterator messagesIt;
	for (messagesIt = activeMessages.begin(); messagesIt != activeMessages.end(); messagesIt++)
	{
		if (message == *messagesIt)
		{
			activeMessages.erase(messagesIt);
			message->Release();
			return;
		}
	}
}

void MessageHandler::stopDisplayingIntroMessage()
{
	this->displayingIntroMessage = false;
}

void MessageHandler::removeAllActiveMessages()
{
	vector<HUDMessage*>::iterator messagesIt;
	for (messagesIt = activeMessages.begin(); messagesIt != activeMessages.end(); messagesIt++)
	{
		(*messagesIt)->Release();
	}
	activeMessages.clear();
}

void MessageHandler::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnRenderHook)
	{
		VisRenderHookDataObject_cl* pRHDO = static_cast<VisRenderHookDataObject_cl*>(pData);
		if (pRHDO->m_iEntryConst == VRH_GUI)
		{		
			float dt = Vision::GetTimer()->GetTimeDifference();
			this->update(dt);
		}
	}
}

void MessageHandler::update(float dt)
{
	if (GameTimer::sharedInstance().isPaused())
		return;

	if (queuedIntroMessages.size() > 0)
	{
		if (introMessagesDelayElapsedTime < showIntroMessagesDelay)
			introMessagesDelayElapsedTime += dt;
		else
		{
			if (!displayingIntroMessage)
			{
				string messageText = queuedIntroMessages.top();
				queuedIntroMessages.pop();
				showIntroMessage(messageText);
			}
		}
	}
}

///////////////////////////////HUDMessage///////////////////////////////
void HUDMessage::init(hkvVec2 position, string text)
{
	ScreenMaskPanel::init(position);

	float scale = GameDialogManager::sharedInstance()->getScale();
	float fontHeight = 50 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fontHeight);

	PrintTextLabel* messageTextLabel = new PrintTextLabel();
	messageTextLabel->AddRef();
	messageTextLabel->init(hkvVec2(0,0), text, neuropolFont, fontHeight, V_RGBA_WHITE);
	messageTextLabel->SetAnchorPoint(hkvVec2(0.5f, 0.5f));
	messageTextLabel->m_iID = VGUIManager::GetID(ID_HUD_MESSAGE_TEXT_LABEL);

	this->addTextLabel(messageTextLabel);
}

void HUDMessage::fadeIn()
{
	this->fadeTo(1.0f, 1.0f, 0.0f);
}

void HUDMessage::fadeOut()
{
	this->fadeTo(0.0f, 1.0f, 0.0f);
}

void HUDMessage::hide()
{
	this->savedPosition = this->GetPosition();
	this->SetPosition(hkvVec2(0, -10000.0f));
}

void HUDMessage::unhide()
{
	this->SetPosition(this->savedPosition);
}

///////////////////////////////ErrorMessage///////////////////////////////
void ErrorMessage::init(string text)
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();
	float fontHeight = 40 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);

	hkvVec2 errorMessagePosition = hkvVec2(10.0f, 0.1f * screenSize.y);
	ScreenMaskPanel::init(errorMessagePosition);

	PrintTextLabel* messageTextLabel = new PrintTextLabel();
	messageTextLabel->AddRef();
	messageTextLabel->init(hkvVec2(0,0), text, neuropolFont, fontHeight, V_RGBA_RED);
	messageTextLabel->SetAnchorPoint(hkvVec2(0.0f, 0.5f));

	this->addTextLabel(messageTextLabel);
}

///////////////////////////////ObjectiveMessage///////////////////////////////
void ObjectiveMessage::init(string text)
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();

	//message (new objective or objective complete)
	float fontHeight = 38 * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_DIGICITY, fontHeight);
	float fontScale = fontHeight / neuropolFont->GetFontHeight();	

	PrintTextLabel* objectiveMessageText = new PrintTextLabel();
	objectiveMessageText->AddRef();
	objectiveMessageText->init(hkvVec2(0,0), text, neuropolFont, fontHeight, VColorRef(198, 253, 255, 255));
	objectiveMessageText->SetAnchorPoint(hkvVec2(0.0f, 0.5f));
	VRectanglef objectiveMessageTextDim;
	neuropolFont->GetTextDimension(text.c_str(), objectiveMessageTextDim);
	objectiveMessageTextDim.operator*=(fontScale);
	float objectiveMessageWidth = objectiveMessageTextDim.GetSizeX();
	float objectiveMessageHeight = objectiveMessageTextDim.GetSizeY();

	//spinning wheel
	VTextureObject* spinningWheelTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc\\spinning_wheel.png");
	float circleWidth = 64 * scale;
	float circleHeight = 64 * scale;
	hkvVec2 circleSize = hkvVec2(circleWidth, circleHeight);

	RotatingSpriteScreenMask* spinningWheel = new RotatingSpriteScreenMask();
	spinningWheel->AddRef();
	spinningWheel->init(hkvVec2(0,0), circleSize, spinningWheelTexture, 180);

	//objective message panel
	float borderOffset = 10.0f * scale;
	this->width = circleWidth + objectiveMessageWidth + borderOffset;
	this->height = circleHeight + objectiveMessageHeight;
	ScreenMaskPanel::init(hkvVec2(0,0));

	hkvVec2 spinningWheelPosition = hkvVec2(-0.5f * (width - circleWidth), 0.0f);
	hkvVec2 objectiveTextPosition = spinningWheelPosition + hkvVec2(0.5f * circleWidth + borderOffset, 0);
	spinningWheel->SetPosition(spinningWheelPosition);
	objectiveMessageText->SetPosition(objectiveTextPosition);
	this->addTextLabel(objectiveMessageText);
	this->addScreenMask(spinningWheel);
}

///////////////////////////////PresentationMessage///////////////////////////////
void IntroMessage::init(string text)
{
	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
	float scale = GameDialogManager::sharedInstance()->getScale();
	float fontHeight = 44.0f * scale;
	VisFont_cl* neuropolFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, fontHeight);

	ScreenMaskPanel::init(0.5f * screenSize);

	vector<string> splitString;
	StringsXmlHelper::sharedInstance().splitStringInArray(splitString, text, 50);
	int substringsCount = (int) splitString.size();
	for (int substringIndex = 0; substringIndex != substringsCount; substringIndex++)
	{
		PrintTextLabel* messageTextLabel = new PrintTextLabel();
		messageTextLabel->AddRef();
		float relativeIndex;
		int halfCount = substringsCount / 2;
		float lineOffset = 80.0f * scale;
		if (substringsCount % 2 == 0) //even
			relativeIndex = (substringIndex < halfCount) ? substringIndex - halfCount + 0.5f : substringIndex - halfCount + 1 - 0.5f;
		else //odd
			relativeIndex = (float) (substringIndex - halfCount);

		hkvVec2 substringPosition = hkvVec2(0, relativeIndex * lineOffset);
		messageTextLabel->init(substringPosition, splitString[substringIndex], neuropolFont, fontHeight, V_RGBA_WHITE);
		messageTextLabel->setRenderedBehindScreenMasks(true);
		
		this->addTextLabel(messageTextLabel);
	}
}

void IntroMessage::removeSelf()
{
	HUDMessage::removeSelf();
}

///////////////////////////////CountdownTimerMessage///////////////////////////////
//CountdownTimerMessage::CountdownTimerMessage() : remainingTime(0)
//{
//	
//}
//
//CountdownTimerMessage::~CountdownTimerMessage()
//{
//	
//}
//
//void CountdownTimerMessage::init(string text, int startTime)
//{
//	this->remainingTime = (float) startTime;
//
//	hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
//	float scale = GameDialogManager::sharedInstance()->getScale();
//
//	ScreenMaskPanel::init(hkvVec2(0.5f * screenSize.x, 56.0f * scale));
//
//	PrintTextLabel* messageTextLabel = new PrintTextLabel();
//	messageTextLabel->AddRef();
//	hkvVec2 messageTextLabelPosition = hkvVec2(0, -18.0f * scale);
//	float messageTextLabelFontHeight = 40.0f * scale;
//	VisFont_cl* messageTextLabelFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, messageTextLabelFontHeight);
//	messageTextLabel->init(messageTextLabelPosition, text, messageTextLabelFont, messageTextLabelFontHeight, V_RGBA_RED);
//	messageTextLabel->setRenderedBehindVeils(true);
//	this->addTextLabel(messageTextLabel);
//
//	PrintTextLabel* timerTextLabel = new PrintTextLabel();
//	timerTextLabel->AddRef();
//	hkvVec2 timerTextLabelPosition = hkvVec2(0, 18.0f * scale);
//	float timerTextLabelFontHeight = 42.0f * scale;
//	VisFont_cl* timerTextLabelFont = FontManager::getFontForNameAndSize(FONT_NEUROPOL, messageTextLabelFontHeight);
//	timerTextLabel->init(timerTextLabelPosition, text, timerTextLabelFont, timerTextLabelFontHeight, V_RGBA_RED);
//	timerTextLabel->m_iID = VGUIManager::GetID(ID_COUNTDOWN_TIMER);
//	timerTextLabel->setRenderedBehindVeils(true);
//	this->addTextLabel(timerTextLabel);
//}
//
//void CountdownTimerMessage::updateTimer(float dt)
//{
//	PrintTextLabel* timerTextLabel = (PrintTextLabel*) this->findElementById(ID_COUNTDOWN_TIMER);
//	this->remainingTime -= (dt * GameTimer::sharedInstance().getTimeScaleFactor(true));
//	if (this->remainingTime <= 0)
//	{
//		this->removeSelf();
//		return;
//	}
//	
//	int remainingTimeInSecs = GeometryUtils::round(remainingTime);
//
//	int hours = remainingTimeInSecs / 3600;
//	remainingTimeInSecs = remainingTimeInSecs % 3600;
//	int mins = remainingTimeInSecs / 60;
//	int secs = remainingTimeInSecs % 60;
//	
//	string hoursStr = (hours < 10) ? stringFromInt(0) + stringFromInt(hours) : stringFromInt(hours);
//	string minsStr = (mins < 10) ? stringFromInt(0) + stringFromInt(mins) : stringFromInt(mins);
//	string secsStr = (secs < 10) ? stringFromInt(0) + stringFromInt(secs) : stringFromInt(secs);
//
//	string timeText;
//	if (hours > 0)
//		timeText = hoursStr + string(":") + minsStr + string(":") + secsStr;
//	else
//		timeText = minsStr + string(":") + secsStr;
//
//	timerTextLabel->setText(timeText);
//}
//
//void CountdownTimerMessage::OnHandleCallback(IVisCallbackDataObject_cl *pData)
//{
//	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
//	{
//		float dt = Vision::GetTimer()->GetTimeDifference();
//		this->updateTimer(dt);
//	}
//}
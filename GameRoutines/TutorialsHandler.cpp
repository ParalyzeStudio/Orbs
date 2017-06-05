#include "OrbsPluginPCH.h"
#include "TutorialsHandler.h"
#include "GameManager.h"

////////////////////////TutorialsHandler////////////////////////
TutorialsHandler TutorialsHandler::instance;

void TutorialsHandler::preloadStringsFiles()
{
	if (!stringsFilesLoaded)
	{
		string tutorialsFilename = GetStringsFolder() + string("tutorials.xml");
		tutorialsDoc.LoadFile(tutorialsFilename.c_str());
		ResourcePreloader::preloadStringsFiles();
	}
}

void TutorialsHandler::buildTutorialsFromXML()
{
	for (TiXmlElement *tutorialNode = tutorialsDoc.RootElement()->FirstChildElement("tutorial"); tutorialNode; tutorialNode=tutorialNode->NextSiblingElement("tutorial"))
	{
		Tutorial tutorial;
		//Tag
		string tag = string(XMLHelper::Exchange_String(tutorialNode, "tag", NULL, false));
		tutorial.tag = tag;

		//display time
		float displayTime;
		XMLHelper::Exchange_Float(tutorialNode, "displayTime", displayTime, false);
		tutorial.displayTime = displayTime;

		//interval time
		float intervalTime;
		XMLHelper::Exchange_Float(tutorialNode, "intervalTime", intervalTime, false);
		tutorial.intervalTime = intervalTime;

		//texts
		for (TiXmlElement *hintNode = tutorialNode->FirstChildElement("hint"); hintNode; hintNode=hintNode->NextSiblingElement("hint"))
		{
			string hintText = string(XMLHelper::Exchange_String(hintNode, "text", NULL, false));
			tutorial.texts.push_back(hintText);
		}

		tutorials.push_back(tutorial);
	}

	this->moveBtnRevealed = false;
	this->rallyBtnRevealed = false;
	this->buildBtnRevealed = false;
	this->corePanelRevealed = false;
}

void TutorialsHandler::showTutorialGroup1()
{
	if (lastTutorialGroupDisplayed == 0)
	{
		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("intro1");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		hkvVec2 tutorialMessage1Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage1->init(tutorialMessage1Position, tuto1, 70.0f * scale);

		Tutorial* tuto2 = getTutorialForTag("intro2");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef();
		hkvVec2 tutorialMessage2Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage2->init(tutorialMessage2Position, tuto2, 70.0f * scale);

		Tutorial* tuto3 = getTutorialForTag("intro3");
		TutorialMessage* tutorialMessage3 = new TutorialMessage();
		tutorialMessage3->AddRef();
		hkvVec2 tutorialMessage3Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage3->init(tutorialMessage3Position, tuto3, 70.0f * scale);

		Tutorial* tuto4 = getTutorialForTag("intro4");
		TutorialMessage* tutorialMessage4 = new TutorialMessage();
		tutorialMessage4->AddRef();
		hkvVec2 tutorialMessage4Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage4->init(tutorialMessage4Position, tuto4, 70.0f * scale);

		Tutorial* tuto5 = getTutorialForTag("intro5");
		TutorialMessage* tutorialMessage5 = new TutorialMessage();
		tutorialMessage5->AddRef();
		hkvVec2 tutorialMessage5Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage5->init(tutorialMessage5Position, tuto5, 70.0f * scale);

		Tutorial* tuto6 = getTutorialForTag("intro6");
		TutorialMessage* tutorialMessage6 = new TutorialMessage();
		tutorialMessage6->AddRef();
		hkvVec2 tutorialMessage6Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage6->init(tutorialMessage2Position, tuto6, 70.0f * scale);

		Tutorial* tuto7 = getTutorialForTag("intro7");
		TutorialMessage* tutorialMessage7 = new TutorialMessage();
		tutorialMessage7->AddRef();
		hkvVec2 tutorialMessage7Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage7->init(tutorialMessage2Position, tuto7, 70.0f * scale);

		Tutorial* tuto8 = getTutorialForTag("tap_sphere");
		TutorialMessage* tutorialMessage8 = new TutorialMessage();
		tutorialMessage8->AddRef();
		hkvVec2 tutorialMessage8Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage8->init(tutorialMessage2Position, tuto8, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage8);
		stackedTutorialsGroups.push(tutorialMessage7);
		stackedTutorialsGroups.push(tutorialMessage6);
		stackedTutorialsGroups.push(tutorialMessage5);
		stackedTutorialsGroups.push(tutorialMessage4);
		stackedTutorialsGroups.push(tutorialMessage3);
		stackedTutorialsGroups.push(tutorialMessage2);

		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;

		sphereCountInsideCameraBoundaries = 1;
		GameManager::GlobalManager().getMainCamera()->updateCameraBoundaries();
	}
}

void TutorialsHandler::showTutorialGroup2()
{
	if (lastTutorialGroupDisplayed == 1)
	{
		GameManager::GlobalManager().lockInputEvents();

		if (this->currentDisplayedTutorialMessage)
		{
			this->currentDisplayedTutorialMessage->SetPosition(hkvVec2(0, - 1500.0f)); //out of screen
			this->currentDisplayedTutorialMessage->setEvacuated(true);
		}

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("good");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		hkvVec2 tutorialMessage1Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage1->init(tutorialMessage1Position, tuto1, 70.0f * scale);


		Tutorial* tuto2 = getTutorialForTag("sphere_info_panel1");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef();
		hkvVec2 tutorialMessage2Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage2->init(tutorialMessage2Position, tuto2, 70.0f * scale);

		Tutorial* tuto3 = getTutorialForTag("sphere_info_panel2");
		TutorialMessage* tutorialMessage3 = new TutorialMessage();
		tutorialMessage3->AddRef();
		hkvVec2 tutorialMessage3Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage3->init(tutorialMessage2Position, tuto3, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage3);
		stackedTutorialsGroups.push(tutorialMessage2);
		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;
	}
}

void TutorialsHandler::showTutorialGroup3()
{
	if (lastTutorialGroupDisplayed == 2)
	{
		this->moveBtnRevealed = true;
		ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		if (actionPanel)
		{
			ActionButton* moveBtn = (ActionButton*) actionPanel->findElementById(ID_MOVE_BTN);
			moveBtn->show();
		}

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("intro_group3");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		hkvVec2 tutorialMessage1Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage1->init(tutorialMessage1Position, tuto1, 70.0f * scale);

		Tutorial* tuto2 = getTutorialForTag("move_orbs");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef();
		hkvVec2 tutorialMessage2Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale); 
		tutorialMessage2->init(tutorialMessage2Position, tuto2, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage2);
		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;

		sphereCountInsideCameraBoundaries = 2;
		GameManager::GlobalManager().getMainCamera()->updateCameraBoundaries();

		//move the camera automatically between the two spheres
		GameManager::GlobalManager().getMainCamera()->moveToPositionForLookAtAndAltitude(hkvVec3(575, -325, 400), 1200, 90);
	}
}

void TutorialsHandler::showTutorialGroup4()
{
	if (lastTutorialGroupDisplayed == 3)
	{
		if (this->currentDisplayedTutorialMessage)
		{
			this->currentDisplayedTutorialMessage->SetPosition(hkvVec2(0, - 1500.0f)); //out of screen
			this->currentDisplayedTutorialMessage->setEvacuated(true);
		}

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("move_orbs_conclusion");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		tutorialMessage1->init(0.5f * screenSize, tuto1, 70.0f * scale);

		Tutorial* tuto2 = getTutorialForTag("rally_point1");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef(); 
		hkvVec2 tutorialMessage2Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 50.0f * scale); 
		tutorialMessage2->init(tutorialMessage2Position, tuto2, 70.0f * scale);

		Tutorial* tuto3 = getTutorialForTag("rally_point2");
		TutorialMessage* tutorialMessage3 = new TutorialMessage();
		tutorialMessage3->AddRef(); 
		hkvVec2 tutorialMessage3Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 50.0f * scale); 
		tutorialMessage3->init(tutorialMessage3Position, tuto3, 70.0f * scale);

		Tutorial* tuto4 = getTutorialForTag("rally_point_action");
		TutorialMessage* tutorialMessage4 = new TutorialMessage();
		tutorialMessage4->AddRef();
		tutorialMessage4->init(0.5f * screenSize, tuto4, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage4);
		stackedTutorialsGroups.push(tutorialMessage3);
		stackedTutorialsGroups.push(tutorialMessage2);
		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;
	}
}

void TutorialsHandler::showTutorialGroup5()
{
	if (lastTutorialGroupDisplayed == 4)
	{
		if (this->currentDisplayedTutorialMessage)
		{
			this->currentDisplayedTutorialMessage->SetPosition(hkvVec2(0, - 1500.0f)); //out of screen
			this->currentDisplayedTutorialMessage->setEvacuated(true);
		}

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("intro_group5_1");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		tutorialMessage1->init(0.5f * screenSize, tuto1, 70.0f * scale);

		Tutorial* tuto2 = getTutorialForTag("intro_group5_2");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef();
		tutorialMessage2->init(0.5f * screenSize, tuto2, 70.0f * scale);

		Tutorial* tuto3 = getTutorialForTag("intro_group5_3");
		TutorialMessage* tutorialMessage3 = new TutorialMessage();
		tutorialMessage3->AddRef();
		tutorialMessage3->init(0.5f * screenSize, tuto3, 70.0f * scale);

		Tutorial* tuto4 = getTutorialForTag("meta_orb_description1");
		TutorialMessage* tutorialMessage4 = new TutorialMessage();
		tutorialMessage4->AddRef();
		tutorialMessage4->init(0.5f * screenSize, tuto4, 70.0f * scale);

		Tutorial* tuto5 = getTutorialForTag("meta_orb_description2");
		TutorialMessage* tutorialMessage5 = new TutorialMessage();
		tutorialMessage5->AddRef();
		tutorialMessage5->init(0.5f * screenSize, tuto5, 70.0f * scale);

		Tutorial* tuto6 = getTutorialForTag("meta_orb_creation1");
		TutorialMessage* tutorialMessage6 = new TutorialMessage();
		tutorialMessage6->AddRef();
		tutorialMessage6->init(0.5f * screenSize, tuto6, 70.0f * scale);

		Tutorial* tuto7 = getTutorialForTag("meta_orb_creation2");
		TutorialMessage* tutorialMessage7 = new TutorialMessage();
		tutorialMessage7->AddRef();
		tutorialMessage7->init(0.5f * screenSize, tuto7, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage7);
		stackedTutorialsGroups.push(tutorialMessage6);
		stackedTutorialsGroups.push(tutorialMessage5);
		stackedTutorialsGroups.push(tutorialMessage4);
		stackedTutorialsGroups.push(tutorialMessage3);
		stackedTutorialsGroups.push(tutorialMessage2);
		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;
	}
}

void TutorialsHandler::showTutorialGroup6()
{
	if (lastTutorialGroupDisplayed == 5)
	{
		this->buildBtnRevealed = true;
		ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
		if (actionPanel)
		{
			ActionButton* buildBtn = (ActionButton*) actionPanel->findElementById(ID_BUILD_BTN);
			buildBtn->show();
		}

		hkvVec2 screenSize = GameDialogManager::sharedInstance()->getScreenSize();
		float scale = GameDialogManager::sharedInstance()->getScale();

		Tutorial* tuto1 = getTutorialForTag("intro_group6_1");
		TutorialMessage* tutorialMessage1 = new TutorialMessage();
		tutorialMessage1->AddRef();
		tutorialMessage1->init(0.5f * screenSize, tuto1, 70.0f * scale);

		Tutorial* tuto2 = getTutorialForTag("intro_group6_2");
		TutorialMessage* tutorialMessage2 = new TutorialMessage();
		tutorialMessage2->AddRef();
		hkvVec2 tutorial2Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale);
		tutorialMessage2->init(tutorial2Position, tuto2, 70.0f * scale);

		Tutorial* tuto3 = getTutorialForTag("isolated_spheres");
		TutorialMessage* tutorialMessage3 = new TutorialMessage();
		tutorialMessage3->AddRef();
		hkvVec2 tutorial3Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale);
		tutorialMessage3->init(tutorial3Position, tuto3, 70.0f * scale);

		Tutorial* tuto4 = getTutorialForTag("bridges");
		TutorialMessage* tutorialMessage4 = new TutorialMessage();
		tutorialMessage4->AddRef();
		hkvVec2 tutorial4Position = hkvVec2(0.5f * screenSize.x, 0.5f * screenSize.y + 100.0f * scale);
		tutorialMessage4->init(tutorial3Position, tuto4, 70.0f * scale);

		stackedTutorialsGroups.push(tutorialMessage4);
		stackedTutorialsGroups.push(tutorialMessage3);
		stackedTutorialsGroups.push(tutorialMessage2);
		tutorialMessage1->show();
		this->currentDisplayedTutorialMessage = tutorialMessage1;
		lastTutorialGroupDisplayed++;

		sphereCountInsideCameraBoundaries = 3;
		GameManager::GlobalManager().getMainCamera()->updateCameraBoundaries();
	}
}

void TutorialsHandler::onFinishDisplayingTutorialMessage(VBaseObject* sender)
{
	TutorialMessage* senderMessage = (TutorialMessage*) sender;
	if (senderMessage->isEvacuated())
	{
		senderMessage->Release();
		return;
	}

	if (stackedTutorialsGroups.size() != 0)
	{
		TutorialMessage* newMessage = stackedTutorialsGroups.top();
		newMessage->show();
		this->currentDisplayedTutorialMessage = newMessage;
		stackedTutorialsGroups.pop();

		if (!newMessage->getTutorial()->tag.compare("tap_sphere"))
			GameManager::GlobalManager().unlockInputEvents();
		else if (!newMessage->getTutorial()->tag.compare("move_orbs"))
		{
			LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[0]->setHidden(false);
			GameManager::GlobalManager().unlockInputEvents();
			this->moveBtnRevealed = true;
			ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
			if (actionPanel)
			{
				float scale = GameDialogManager::sharedInstance()->getScale();
				GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->showMoveBtn(hkvVec2(-78.0f * scale, -180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
		}
		else if (!newMessage->getTutorial()->tag.compare("rally_point_action"))
		{
			GameManager::GlobalManager().unlockInputEvents();
			this->rallyBtnRevealed = true;
			ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
			if (actionPanel)
			{
				float scale = GameDialogManager::sharedInstance()->getScale();
				GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->showRallyBtn(hkvVec2(-78.0f * scale, 0.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}
		}
		else if (!newMessage->getTutorial()->tag.compare("gesture_slide"))
		{
			GameManager::GlobalManager().unlockInputEvents();
			GameManager::GlobalManager().getMainCamera()->unblockCamera();
		}
		else if (!newMessage->getTutorial()->tag.compare("isolated_spheres"))
		{
			GameManager::GlobalManager().getMainCamera()->moveToPositionForLookAtAndAltitude(hkvVec3(1150,-200,400), 1800, 114);
		}
	}
	else
	{
		if (!this->currentDisplayedTutorialMessage->getTutorial()->tag.compare("sphere_info_panel2")) //end of group 2, start group 3
		{
			CallFuncObject* startGroup3Action = new CallFuncObject(this, (CallFunc)(&TutorialsHandler::showTutorialGroup3), 2.0f);
			CallFuncHandler::sharedInstance().addCallFuncInstance(startGroup3Action);
			startGroup3Action->executeFunc();

		}
		else if (!this->currentDisplayedTutorialMessage->getTutorial()->tag.compare("upgrade_generator_cost"))
		{
			LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[1]->setHidden(false);
			LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[2]->setHidden(false);
		}
		else if (!this->currentDisplayedTutorialMessage->getTutorial()->tag.compare("bridges"))
		{
			LevelManager::sharedInstance().getCurrentLevel()->getObjectives()[3]->setHidden(false);
			this->buildBtnRevealed = true;
			ActionPanel* actionPanel = GameDialogManager::sharedInstance()->getHUD()->getActionPanel();
			if (actionPanel)
			{
				float scale = GameDialogManager::sharedInstance()->getScale();
				GameDialogManager::sharedInstance()->getHUD()->getActionPanel()->showBuildBtn(hkvVec2(-78.0f * scale, 180.0f * scale), hkvVec2(1.0f, 1.0f), true, 0.2f, 0.0f);
			}

			GameManager::GlobalManager().unlockInputEvents();

			MiscPanel* pMiscPanel = GameDialogManager::sharedInstance()->getHUD()->getMiscPanel();
			pMiscPanel->buildAndStackNewObjectivesAlert();
		}
		else if (!this->currentDisplayedTutorialMessage->getTutorial()->tag.compare("meta_orb_creation2"))	
		{
			GameManager::GlobalManager().unlockInputEvents();

			NodeSphere* nodeSphere2 = GameManager::GlobalManager().findNodeSphereBy3DPosition(hkvVec3(1150,0,400));
			nodeSphere2->pick();
			GameManager::GlobalManager().OnPickSphere();

			this->corePanelRevealed = true;
			GameDialogManager::sharedInstance()->getHUD()->showCorePanel();

			MiscPanel* pMiscPanel = GameDialogManager::sharedInstance()->getHUD()->getMiscPanel();
			pMiscPanel->buildAndStackNewObjectivesAlert();
		}	
		
		this->currentDisplayedTutorialMessage = NULL;
	}

	senderMessage->Release();
}

Tutorial* TutorialsHandler::getTutorialForTag(string tag)
{
	for (int tutorialIndex = 0; tutorialIndex != tutorials.size(); tutorialIndex++)
	{
		if (!tutorials[tutorialIndex].tag.compare(tag))
			return &tutorials[tutorialIndex];
	}

	return NULL;
}

////////////////////////TutorialMessage////////////////////////
TutorialMessage::~TutorialMessage()
{
	for (int hintIndex = 0; hintIndex != hintsMessages.size(); hintIndex++)
	{
		hintsMessages[hintIndex]->removeSelf();
	}
}

void TutorialMessage::init(hkvVec2 position, Tutorial* tutorial, float lineOffset)
{
	float scale = GameDialogManager::sharedInstance()->getScale();

	ScreenMaskPanel::init(position);
	this->tutorial = tutorial;

	vector<string> hintTexts = tutorial->texts;
	int hintsCount = (int) hintTexts.size();
	for (int hintIndex = 0; hintIndex != hintTexts.size(); hintIndex++)
	{
		string hintText = hintTexts[hintIndex];

		HUDMessage* hintMessage = new HUDMessage();
		hintMessage->AddRef();
		float relativeIndex;
		int halfCount = hintsCount / 2;
		if (hintsCount % 2 == 0) //even
		{
			relativeIndex = (hintIndex < halfCount) ? hintIndex - halfCount + 0.5f : hintIndex - halfCount + 1 - 0.5f;
		}
		else //odd
			relativeIndex = (float) (hintIndex - halfCount);
		
		hkvVec2 hintPosition = hkvVec2(0, relativeIndex * lineOffset);

		hintMessage->init(hintPosition, hintText);
		hintMessage->getTextLabel()->setRenderedBehindScreenMasks(true);
		this->addPanel(hintMessage);
		hintMessage->SetOpacity(0.0f);
	
		hintsMessages.push_back(hintMessage);
	}
}

void TutorialMessage::show()
{
	for (int hintMessageIndex = 0; hintMessageIndex != hintsMessages.size(); hintMessageIndex++)
	{
		HUDMessage* hint = hintsMessages[hintMessageIndex];

		CallFuncObject* showHintAction = new CallFuncObject(hint, (CallFunc)(&HUDMessage::fadeIn), hintMessageIndex * tutorial->intervalTime);
		CallFuncHandler::sharedInstance().addCallFuncInstance(showHintAction);
		showHintAction->executeFunc();
	}

	CallFuncObject* dismissTutorialMessageAction = new CallFuncObject(this, (CallFunc)(&TutorialMessage::dismiss), tutorial->displayTime);
	CallFuncHandler::sharedInstance().addCallFuncInstance(dismissTutorialMessageAction);
	dismissTutorialMessageAction->executeFunc();
}

void TutorialMessage::dismiss()
{
	this->fadeTo(0.0f, 1.0f, 0.0f);
	
	CallFuncSenderObject* callbackAction = new CallFuncSenderObject(&TutorialsHandler::sharedInstance(), (CallFuncSender)(&TutorialsHandler::onFinishDisplayingTutorialMessage), this, 1.0f);
	CallFuncHandler::sharedInstance().addCallFuncSenderInstance(callbackAction);
	callbackAction->executeFunc();
}

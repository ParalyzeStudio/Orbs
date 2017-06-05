#include "OrbsPluginPCH.h"
#include "GameTimer.h"

GameTimer GameTimer::instance;

void GameTimer::OneTimeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

void GameTimer::OneTimeDeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void GameTimer::start()
{
	this->running = true;
	this->gameTime = 0;
	this->timeScaleFactor = 1.0f;
}

void GameTimer::getTimeHoursMinsSecs(int hoursMinsSecs[3])
{
	int gameTimeSecs = GeometryUtils::round(this->gameTime);

	int hours = gameTimeSecs / 3600;
	gameTimeSecs = gameTimeSecs % 3600;
	int mins = gameTimeSecs / 60;
	gameTimeSecs = gameTimeSecs % 60;
	
	hoursMinsSecs[0] = hours;
	hoursMinsSecs[1] = mins;
	hoursMinsSecs[2] = gameTimeSecs;
}

int GameTimer::getTimeAsSecs()
{
	return GeometryUtils::round(this->gameTime);
}

string GameTimer::getTimeAsString()
{
	int hoursMinsSecs[3];
	this->getTimeHoursMinsSecs(hoursMinsSecs);
	string hoursStr = (hoursMinsSecs[0] < 10) ? stringFromInt(0) + stringFromInt(hoursMinsSecs[0]) : stringFromInt(hoursMinsSecs[0]);
	string minsStr = (hoursMinsSecs[1] < 10) ? stringFromInt(0) + stringFromInt(hoursMinsSecs[1]) : stringFromInt(hoursMinsSecs[1]);
	string secsStr = (hoursMinsSecs[2] < 10) ? stringFromInt(0) + stringFromInt(hoursMinsSecs[2]) : stringFromInt(hoursMinsSecs[2]);

	string timeText;
	if (hoursMinsSecs[0] > 0)
		timeText = hoursStr + string(":") + minsStr + string(":") + secsStr;
	else
		timeText = minsStr + string(":") + secsStr;

	return timeText;
}

void GameTimer::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
		this->update(dt);
	}
}
	
void GameTimer::update(float dt)
{
	if (this->running)
		this->gameTime += dt;
}

float GameTimer::getTimeScaleFactor(bool withPause)
{
	if (withPause && this->isPaused())
		return 0;
	return timeScaleFactor;
}

///////////////////////////CountdownTimer///////////////////////////
CountdownTimer::CountdownTimer() : m_bActive(false), m_fTime(0)
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

CountdownTimer::~CountdownTimer()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}


void CountdownTimer::init(float fStartTime)
{
	this->m_fTime = fStartTime;
}

void CountdownTimer::start()
{
	this->m_bActive = true;
}

void CountdownTimer::updateTime(float dt)
{
	if (this->m_bActive)
		this->m_fTime -= dt;

	if (this->m_fTime <= 0) //timer has reached the end
		this->m_bActive = false;
}

void CountdownTimer::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		float dt = Vision::GetTimer()->GetTimeDifference() * GameTimer::sharedInstance().getTimeScaleFactor(true);
		this->updateTime(dt);
	}
}

string CountdownTimer::getRemainingTimeAsString()
{
	int iRemainingTimeInSecs = GeometryUtils::round(this->m_fTime);

	int iHours = iRemainingTimeInSecs / 3600;
	iRemainingTimeInSecs = iRemainingTimeInSecs % 3600;
	int iMins = iRemainingTimeInSecs / 60;
	int iSecs = iRemainingTimeInSecs % 60;

	string strHours = (iHours < 10) ? stringFromInt(0) + stringFromInt(iHours) : stringFromInt(iHours);
	string strMins = (iMins < 10) ? stringFromInt(0) + stringFromInt(iMins) : stringFromInt(iMins);
	string strSecs = (iSecs < 10) ? stringFromInt(0) + stringFromInt(iSecs) : stringFromInt(iSecs);

	string strTimeText;
	if (iHours > 0)
		strTimeText = strHours + string(":") + strMins + string(":") + strSecs;
	else
		strTimeText = strMins + string(":") + strSecs;

	return strTimeText;
}
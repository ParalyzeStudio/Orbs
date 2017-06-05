#pragma once

#define TIME_SCALE_AMPLITUDE 2.0f //from 1x to 3x time

class GameTimer : public IVisCallbackHandler_cl
{
public:
	static GameTimer& sharedInstance(){return instance;};

	void OneTimeInit();
	void OneTimeDeInit();

	void start();
	float getExactTime(){return this->gameTime;};
	void getTimeHoursMinsSecs(int hoursMinsSecs[3]);
	int getTimeAsSecs();
	string getTimeAsString();

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	virtual void update(float dt);

	float getTimeScaleFactor(bool withPause);
	void setTimeScaleFactor(float timeScaleFactor){this->timeScaleFactor = timeScaleFactor;};
	bool isPaused(){return !this->running;};
	void setPaused(bool pause){this->running = !pause;};

private:
	GameTimer() : gameTime(0), running(false), timeScaleFactor(0){};

	static GameTimer instance;

	float gameTime;
	bool running;
	float timeScaleFactor;
};

class CountdownTimer : public VRefCounter, public IVisCallbackHandler_cl
{
public:
	CountdownTimer();
	~CountdownTimer();

	void init(float fStartTime);
	void start();
	void updateTime(float dt);
	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	float getRemainingTime(){return this->m_fTime;};
	string getRemainingTimeAsString();
	bool isActive(){return this->m_bActive;};

private:
	bool m_bActive;
	float m_fTime;
};
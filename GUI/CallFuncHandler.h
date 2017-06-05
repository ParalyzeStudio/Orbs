#pragma once

class CallFuncObject;
class CallFuncSenderObject;

typedef void (VBaseObject::*CallFunc) ();
typedef void (VBaseObject::*CallFuncSender) (VBaseObject*);


//Class that handles actions that needs to be delayed using function pointers
class CallFuncHandler : public IVisCallbackHandler_cl
{
public:
	~CallFuncHandler();
	static CallFuncHandler& sharedInstance() {return instance;};

	void OneTimeInit();
	void OneTimeDeInit();

	void addCallFuncInstance(CallFuncObject* callFunc);
	void removeCallFuncInstance(CallFuncObject* callFunc);
	void addCallFuncSenderInstance(CallFuncSenderObject* callFunc);
	void removeCallFuncSenderInstance(CallFuncSenderObject* callFunc);

	virtual void update(float dt);
	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

private:
	CallFuncHandler();
	static CallFuncHandler instance;

	vector<CallFuncObject*> callFuncInstances;
	vector<CallFuncSenderObject*> callFuncSenderInstances;
};

//Container for an action that needs to be delayed
class CallFuncObject : public VRefCounter
{
public:
	CallFuncObject(VBaseObject* target, CallFunc func, float delay) : target(target), func(func), timer(delay), executing(false){};

	void executeFunc(){this->executing = true;};  //execute a function after a certain delay

	bool isExecuting(){return this->executing;};
	void setExecuting(bool executing){this->executing = executing;};
	VBaseObject* getTarget(){return this->target;};
	CallFunc getFunc(){return this->func;};
	float decreaseTimer(float dt){this->timer -= dt; return this->timer;}; //return the new value of timer

private:
	bool executing;
	VBaseObject* target;
	CallFunc func;
	float timer;
};

class CallFuncSenderObject : public VRefCounter
{
public:
	CallFuncSenderObject(VBaseObject* target, CallFuncSender func, VBaseObject* sender, float delay) : target(target), func(func), timer(delay), sender(sender), executing(false){};

	void executeFunc(){this->executing = true;};  //execute a function after a certain delay

	bool isExecuting(){return this->executing;};
	void setExecuting(bool executing){this->executing = executing;};
	VBaseObject* getTarget(){return this->target;};
	CallFuncSender getFunc(){return this->func;};
	VBaseObject* getSender(){return this->sender;};
	float decreaseTimer(float dt){this->timer -= dt; return this->timer;}; //return the new value of timer

private:
	bool executing;
	VBaseObject* sender;
	VBaseObject* target;
	CallFuncSender func;
	float timer;
};
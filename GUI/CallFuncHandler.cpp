#include "OrbsPluginPCH.h"
#include "CallFuncHandler.h"

/////////////////////////////CallFuncHandler/////////////////////////////
CallFuncHandler CallFuncHandler::instance;

CallFuncHandler::CallFuncHandler()
{
	
}

CallFuncHandler::~CallFuncHandler()
{

}

void CallFuncHandler::OneTimeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin += this;
}

void CallFuncHandler::OneTimeDeInit()
{
	Vision::Callbacks.OnUpdateSceneBegin -= this;
}

void CallFuncHandler::addCallFuncInstance(CallFuncObject* callFunc)
{
	callFunc->AddRef();
	this->callFuncInstances.push_back(callFunc);
}

void CallFuncHandler::removeCallFuncInstance(CallFuncObject* callFunc)
{
	vector<CallFuncObject*>::iterator callFuncInstancesIt;
	for (callFuncInstancesIt = callFuncInstances.begin(); callFuncInstancesIt != callFuncInstances.end();callFuncInstancesIt++)
	{
		CallFuncObject* currentCallFunc = *callFuncInstancesIt;
		if (currentCallFunc == callFunc)
		{
			callFuncInstances.erase(callFuncInstancesIt);
			callFunc->Release();
			return;
		}
	}
}

void CallFuncHandler::addCallFuncSenderInstance(CallFuncSenderObject* callFunc)
{
	callFunc->AddRef();
	this->callFuncSenderInstances.push_back(callFunc);
}

void CallFuncHandler::removeCallFuncSenderInstance(CallFuncSenderObject* callFunc)
{
	vector<CallFuncSenderObject*>::iterator callFuncInstancesIt;
	for (callFuncInstancesIt = callFuncSenderInstances.begin(); callFuncInstancesIt != callFuncSenderInstances.end();callFuncInstancesIt++)
	{
		CallFuncSenderObject* currentCallFunc = *callFuncInstancesIt;
		if (currentCallFunc == callFunc)
		{
			callFuncSenderInstances.erase(callFuncInstancesIt);
			callFunc->Release();
			return;
		}
	}
}

void CallFuncHandler::update(float dt)
{
	int callFuncInstancesSize = callFuncInstances.size();
	for (int callFuncInstanceIndex = 0; callFuncInstanceIndex != callFuncInstancesSize; callFuncInstanceIndex++)
	{
		CallFuncObject* callFunc = callFuncInstances[callFuncInstanceIndex];
		if (callFunc->isExecuting())
		{
			float timer = callFunc->decreaseTimer(dt);
			if (timer < 0)
			{
				VBaseObject* target = callFunc->getTarget();
				CallFunc func = callFunc->getFunc();

				(target->*func)();

				callFunc->setExecuting(false);

				this->removeCallFuncInstance(callFunc);
				callFuncInstancesSize--;
				callFuncInstanceIndex--;
			}
		}
	}

	int callFuncSenderInstancesSize = callFuncSenderInstances.size();
	for (int callFuncSenderInstanceIndex = 0; callFuncSenderInstanceIndex != callFuncSenderInstancesSize; callFuncSenderInstanceIndex++)
	{
		CallFuncSenderObject* callFunc = callFuncSenderInstances[callFuncSenderInstanceIndex];
		if (callFunc->isExecuting())
		{
			float timer = callFunc->decreaseTimer(dt);
			if (timer < 0)
			{
				VBaseObject* target = callFunc->getTarget();
				CallFuncSender func = callFunc->getFunc();
				VBaseObject* sender = callFunc->getSender();

				(target->*func)(sender);

				callFunc->setExecuting(false);

				this->removeCallFuncSenderInstance(callFunc);
				callFuncSenderInstancesSize--;
				callFuncSenderInstanceIndex--;
			}
		}
	}
}

void CallFuncHandler::OnHandleCallback(IVisCallbackDataObject_cl *pData)
{
	if (pData->m_pSender == &Vision::Callbacks.OnUpdateSceneBegin)
	{
		if (GameDialogManager::sharedInstance()->isContextActive() && GameTimer::sharedInstance().isPaused()) //inside a level and timer is paused
			return;
		float dt = Vision::GetTimer()->GetTimeDifference();
		update(dt);
	}
}
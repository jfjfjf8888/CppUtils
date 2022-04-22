#include "ShellExcuteChain.h"
#include <QDebug>

CommandItem::CommandItem()
{
}


CommandItem::CommandItem(CommandItem & executor)
{
	mCommand = executor.getCommand();
	mOnSuccessCallback = executor.getOnSuccessCallback();
	mOnErrorCallback = executor.getOnErrorCallback();
	mOnBeforeCallback = executor.getBeforeExecuteCallback();
}

CommandItem::~CommandItem()
{
}

CommandItem & CommandItem::command(const QString cmd)
{
	mCommand = cmd;
	return *this;
}

CommandItem & CommandItem::onBefore(LifeCycleCallback cb)
{
	mOnBeforeCallback = cb;
	return *this;
}

CommandItem & CommandItem::onAfter(LifeCycleCallback cb)
{
	mOnAfterCallback = cb;
	return *this;
}

CommandItem & CommandItem::onSuccess(Callback cb)
{
	mOnSuccessCallback = cb;
	return *this;
}

CommandItem & CommandItem::onError(Callback cb)
{
	mOnErrorCallback = cb;
	return *this;
}

QString & CommandItem::getCommand()
{
	return mCommand;
}

const Callback & CommandItem::getOnSuccessCallback() const
{
	return mOnSuccessCallback;
}

const Callback & CommandItem::getOnErrorCallback() const
{
	return mOnErrorCallback;
}

const LifeCycleCallback &CommandItem::getBeforeExecuteCallback() const
{
	return mOnBeforeCallback;
}

const LifeCycleCallback &CommandItem::getAfterExecuteCallback() const
{
	return mOnAfterCallback;
}


ShellExcuteChain::ShellExcuteChain(int maxThreadCount)
{
	mCommandItems.clear();
	mThreadPool.setMaxThreadCount(maxThreadCount);
}


ShellExcuteChain::~ShellExcuteChain()
{
	stop();
	foreach(CommandItem * excutor, mCommandItems) {
		delete excutor;
	}
	mCommandItems.clear();
}

ShellExcuteChain * ShellExcuteChain::getInstance(int maxThreadCount)
{
	return new ShellExcuteChain(maxThreadCount);
}

ShellExcuteChain * ShellExcuteChain::appendCommandItem(CommandItem& executor)
{
	CommandItem * myExecutor = new CommandItem(executor);
	mCommandItems.push_back(myExecutor);
	return this;
}

QList<CommandItem*>& ShellExcuteChain::getCommandItems()
{
	return mCommandItems;
}

void ShellExcuteChain::start()
{
	if (!checkCommandItemsCallbacks()) {
		qDebug() << "Error: Some CommandItem OnSuccessCallback or OnErrorCallback, work not start, pls check!";
		return;
	}

	if (!mOnAllCompletedCallback) {
		qDebug() << "Wranning: OnAllCompletedCallback not set, you can call ShellExcuteChain->onAllCompleted to set it, your callback will be call when all job completed.";
	}
	mCancelWork = false;
	mLastResult.clear();
	mLastError.clear();

	mThreadPool.start(new ShellExcuteRunnable(this));
}

void ShellExcuteChain::stop()
{
	mCancelWork = true;
}

QString & ShellExcuteChain::getLastResult()
{
	return mLastResult;
}

QString & ShellExcuteChain::getLastError()
{
	return mLastError;
}

ShellExcuteChain * ShellExcuteChain::onAllCompleted(OnAllCompletedCallback cb)
{
	mOnAllCompletedCallback = cb;
	return this;
}

bool ShellExcuteChain::checkCommandItemsCallbacks()
{
	foreach(CommandItem * excutor, mCommandItems) {
		if (excutor->getOnErrorCallback() && excutor->getOnSuccessCallback()) continue;
		return false;
	}

	return true;
}

const OnAllCompletedCallback & ShellExcuteChain::getOnAllCompletedCallback() const
{
	return mOnAllCompletedCallback;
}

bool ShellExcuteChain::cancelWork()
{
	return mCancelWork;
}

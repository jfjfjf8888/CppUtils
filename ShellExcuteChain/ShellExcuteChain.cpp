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
    mBeforeExecuteCallback = executor.getBeforeExecuteCallback();
}

CommandItem::~CommandItem()
{
}

CommandItem & CommandItem::command(const QString cmd)
{
	mCommand = cmd;
    return *this;
}

CommandItem & CommandItem::onBefore(BeforeExecuteCallback cb)
{
    mBeforeExecuteCallback = cb;
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

const BeforeExecuteCallback &CommandItem::getBeforeExecuteCallback() const
{
    return mBeforeExecuteCallback;
}


ShellExcuteChain::ShellExcuteChain(int maxThreadCount, bool autoDelete)
{
	mExecutors.clear();
	mThreadPool.setMaxThreadCount(maxThreadCount);
	mAutoDetele = autoDelete;
	mCancelWork = false;
}

ShellExcuteChain::~ShellExcuteChain()
{
    foreach (CommandItem * excutor, mExecutors) {
		delete excutor;
	}
	mExecutors.clear();
}

ShellExcuteChain * ShellExcuteChain::getInstance(int maxThreadCount, bool autoDelete)
{
	return new ShellExcuteChain(maxThreadCount, autoDelete);
}

ShellExcuteChain * ShellExcuteChain::appendCommandItem(CommandItem& executor)
{
	CommandItem * myExecutor = new CommandItem(executor);
	mExecutors.push_back(myExecutor);
	return this;
}

QList<CommandItem*>& ShellExcuteChain::getExcutors()
{
	return mExecutors;
}

void ShellExcuteChain::start()
{
	if (!checkCommandItemsCallbacks()) {
		qDebug() << "Error: Some CommandItem OnSuccessCallback or OnErrorCallback, work not start, pls check!";
		return;
	}

	if (!mAutoDetele && !mOnAllCompletedCallback) {
		qDebug() << "Wranning: OnAllCompletedCallback not set, you can call ShellExcuteChain->onAllCompleted to set it, your callback will be call when all job completed.";
	}


	mCancelWork = false;
	mLastResult.clear();
	mLastError.clear();

	ShellExcuteRunnable * runnable = new ShellExcuteRunnable(mExecutors[0], 0);
    runnable->OnJobCompleted([&, runnable](int idx, bool continuable, const QString lastResult, const QString lastError) {
		mCancelWork = !continuable;
		mLastResult = lastResult;
		mLastError = lastError;

		if (mCancelWork) {
			if (mOnAllCompletedCallback) {
				mOnAllCompletedCallback();
			}
			if (mAutoDetele) delete this;
			return;
		}

		CommandItem * next = nextShellItem(idx);
		if (next) {
            ShellExcuteRunnable * newRunnable = new ShellExcuteRunnable(next, idx + 1);
            newRunnable->OnJobCompleted(runnable->getCallback());
            mThreadPool.start(newRunnable);
        }
		else {
			if (mOnAllCompletedCallback) {
				mOnAllCompletedCallback();
			}	
			if (mAutoDetele) delete this;
		}
	});

	mThreadPool.start(runnable);
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

CommandItem * ShellExcuteChain::nextShellItem(int oldIndex)
{
	if (oldIndex < mExecutors.count() - 1) {
		return mExecutors[oldIndex + 1];
	}

	return nullptr;
}

bool ShellExcuteChain::checkCommandItemsCallbacks()
{
    foreach (CommandItem * excutor, mExecutors) {
		if (excutor->getOnErrorCallback() && excutor->getOnSuccessCallback()) continue;

		return false;
	}

    return true;
}

ShellExcuteChain * ShellExcuteChain::setAutoDetele(bool newAutoDetele)
{
    mAutoDetele = newAutoDetele;
    return this;
}

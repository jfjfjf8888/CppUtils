#pragma once

#include <QString>
#include <QList>
#include <QThreadPool>
#include <QRunnable>
#include <QProcess>

class CommandItem;
class ShellExcuteChain;
class ShellExcuteRunnable;

/**
* @brief OnAllCompletedCallback @ShellExcuteChain
*/
typedef std::function<void()> OnAllCompletedCallback;

/**
* @brief Callback @CommandItem OnSuccess and onError callback
*/
typedef std::function<bool(CommandItem * thisItem, QString&)> Callback;

/**
* @brief LifeCycleCallback  @CommandItem
*/
typedef std::function<void(CommandItem * thisItem)> LifeCycleCallback;


/**
* @brief CommandItem 类是 @ShellExcuteChain 类中的执行节点
*          CommandItem 可以设置 命令行、被执行前回调、成功回调、失败回调
*          其中，成功回调和失败回调是必须要设置的，否则不会运行。
*          所有回调函数都支持lambda表达式。
*  @author xn
*  @note Email: xn19900207@gmail.com
*/
class CommandItem
{
public:
	/**
	* @brief CommandItem 默认无参构造函数
	*/
	CommandItem();

	/**
	* @brief CommandItem 拷贝构造函数
	* @param executor
	*/
	CommandItem(CommandItem& executor);


	~CommandItem();

	/**
	* @brief command 设置命令行
	* @param cmd 命令行
	* @return 当前对象的引用
	*/
	CommandItem & command(const QString cmd);

	/**
	* @brief onBefore 设置命令行被执行前的回调，该回调不是必须的，
	*                 此函数在链式执行多个CommandItem时非常有用，
	*                 可以将上一个CommandItem的执行结果应用到本次
	*                 执行当中，上一次的执行结果参照@ShellExcuteChain
	*                 中的@getLastResult 函数
	* @param cb       callback @see LifeCycleCallback
	* @return         当前对象引用
	*
	* @warning 设置的回调会在线程中被调用，不要在回调中操作界面!!!可以发射信号
	*/
	CommandItem & onBefore(LifeCycleCallback cb);

	/**
	* @brief onAfter  设置命令行被执行后的回调，该回调不是必须的，
	* @param cb       callback @see LifeCycleCallback
	* @return         当前对象引用
	*
	* @warning 设置的回调会在线程中被调用，不要在回调中操作界面!!!可以发射信号
	*/
	CommandItem & onAfter(LifeCycleCallback cb);

	/**
	* @brief onSuccess 设置命令行执行成功回调，可以在成功回调里修改本次执行结果，
	*                  修改后的结果将保存到@ShellExcuteChain上，下一个CommandItem
	*                  可以通过@ShellExcuteChain拿到本次执行的结果
	* @param cb callback @see Callback
	* @return   当前对象引用
	*
	* @warning 设置的回调会在线程中被调用，不要在回调中操作界面!!!可以发射信号
	*/
	CommandItem & onSuccess(Callback cb);

	/**
	* @brief onError 设置执行失败回调
	* @param cb      callback @see Callback
	* @return        当前对象引用
	*
	* @warning 设置的回调会在线程中被调用，不要在回调中操作界面!!!可以发射信号
	*/
	CommandItem & onError(Callback cb);

	/**
	* @brief getCommand 获取command的引用，可以通过该引用修改 cmd
	* @return cmd引用
	*/
	QString & getCommand();

	/**
	* @brief getOnSuccessCallback
	* @return OnSuccessCallback， @see Callback
	*/
	const Callback & getOnSuccessCallback() const;

	/**
	* @brief getOnErrorCallback
	* @return OnErrorCallback， @see Callback
	*/
	const Callback & getOnErrorCallback() const;

	/**
	* @brief getBeforeExecuteCallback
	* @return OnBeforeCallback， @see LifeCycleCallback
	*/
	const LifeCycleCallback & getBeforeExecuteCallback() const;


	/**
	* @brief getAfterExecuteCallback
	* @return OnAfterCallback， @see LifeCycleCallback
	*/
	const LifeCycleCallback & getAfterExecuteCallback() const;
private:
	QString mCommand;
	Callback mOnSuccessCallback;
	Callback mOnErrorCallback;
	LifeCycleCallback mOnBeforeCallback;
	LifeCycleCallback mOnAfterCallback;
};


/**
* @brief The ShellExcuteChain class
*        Shell命令执行链，内部维护了一个 @QThreadPool, 将链上的每一个 @CommandItem 按照append顺序在ThreadPool执行
*/
class ShellExcuteChain
{
public:
	~ShellExcuteChain();
	/**
	* @brief getInstance
	* @param maxThreadCount 内部线程池同时执行的线程数量，默认1，自己最好不要动
	* @param autoDelete     是否在任务全部完成后自动删除自己，默认false，可以在 @onAllCompleted 完成一些其它操作后删除chain
	*                       如果任务完成后不需要做别的动作，则可以设置autoDelete为true
	* @return
	*/
	static ShellExcuteChain * getInstance(int maxThreadCount = 10);

	/**
	* @brief appendCommandItem 添加 @CommandItem 到链
	* @param executor
	* @return this
	*/
	ShellExcuteChain * appendCommandItem(CommandItem & executor);

	/**
	* @brief getCommandItems 获取所有添加的 @CommandItem 列表
	* @return
	*/
	QList<CommandItem *> & getCommandItems();

	/**
	* @brief start 开始执行
	*/
	void start();

	/**
	* @brief stop 停止执行
	*/
	void stop();

	/**
	* @brief getLastResult
	* @return 最后一次执行结果
	*/
	QString & getLastResult();

	/**
	* @brief getLastError
	* @return 最后一次执行错误信息
	*/
	QString & getLastError();

	/**
	* @brief onAllCompleted 设置 全部完成 回调 @OnAllCompletedCallback
	* @param cb
	* @return this
	*/
	ShellExcuteChain * onAllCompleted(OnAllCompletedCallback cb);

	/**
	* @brief getOnAllCompletedCallback
	* @return @OnAllCompletedCallback
	*/
	const OnAllCompletedCallback & getOnAllCompletedCallback() const;

	/**
	* @brief cancelWork
	* @return 
	*/
	bool cancelWork();

private:
	/**
	* @brief ShellExcuteChain 私有构造函数
	* @param maxThreadCount
	* @param autoDelete
	*/
	ShellExcuteChain(int maxThreadCount);

	/**
	* @brief checkCommandItemsCallbacks 函数会遍历 @mCommandItems, 并检查内部所有 @CommandItem 是否设置了OnSuccess 和 OnError回调
	*        回调原型参照 @Callback
	* @return 是否所有 @CommandItem 都设置了OnSuccess 和 OnError回调
	*/
	bool checkCommandItemsCallbacks();
private:
	OnAllCompletedCallback mOnAllCompletedCallback;
	QList<CommandItem *> mCommandItems;
	QThreadPool mThreadPool;
	QString mLastResult;
	QString mLastError;
	bool mCancelWork;
};

/**
* @brief The ShellExcuteRunnable class 执行shell的Runnable， 继承自 @QRunnable 为了方便的丢进 @QThreadPool 执行
*
* @inherits QRunnable
*/
class ShellExcuteRunnable : public QRunnable {
public:
	/**
	* @brief ShellExcuteRunnable
	* @param chain   chain @ShellExcuteChain
	* @param items   要执行的 @CommandItem list
	*/
	ShellExcuteRunnable(ShellExcuteChain * chain) {
		mChain = chain;
		setAutoDelete(true);
	}

	void run() {
		foreach (CommandItem * item, mChain->getCommandItems()){
			if (mChain->cancelWork()) break;

			// 检查是否设置了onBefore (原型参考 @LifeCycleCallback)回调，如果已设置则调用
			if (item->getBeforeExecuteCallback()) item->getBeforeExecuteCallback()(item);

			QProcess process;
			// Qt 7.0.0
			//process.startCommand(mCommandItem->getCommand());

			// Qt 5.8
			process.start(item->getCommand());

			process.waitForFinished();

			mChain->getLastResult() = process.readAllStandardOutput();
			mChain->getLastError() = process.readAllStandardError();

			bool continuable = true;

			// 根据命令行执行结果调用OnSuccess 活着OnError
			if (!mChain->getLastError().isEmpty())
				continuable = item->getOnErrorCallback()(item, mChain->getLastError());
			else
				continuable = item->getOnSuccessCallback()(item, mChain->getLastResult());

			if (item->getAfterExecuteCallback()) item->getAfterExecuteCallback()(item);

			if (!continuable) break;
		}

		// 检查是否设置了 getOnAllCompletedCallback (原型参考 @OnAllCompletedCallback)回调, 如果已设置则调用
		if (mChain->getOnAllCompletedCallback()) mChain->getOnAllCompletedCallback()();
	}

private:
	ShellExcuteChain * mChain;
};

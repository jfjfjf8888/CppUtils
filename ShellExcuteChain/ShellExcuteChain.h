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
 * @brief OnJobCompletedCallback @ShellExcuteChain
 */
typedef std::function<void (int, bool, const QString, const QString)> OnJobCompletedCallback;

/**
 * @brief OnAllCompletedCallback @ShellExcuteChain
 */
typedef std::function<void()> OnAllCompletedCallback;

/**
 * @brief Callback @CommandItem OnSuccess and onError callback
 */
typedef std::function<bool(CommandItem * thisItem, QString&)> Callback;

/**
 * @brief BeforeExecuteCallback  @CommandItem
 */
typedef std::function<void(CommandItem * thisItem)> BeforeExecuteCallback;


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
     * @param cb       callback @see BeforeExecuteCallback
     * @return         当前对象引用
     *
     * @warning 设置的回调会在线程中被调用，不要在回调中操作界面!!!可以发射信号
     */
    CommandItem & onBefore(BeforeExecuteCallback cb);

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
     * @return BeforeExecuteCallback， @see BeforeExecuteCallback
     */
    const BeforeExecuteCallback & getBeforeExecuteCallback() const;

private:
    QString mCommand;
    Callback mOnSuccessCallback;
    Callback mOnErrorCallback;
    BeforeExecuteCallback mBeforeExecuteCallback;
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
    static ShellExcuteChain * getInstance(int maxThreadCount = 1, bool autoDelete = false);

    /**
     * @brief appendCommandItem 添加 @CommandItem 到链
     * @param executor
     * @return this
     */
    ShellExcuteChain * appendCommandItem(CommandItem & executor);

    /**
     * @brief getExcutors 获取所有添加的 @CommandItem 列表
     * @return
     */
    QList<CommandItem *> & getExcutors();

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
     * @brief setAutoDetele 是否在任务全部完成后自动删除自己
     * @param newAutoDetele
     * @return this
     */
    ShellExcuteChain * setAutoDetele(bool newAutoDetele);

private:
    /**
     * @brief ShellExcuteChain 私有构造函数 @单例模式
     * @param maxThreadCount
     * @param autoDelete
     */
    ShellExcuteChain(int maxThreadCount, bool autoDelete);

    /**
     * @brief nextShellItem 获取链上下一个 @CommandItem
     * @param oldIndex 上一个 @CommandItem 在 @mExecutors 中的index
     * @return
     */
    CommandItem * nextShellItem(int oldIndex);

    /**
     * @brief checkCommandItemsCallbacks 函数会遍历 @mExecutors, 并检查内部所有 @CommandItem 是否设置了OnSuccess 和 OnError回调
     *        回调原型参照 @Callback
     * @return 是否所有 @CommandItem 都设置了OnSuccess 和 OnError回调
     */
    bool checkCommandItemsCallbacks();
private:
    OnAllCompletedCallback mOnAllCompletedCallback;
    QList<CommandItem *> mExecutors;
    QThreadPool mThreadPool;
    QString mLastResult;
    QString mLastError;
    bool mCancelWork;
    bool mAutoDetele;
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
     * @param item    要执行的 @CommandItem
     * @param index   该 @CommandItem 在 @mExecutors 中的index
     */
    ShellExcuteRunnable(CommandItem * item, int index) {
        mCommandItem = item;
        mIndex = index;
        setAutoDelete(true);
    }

    /**
     * @brief run
     */
    void run() {
        // 检查是否设置了onBefore (原型参考 @BeforeExecuteCallback)回调，如果已设置则调用
        if (mCommandItem->getBeforeExecuteCallback()) {
            mCommandItem->getBeforeExecuteCallback()(mCommandItem);
        }

        QProcess process;
        process.startCommand(mCommandItem->getCommand());

        process.waitForFinished();

        QString result = process.readAllStandardOutput();
        QString error = process.readAllStandardError();

        bool continuable = true;

        // 根据命令行执行结果调用OnSuccess 活着OnError
        if (!error.isEmpty())
            continuable = mCommandItem->getOnErrorCallback()(mCommandItem, error);
        else
            continuable = mCommandItem->getOnSuccessCallback()(mCommandItem, result);

        // 检查是否设置了 OnJobCompletedCallback (原型参考 @OnJobCompletedCallback)回调, 如果已设置则调用
        if (mOnJobCompletedCallback)
            mOnJobCompletedCallback(mIndex, continuable, result, error);
    }

    /**
     * @brief OnJobCompleted 设置 OnJobCompleted(原型 @OnJobCompletedCallback) 回调
     * @param cb
     */
    void OnJobCompleted(OnJobCompletedCallback cb) {
        mOnJobCompletedCallback = cb;
    }

    /**
     * @brief getCallback mOnJobCompletedCallback Getter
     * @return
     */
    OnJobCompletedCallback getCallback() {
        return mOnJobCompletedCallback;
    }

    /**
     * @brief setNext 设置下一个要执行的 @CommandItem 以及其对应的index
     * @param nextItem
     * @param newIndex
     */
    void setNext(CommandItem * nextItem, int newIndex) {
        mCommandItem = nextItem;
        mIndex = newIndex;
    }

private:
    CommandItem * mCommandItem;
    int mIndex;
    OnJobCompletedCallback mOnJobCompletedCallback;
};

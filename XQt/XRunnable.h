#ifndef XRUNNABLE_H
#define XRUNNABLE_H
#include <QRunnable>
#include <QThreadPool>

typedef std::function<void()> OnRunCallback;

class XRunnable : public QRunnable {
public:
    XRunnable() {
        setAutoDelete(true);
    }
    XRunnable * onRun(OnRunCallback cb){
        mOnRunCallback = cb;
        return this;
    }

    void run() {
        if (mOnRunCallback) mOnRunCallback();
    }

private:
    OnRunCallback mOnRunCallback;
};

#endif // XRUNNABLE_H

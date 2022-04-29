#ifndef XPROCESS_H
#define XPROCESS_H
#include <QProcess>
#include <QByteArray>
#include <QDebug>
#include <QString>

typedef std::function<void(const QString & result)> OnResultCallback;

class XProcess : public QProcess {
public:
    XProcess & onSuccess(OnResultCallback cb) {
        mOnSuccessBack = cb;
        return *this;
    }
    XProcess & onError(OnResultCallback cb) {
        mOnErrorCallback = cb;
        return *this;
    }

    XProcess & command(const QString cmd) {
        mCmd = cmd;
        return *this;
    }

    XProcess & printCommand(){
        qDebug() << "[Command]: " << mCmd;
        return *this;
    }

    void start() {
        QProcess process;
        process.startCommand(mCmd);
        process.waitForFinished();

        QString result = process.readAllStandardOutput();
        QString error = process.readAllStandardError();

        if(mOnErrorCallback && !error.isEmpty()) {
            mOnErrorCallback(error);
            return;
        }

        if (mOnSuccessBack) mOnSuccessBack(result);
    }
private:
    OnResultCallback mOnSuccessBack;
    OnResultCallback mOnErrorCallback;
    QString mCmd;
};

#endif // XPROCESS_H

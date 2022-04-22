#include "ShellExcuteChain.h"
#include <QDebug>


int main(int argc, char *argv[])
{
	// 从Android手机导出 微信的 apk包 测试
    ShellExcuteChain * chain = ShellExcuteChain::getInstance();
	
	static QString device = "XXXXX";
	static QString packageName = "com.tencent.mm";
	static QString exportSavePath = "D:/Export";

    chain->appendCommandItem(
                CommandItem()
                .command(QString("adb -s %1 shell pm path %2").arg(device).arg(packageName))
                .onSuccess([&](CommandItem * thisItem, QString & result) -> bool {
                    // 处理apk的完整路径
                    result = result.remove("package:").remove("\r").remove("\n");

                    // 继续执行
                    return true;
                })
                .onError([&](CommandItem * thisItem, const QString & error) -> bool {
                    QString msg = QString("[%1] 导出失败，请检查App是否安装!").arg(packageName);
                    qDebug() << msg;

                    // 终止执行
                    return false;
                })
            )
        ->appendCommandItem(
                CommandItem()
                .command(QString("adb -s %1 pull [PATH] %3/%4.apk").arg(device).arg(exportSavePath).arg(packageName))
                .onBefore([chain](CommandItem * thisItem) {
                    // 在开始执行以前，把上一步的执行结果应用到当前Item
                     thisItem->getCommand().replace("[PATH]", chain->getLastResult());
                })
                .onSuccess([&, chain](CommandItem * thisItem, QString & result) -> bool {
                    QString savaPath = QString("%1/%2.apk").arg(exportSavePath).arg(packageName);
                    QString msg = QString("[%1] 导出成功，保存路径: %2").arg(packageName).arg(savaPath);
                    qDebug() << msg;
                    return true;
                })
                .onError([&](CommandItem * thisItem, const QString & error) -> bool {
                    QString msg = QString("[%1] 导出失败，原因:%2").arg(packageName).arg(error);
                    qDebug() << msg;
                    return false;
                })
            )
        ->onAllCompleted([chain]{
                delete chain;
            })
        ->start();
}

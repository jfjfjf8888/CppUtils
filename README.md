# CppUtils

## ShellExcuteChain 是一个基于C++、Qt 和 lambda 的 Shell命令执行链，可以连续执行多个相互关联的shell命令

## XQt 对QProcess、QRunnable进行了扩展，使其支持lambda表达式，让程序逻辑更加清晰

    ```C++

    // include header files
    #include "XRunnable.h"
    #include "XProcess.h"


    // use
    QThreadPool::globalInstance()->start((new XRunnable())->onRun([this](){
        XProcess().command("[command]").onSuccess([this](const QString & result){
            // execute [command] success
            // YOUR CODE
        }).onError([this](const QString & error){
            // execute [command] error
            // YOUER CODE
        }).start();
    }));
    ```

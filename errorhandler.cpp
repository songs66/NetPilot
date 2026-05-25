#include "errorhandler.h"

ErrorHandler::ErrorHandler(QObject *parent)
    : QObject{parent}
{}

ErrorHandler::~ErrorHandler() {
    if (logStream) {
        // 先删流，再关文件
        delete logStream;
    }
    if (logFile) {
        logFile->close();   // 确保落盘
        delete logFile;
    }
}

// 功能：获取全局单例，首调时创建并初始化日志
ErrorHandler& ErrorHandler::instance(){
    static ErrorHandler instance; // 首次调用创建，进程结束自动析构
    return instance;              // 返回引用，避免拷贝
}

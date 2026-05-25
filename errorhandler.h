#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>

#include <QString>      // 文本处理（消息、路径等）
#include <QWidget>      // 父窗口指针，用于消息框归属
#include <QMessageBox>  // 用户可见的弹窗提示
#include <QDateTime>    // 时间戳，用于日志前缀与按日切换
#include <QFile>        // 文件句柄，写入日志
#include <QFileInfo>    // 获取文件大小等
#include <QTextStream>  // 文本流，负责编码与缓冲

#include <QMutex>       // 互斥锁，保证多线程写日志安全
#include <QMutexLocker> // RAII方式持锁，防止异常提前释放

#include <QApplication> //Qt图形界面程序的“总管家”+“启动入口”
#include <QDebug>       // 调试输出，可辅助开发阶段

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")    // MSCV下确保源文件按 UTF-8解析
#endif

class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    // 功能：构造函数，初始化成员并创建当前日志
    explicit ErrorHandler(QObject *parent = nullptr);

    // 功能：析构函数：安全关闭并释放资源
    ~ErrorHandler();

    // 错误来源分类：便于在日志与弹窗中标识错误归属
    enum ErrorType {
        NetworkError,       // 网络异常
        ValidationError,    // 输入/校验异常
        FileError,          // 文件读写异常
        ConfigError,        // 配置异常
        UnknownError        // 未知异常
    };

    // 错误严重程序：控制展示样式、日志等级及是否触发退出
    enum ErrorLevel {
        Info,               // 仅信息
        Warning,            // 警告
        Critical,           // 严重
        Fatal               // 致命（会触发退出）
    };

    // 功能：获取全局单例，首调时创建并初始化日志
    static ErrorHandler& instance();

    // 功能：弹出消息框并记录日志，按级别选择图标/标题，Fatal通常触发退出
    void handleError(ErrorType type, ErrorLevel level, const QString &message,
                     QWidget *parent = nullptr);

    // 功能：仅记录日志（不弹窗），包含时间/级别/来源
    void logError(const QString &context, const QString &error,
                  ErrorLevel level = Warning);

    // 功能：设置/切换日志文件路径（追加），后续按日期/大小自动流动
    void setLogFile(const QString &filename);

    // 功能：开启/关闭日志写盘
    void setLoggingEnabled(bool enabled);

    // 功能：错误来源枚举转可读字符串
    static QString errorTypeToString(ErrorType type);

    // 功能：错误级别枚举转可读字符串
    static QString errorLevelToString(ErrorLevel level);

private:
    // 禁止对象拷贝
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;

    QFile *logFile;         // 当前打开的日志文件句柄（可能为nullptr）
    QTextStream *logStream; // 配套文本流，负责写入编码与刷新
    QMutex logMutex;        // 保护logFile/logStream的互斥锁
    bool loggingEnabled;    // 总开关

    // 功能：初始化，切换当前日志，创建目录与文件
    void initializeLogFile();

    // 功能：在持锁状态下写入日志行并刷新
    void writeToLog(const QString &message);

    // 功能：按大小滚动日志文件
    void rolloverIfNeeded_unlocked(qint64 maxBytes = 5 * 1024 * 1024);

    // 功能：当前日志文件路径
    QString currentLogPath;

    // 功能：按日期切换日志（需先持锁）
    void reopenForToday_unlocked();
};

// 定义宏：统一日志格式与默认等级
// 记录严重错误
#define LOG_ERROR(context, message) \
    ErrorHandler::instance().logError(context, message, ErrorHandler::Critical)
// 记录警告
#define LOG_WARNING(context, message) \
    ErrorHandler::instance().logError(context, message, ErrorHandler::Warning)
// 记录信息
#define LOG_INFO(context, message) \
    ErrorHandler::instance().logError(context, message, ErrorHandler::Info)
// 弹窗并记录，统一入口
#define HANDLE_ERROR(type, level, message, parent) \
    ErrorHandler::instance().handleError(type, level, message, parent)

#endif // ERRORHANDLER_H

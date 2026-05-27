#include "errorhandler.h"


#include <QStandardPaths>                   // 获取平台通用的可写目录
#include <QDir>                             // 目录操作（创建路径等）
#include <QApplication>                     // 退出应用 应用信息

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) // 仅Qt6需要显式编码转换类
#include <QStringConverter>                 // Qt6编码设置
#endif

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")    // MSVC编译时按UTF-8解释源文件
#endif


// 功能：构造函数，初始化成员并创建当日日志
ErrorHandler::ErrorHandler(QObject *parent)
    : QObject{parent}
{}
// 功能：析构函数，关闭并释放日志资源
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



void ErrorHandler::handleError(ErrorType type, ErrorLevel level, const QString &message, QWidget *parent) {
    // 先写日志，context 取自错误类型
    QString context = errorTypeToString(type);
    logError(context,message,level);

    // 再根据级别弹出消息框
    QMessageBox::Icon icon;
    QString title;

    // 设置映射级别到 UI 图标与标题
    switch (level) {
    case Info:
        icon = QMessageBox::Information;    // 信息图标
        title = "信息";
        break;
    case Warning:
        icon = QMessageBox::Warning;    // 警告图标
        title = "警告";
        break;
    case Critical:
        icon = QMessageBox::Critical;    // 错误图标
        title = "错误";
        break;
    case Fatal:
        icon = QMessageBox::Critical;    // 严重错误同样用 Critical 图标
        title = "严重错误";
        break;
    default:
        icon = QMessageBox::Critical;    // 未知错误按照严重处理
        title = "严重错误";
        break;
    }

    // 以父窗口作为归属，避免顶层无父导致窗口乱序
    QMessageBox msgBox(parent);
    msgBox.setIcon(icon);                       // 设置图标
    msgBox.setWindowTitle(title);               // 设置标题
    msgBox.setText(message);                    // 主要文本
    msgBox.setStandardButtons(QMessageBox::Ok); // 仅确认按钮
    msgBox.exec();                              // 阻塞显示

    // Fatal 级别：记录后直接请求退出应用
    if (level == Fatal) {
        QApplication::quit();
    }
}

// 功能：仅记录日志（不弹窗），包含时间/级别/来源/内容
void ErrorHandler::logError(const QString &context, const QString &error, ErrorLevel level)
{
    // 日志关闭或流未初始化时直接返回
    if (!loggingEnabled || !logStream) {
        return;
    }

    QMutexLocker locker(&logMutex); // 保护日志写入的互斥

    rolloverIfNeeded_unlocked();    // 按大小判断是否滚动
    reopenForToday_unlocked();      // 按日期判断是否切换

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"); // 当前时间
    QString levelStr = errorLevelToString(level);                                     // 级别文本

    // 统一的日志格式：[时间][级别][来源] 内容
    QString logMessage = QString("[%1] [%2] [%3] %4").arg(timestamp, levelStr, context, error);

    writeToLog(logMessage);  // 持锁写入
}


// 功能：设置/切换日志文件路径（追加），后续按日期/大小自动流动
void ErrorHandler::setLogFile(const QString &filename)
{
    QMutexLocker locker(&logMutex); // 切换文件也需互斥保护

    // 若已有流/文件，先释放
    if (logStream) {
        delete logStream;
        logStream = nullptr;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }

    // 尝试打开新的日志文件（追加模式）
    logFile = new QFile(filename);
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        logStream = new QTextStream(logFile); // 绑定文本流
        currentLogPath = filename;            // 记录当前路径

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        logStream->setCodec("UTF-8");             // Qt5设置编码
#else
        logStream->setEncoding(QStringConverter::Utf8); // Qt6设置编码
#endif
    } else { // 打开失败：释放指针并发出警告
        delete logFile;
        logFile = nullptr;
        qWarning() << "无法打开日志文件:" << filename;
    }
}

// 功能：在持锁状态下写入日志行并刷新
void ErrorHandler::writeToLog(const QString &message)
{
    if (logStream) {                            // 仅在流可用时写入
        *logStream << message << Qt::endl;      // Qt::endl 刷新缓冲区
        logStream->flush();                     // 再显式刷新，确保立即落盘
    }
}

// 功能：错误来源枚举转可读字符串
QString ErrorHandler::errorTypeToString(ErrorType type)
{
    switch (type) {
    case NetworkError:    return "网络错误";
    case ValidationError: return "验证错误";
    case FileError:       return "文件错误";
    case ConfigError:     return "配置错误";
    case UnknownError:
    default:              return "未知错误"; // 兜底，避免空字符串
    }
}

// 功能：错误级别枚举转可读字符串
QString ErrorHandler::errorLevelToString(ErrorLevel level)
{
    switch (level) {
    case Info:     return "信息";
    case Warning:  return "警告";
    case Critical: return "错误";
    case Fatal:    return "致命";
    default:       return "未知";        // 兜底
    }
}


void ErrorHandler::rolloverIfNeeded_unlocked(qint64 maxBytes)
{
    if (currentLogPath.isEmpty() || !logFile) return;   // 未初始化则跳过
    QFileInfo info(*logFile);                           // 查询当前大小
    if (info.size() < maxBytes) return;                 // 未超阈值则不滚动

    // 生成带时间戳的滚动文件名
    QString timeSuffix = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString rolled = currentLogPath;
    rolled.replace(".log", QString("_%1.log").arg(timeSuffix));

    logStream->flush();                                 // 刷新缓冲
    logFile->close();                                   // 关闭当前文件
    QFile::rename(currentLogPath, rolled);              // 重命名为历史文件

    // 释放旧指针
    delete logStream;
    delete logFile;
    logStream = nullptr;
    logFile = nullptr;

    setLogFile(currentLogPath);                         // 重新创建一个同名新文件
}

// 功能：按日期切换日志（需先持锁）
void ErrorHandler::reopenForToday_unlocked()
{
    if (currentLogPath.isEmpty()) return;                           // 未初始化则跳过
    QString today = QDate::currentDate().toString("yyyyMMdd");      // 今日日期
    if (currentLogPath.contains(today)) return;                     // 已是今日文件则不切换


    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    // 如果路径以应用名称结尾，则使用父目录（与initializeLogFile逻辑一致）
    QString appName = QApplication::applicationName();
    if (!appName.isEmpty() && (appDataPath.endsWith("/" + appName) ||
                               appDataPath.endsWith("\\" + appName))) {
        QDir parentDir = appDataDir;
        if (parentDir.cdUp()) {
            appDataPath = parentDir.absolutePath();
        }
    }
    QDir().mkpath(appDataPath);     // 确保目录存在
    QString newPath = QString("%1/NDAssistantTools_%2.log").arg(appDataPath, today); // 新日期文件

    logStream->flush();             // 刷新当前文件
    logFile->close();               // 关闭旧文件
    delete logStream;               // 释放流
    delete logFile;                 // 释放文件
    logStream = nullptr;
    logFile = nullptr;
    setLogFile(newPath);            // 打开新日期文件
}






























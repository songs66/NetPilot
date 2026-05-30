#ifndef FORMTCPSERVER_H
#define FORMTCPSERVER_H
/*
 fortcpserver.h/.cpp  提供TCP服务器界面及逻辑  TCP协议调试模块头文件接口声明
 UI初始化与样式
 服务器控制
 连接管理
 消息收发
 日志展示与持久化
 校验与提示
*/

#include <QWidget>


#include <QTcpServer>               // TCP 服务器
#include <QTcpSocket>               // TCP 客户端套接字
#include <QNetworkInterface>        // 网络接口信息（可用于本机 IP）
#include <QDateTime>                // 时间戳，用于日志
#include <QMessageBox>              // 用户提示
#include <QFile>                    // 文件保存
#include <QTextStream>              // 文本流写文件
#include <QListWidget>              // 日志列表控件
#include <QColor>                   // 日志颜色
#include <QCloseEvent>              // 窗口关闭事件

#include <QSettings>                // 持久化最近使用的 IP/端口
#include <QStandardPaths>           // 获取配置文件目录
#include <QDir>                     // 目录操作
#include "errorhandler.h"           // 统一错误处理的与弹窗



namespace Ui {
class FormTcpServer;
}

class FormTcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit FormTcpServer(QWidget *parent = nullptr);
    ~FormTcpServer();

private slots:
    void on_pushButton_TCPServerStart_clicked();

    void on_pushButton_TCPServerStop_clicked();

    void on_pushButton_TCPServerClose_clicked();

    void on_pushButton_TCPServerSendMsg_clicked();

private:
    Ui::FormTcpServer *ui;


    QTcpServer *tcpServer = nullptr;  // 服务器监听实例
    QTcpSocket *tcpServerSocket = nullptr; // 最近一次建立的 socket，用于简单模式
    QList<QTcpSocket*> tcpServerSocketList; // 活跃客户端列表
    bool serverRunning = false;   // 当前是否在监听
    int receivedMessageCount = 0; // 已接收消息计数（累加）

    // 功能：在日志列表追加彩色行
    void appendColorLog(const QString &text, const QColor &color);

    // 裁剪日志行数，避免控件过大
    void trimLog(int keepRows = 1000, int trimStep = 200);

public:
    // 功能：检验 IP 字符串是否合法
    bool CheckIPAddrIsValid(QString strIpAddress);

    // 功能：将列表控件内部保存到文件（追加或覆盖由实现决定）
    void saveListWidgetToFile(QListWidget* listWidget);

    // 保存日志（public接口，供主窗口调用，无需参数）
    void saveLog();

    // 功能：窗口关闭事件；保存日志、清理资源
    void closeEvent(QCloseEvent *event) override;

    // 是Qt框架中的一个特殊宏，用于在类中声明槽函数（slot）。它是Qt信号与槽机制的重要组成部分。
public Q_SLOTS:   // Q_SLOTS告诉Qt的元对象系统，后面的函数是槽函数，public表示这些槽函数可以从外部调用
    // 功能：处理新客户端连接，记录并更新列表
    void TcpServerConnectedFunc();

    // 功能：处理客户端断开，清理列表与 UI 状态
    void ClientDisconnectedFunc();

    // 功能：读取所有可用数据并计数、展示日志
    void ReadAllDataFunc();

};

#endif // FORMTCPSERVER_H

#ifndef FORMUDPCLIENT_H
#define FORMUDPCLIENT_H

#include <QWidget>

// Qt核心与网络
#include <QWidget>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>

#include <QDateTime>
#include <QSettings>
#include <QTextCursor>
#include <QStandardPaths>
#include <QDir>

// 项目公共模块
#include "inputvalidator.h"
#include "errorhandler.h"

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif


// UDP客户端界面类：FormUdpClient，用于向UDP服务器发送数据报
// 接收服务器响应，并在界面中记录、裁剪收发日志。UDP套接字采用懒创建策略

namespace Ui {
class FormUdpClient;
}

class FormUdpClient : public QWidget
{
    Q_OBJECT

public:
    explicit FormUdpClient(QWidget *parent = nullptr);
    ~FormUdpClient();

private slots:
    void on_pushButton_UDPClientSendMsg_clicked();

private:
    // 套接字就绪标志
    bool socketReady=false;

    // 日志裁剪
    void trimLog(int keepBlocks=1000,int trimStep=200);

    // 窗口关闭时保存日志并接受关闭
    void closeEvent(QCloseEvent *event) override;

public:
    // 将指定QPlainTextEdit的全部记录只在到应用数据目录下，追加模式，UTF-8
    void savePlainTextEditToFile(QPlainTextEdit * plainTextEdit);

    // 保存当前界面日志到文件
    void saveLog();

    // UDP客户端套接字指针，初始化为nullptr
    QUdpSocket *UDPClientSocket=nullptr;

    // 发送目标地址缓存，可选用途
    QHostAddress UDPClientHostAddress;

public Q_SLOTS:
    // 读取服务器返回的数据报并加载到日志框，由readyRead信号触发
    void ReadServerDatagramFunc();


private:

private:
    Ui::FormUdpClient *ui;
};

#endif // FORMUDPCLIENT_H

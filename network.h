#ifndef NETWORK_H
#define NETWORK_H

// 代码已审核--Vico

// TCP客户端底层封装
// 职责：连接/断开，发送文本/字节，接收并转发数据，支持自动测试定时器与连接统计

#include <QObject>

#include <QTcpSocket>       // TCP套接字类
#include <QTimer>           // Qt定时器类
#include <QtCore>           // Qt核心模块的所有类
#include <QHostAddress>     // Qt主机地址类
#include <QAbstractSocket>  // Qt抽象套接字基类
#include <QElapsedTimer>    // Qt高精度计时器类
#include <QMutex>           // Qt互斥锁类

#include <QAtomicInt>       // Qt原子整型类--为扩展预留
#include <QStringBuilder>   // Qt字符串构建器--为扩展预留

#include <memory.h>         // C++标准库智能指针头文件----为扩展预留
#include <atomic>           // C++标准库原子操作头文件，提供std::atomic类型支持


#ifdef _MSC_VER
#pragma execution_character_set("utf-8")     // MSCV下确保源文件按 UTF-8解析
#endif

// Network类：继承自QObject，实现TCP客户端底层通信逻辑封装
// 设计要点：使用信号与槽机制实现异步通信，提供连接管理、数据收发、自动测试和统计功能
class Network : public QObject
{
    Q_OBJECT
public:
    // 构造函数：初始化网络客户端对象，创建套接字和定时器，建立信号槽连接
    explicit Network(QObject *parent = nullptr);

    // 析构函数：清理资源，停止定时器并断开套接字连接
    ~Network(void);

public:
    // 客户端连接到服务器：尝试建立TCP连接到指定的服务器地址和端口
    bool ClientConnectToServer(QString ServerIpAddress, int ServerPort);

    // 客户端发送文本消息到服务器：将UTF-8编码的字符串发送到服务器
    void ClientSendMsgToServer(const QString &StrData);

    // 客户端发送原始字节数据到服务器：直接发送字节数组，不进行编码转换
    void ClientSendBytesToServer(const QByteArray &data);

    // 设置套接字选项：配置TCP套接字的性能参数和连接参数
    void setSocketOptions();

    // 连接统计信息结构体：封装连接相关的统计数据和性能指针
    struct ConnectionStats{
        qint64 connectionTime=0;
        qint64 bytesSent=0;
        qint64 bytesReceived=0;
        qint64 messagesCount=0;
    };

    // 获取连接统计信息：返回当前连接的统计数据（只读访问）
    ConnectionStats getConnectionStats() const;

    // 最近一次接收到的数据缓存：存储最后一次从服务器收到的数据
    QString strTempData;


private:
    QTcpSocket *socket;                         // TCP套接字指针
    QTimer *timer;                              // 自动测试定时器指针
    std::atomic<int> testMessageCount;          // 自动测试发送计数（原子类型）
    QString autoTestMessage;                    // 自动测试消息内容
    QByteArray receiveBuffer;                   // 接收缓冲区
    QElapsedTimer performanceTimer;             // 性能计时器
    mutable QMutex bufferMutex;                 // 缓冲区互斥锁（可变类型）

Q_SIGNALS:  // Qt信号区域
    // 连接建立信号：当TCP连接成功建立时发出，无参数
    void connectionEstablished();

    // 连接失败信号：当TCP连接失败时发出，携带错误信息
    void connectionFailed(const QString &errorString);

    // 数据接收信号：当从服务器接收到数据时发出，携带接收到的数据内容
    void dataReceived(const QString &data);


public Q_SLOTS: // 公共槽函数区域
    // 客户端断开连接回调：当套接字断开连接时自动调用
    void ClientDisconnectedFunc(void);

    // 主动断开连接：由外部调用，主动断开与服务器TCP连接
    void DisconnectFromHost();

    // 读取服务器消息：当socket有数据可读时自动调用
    void ReadServerMsg();

    // 启动定时器超时功能：启动自动测试模式，定时向服务器发送测试消息
    void StartTimerOutFunc();

    // 设置自动测试消息：设置自动化测试模式下要发送的消息内容
    void setAutoTestMessage(const QString &message);

    // 停止定时器超时功能：停止自动测试模式，取消定时发送
    void StopTimerOutFunc();

private Q_SLOTS:
    // 连接建立回调：当socket成功连接到服务器时自动调用
    void onConnected();

    // 套接字错误处理回调：当socket发生错误时自动调用
    void onSocketError(QAbstractSocket::SocketError error);

};

#endif // NETWORK_H

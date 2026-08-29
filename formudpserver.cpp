#include "formudpserver.h"
#include "ui_formudpserver.h"

#include <QApplication>

FormUdpServer::FormUdpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormUdpServer)
{
    ui->setupUi(this);


    // 设置IP地址下拉框为可编辑
    ui->comboBox_UDPServerIp->setEditable(true);

    // 获取本机所有网络接口的IP地址列表
    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();

    // 创建IP地址字符串列表
    QStringList ipList;

    // 预分配列表容量
    ipList.reserve(addressList.size());

    // 遍历地址列表
    for(const QHostAddress &address:addressList){
        if(address.protocol()==QAbstractSocket::IPv4Protocol){
            // 将IPv4地址转换为字符串并添加到列表
            ipList.append(address.toString());
        }
    }

    // 批量添加IP地址到下拉框控件
    ui->comboBox_UDPServerIp->setUpdatesEnabled(false);

    // 批量添加所有IP地址
    ui->comboBox_UDPServerIp->addItems(ipList);

    ui->comboBox_UDPServerIp->setUpdatesEnabled(true);

    // 加载上次保存的配置：从QSettings读取上次IP和端口
    {
        QSettings settings;
        const QString lastIp = settings.value("UDPServer/lastIp").toString();
        const int lastPort = settings.value("UDPServer/lastPort", 9999).toInt();

        if(!lastIp.isEmpty()){
            int index = ui->comboBox_UDPServerIp->findText(lastIp);

            if(index>=0){
                ui->comboBox_UDPServerIp->setCurrentIndex(index);
            }else{
                ui->comboBox_UDPServerIp->setEditText(lastIp);
            }
        }

        if(lastPort>=ui->spinBox_UDPServerPort->minimum() &&
            lastPort<=ui->spinBox_UDPServerPort->maximum()){
            ui->spinBox_UDPServerPort->setValue(lastPort);
        }
    }

    // 创建UDP服务器套接字
    udpServerSocket=new QUdpSocket(this);

    // 建立信号槽连接：将UDP套接字readyRead信号连接到数据读取槽函数
    connect(udpServerSocket, &QUdpSocket::readyRead,
            this, &FormUdpServer::udpSeverReadPendingDatagrams);

    // 设置控件尺寸策略为固定大小：防止窗口大小改变时控件尺寸变化，保持界面稳定
    ui->groupBox_Left->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->groupBox_Right->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->listWidget_UDPServerListMsg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->label_1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->label_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->comboBox_UDPServerIp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->spinBox_UDPServerPort->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->pushButton_UDPServerClose->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->pushButton_UDPServerStart->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (ui->plainTextEdit_UDPServerSendData) {
        ui->plainTextEdit_UDPServerSendData->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    if (ui->pushButton_UDPServerSendMsg) {
        ui->pushButton_UDPServerSendMsg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // 设置界面样式：使用Qt样式表（QSS）美化界面，定义控件的视觉效果
    setStyleSheet(
        // 基础窗口样式：设置所有控件的默认背景色和文字颜色
        "QWidget {"
        "  background-color: #F3F4F6;"      // 浅灰色背景（RGB: 243, 244, 246）
        "  color: #1F2933;"                 // 深灰色文字（RGB: 31, 41, 51）
        "}"
        // 分组框样式：设置分组框（QGroupBox）的外观
        "QGroupBox {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  background-color: #FFFFFF;"      // 白色背景
        "  border: 1px solid #D0D7E2;"      // 浅灰色边框，1 像素实线
        "  border-radius: 6px;"             // 圆角半径 6 像素
        "  margin-top: 10px;"               // 上边距 10 像素
        "}"
        // 分组框标题样式：设置分组框标题的样式
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"      // 标题位置基于边距
        "  left: 10px;"                     // 距离左边 10 像素
        "  padding: 0 4px;"                 // 内边距：上下 0，左右 4 像素
        "  background-color: #F3F4F6;"      // 浅灰色背景（与窗口背景一致）
        "  color: #111827;"                 // 深灰色文字
        "}"
        // 标签样式：设置标签（QLabel）的样式
        "QLabel {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  color: #374151;"                 // 中灰色文字（RGB: 55, 65, 81）
        "  background-color: transparent;"  // 透明背景
        "}"
        // 下拉框和数字输入框样式：设置下拉框（QComboBox）和数字输入框（QSpinBox）的样式
        "QComboBox, QSpinBox {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  background-color: #FFFFFF;"      // 白色背景
        "  border: 1px solid #D0D7E2;"      // 浅灰色边框
        "  border-radius: 4px;"             // 圆角半径 4 像素
        "  padding: 2px 6px;"                // 内边距：上下 2 像素，左右 6 像素
        "}"
        // 文本编辑框样式：设置多行文本编辑框（QPlainTextEdit）的样式
        "QPlainTextEdit {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  background-color: #FFFFFF;"      // 白色背景
        "  border: 1px solid #D0D7E2;"      // 浅灰色边框
        "  border-radius: 4px;"             // 圆角半径 4 像素
        "}"
        // 按钮基础样式：设置按钮（QPushButton）的默认样式
        "QPushButton {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  background-color: #2563EB;"      // 蓝色背景（RGB: 37, 99, 235）
        "  color: #FFFFFF;"                 // 白色文字
        "  border-radius: 4px;"             // 圆角半径 4 像素
        "  padding: 4px 12px;"               // 内边距：上下 4 像素，左右 12 像素
        "  border: none;"                    // 无边框
        "}"
        // 按钮悬停样式：鼠标悬停在按钮上时的样式
        "QPushButton:hover {"
        "  background-color: #1D4ED8;"      // 深蓝色背景（悬停效果）
        "}"
        // 按钮按下样式：鼠标按下按钮时的样式
        "QPushButton:pressed {"
        "  background-color: #1E40AF;"      // 更深蓝色背景（按下效果）
        "}"
        // 按钮禁用样式：按钮被禁用时的样式
        "QPushButton:disabled {"
        "  background-color: #CBD5F5;"      // 浅蓝色背景（禁用状态）
        "  color: #6B7280;"                 // 灰色文字（禁用状态）
        "}"
        // 列表控件样式：设置列表控件（QListWidget）的样式
        "QListWidget {"
        "  font-size: 14px;"                // 字体大小 14 像素
        "  background-color: #FFFFFF;"      // 白色背景
        "  border: 1px solid #D0D7E2;"      // 浅灰色边框
        "  border-radius: 4px;"             // 圆角半径 4 像素
        "}"
        // 列表项选中样式：设置列表项被选中时的样式
        "QListWidget::item:selected {"
        "  background-color: #FEE2E2;"      // 浅红色背景（选中高亮）
        "}"
        );

    // 设置列表控件属性：优化列表控件的显示和选择行为
    ui->listWidget_UDPServerListMsg->setUniformItemSizes(true);

    // 设置选项模式
    ui->listWidget_UDPServerListMsg->setSelectionMode(QAbstractItemView::SingleSelection);

    // DPI设置已在main.cpp中统一配置，此处不再重复设置

}

FormUdpServer::~FormUdpServer()
{
    delete ui;
}

// 启动/停止服务器按钮点击事件：切换UDP服务器的监听状态
/* 处理流程：
1、读取IP地址和端口号
2、验证输入有效性
3、启动监听
4、停止监听
5、更新按钮文本和运行状态
6、保存配置到QSettings
*/
void FormUdpServer::on_pushButton_UDPServerStart_clicked()
{
    // 获取用户输入的IP地址
    QString ip=ui->comboBox_UDPServerIp->currentText();

    // 将IP地址字符串转换为QHostAddress对象，用于验证和绑定操作
    QHostAddress hostAddress(ip);

    // 获取用户输入的端口号
    quint16 port=ui->spinBox_UDPServerPort->value();

    // 判断当前按钮状态：根据按钮文本判断是启动还是停止操作
    if(ui->pushButton_UDPServerStart->text()=="启动监听"){
        // 第一层验证：使用输入验证器验证IP地址格式
        auto ipValidation=InputValidator::validatorNetworkAddress(ip);
        if(!ipValidation.isValid){
            HANDLE_ERROR(ErrorHandler::ValidationError,ErrorHandler::Warning,ipValidation.errorMessage,this);

            return;
        }

        // 第二层验证：验证QHostAddress对象是否有效且为IPv4协议
        if(hostAddress.isNull() || hostAddress.protocol()!=QAbstractSocket::IPv4Protocol){
            HANDLE_ERROR(ErrorHandler::ValidationError,ErrorHandler::Warning,"无效的IP地址",this);

            return;
        }

        // 第三层验证：验证端口号是否在有效范围内
        if(port < 1 || port > 65535){
            HANDLE_ERROR(ErrorHandler::ValidationError,ErrorHandler::Warning,"端口号必须在1-65535范围内",this);

            return;
        }

        // 第四层验证：验证UDP套接字是否已初始化
        if(!udpServerSocket){
            HANDLE_ERROR(ErrorHandler::NetworkError,ErrorHandler::Critical,"UDP套接字未初始化",this);

            return;
        }

        // 绑定IP地址和端口
        if(udpServerSocket->bind(hostAddress,port)){
            ui->pushButton_UDPServerStart->setText("关闭监听");
            udpServerAppendStrItem(1,"UDP服务器启动监听成功",false);
            serverRunning=true;

            // 保存配置IP和端口
            QSettings settings;
            settings.setValue("UDPServer/lastIp",hostAddress.toString());
            settings.setValue("UDPServer/lastPort",static_cast<int>(port));

            // 立即同步磁盘
            settings.sync();
        }else{
            // 绑定失败
            if(udpServerSocket){
                HANDLE_ERROR(ErrorHandler::NetworkError,ErrorHandler::Warning,udpServerSocket->errorString(),this);
            }else{
                HANDLE_ERROR(ErrorHandler::NetworkError,ErrorHandler::Warning,"UDP套接字未初始化",this);
            }
        }
    }else{
        // 停止监听：当前按钮文本为“关闭监听”，需要停止服务器监听
        if(udpServerSocket){
            udpServerSocket->abort();
        }

        // 更新按钮文本：将按钮文本改回“启动监听”，表示当前已停止，下次点击将启动
        ui->pushButton_UDPServerStart->setText("启动监听");

        // 追加状态日志
        udpServerAppendStrItem(1,"UDP服务器已停止监听",false);

        // 更新运行状态标志
        serverRunning=false;
    }

}

// 关闭按钮点击事件：保存日志并退出应用程序
void FormUdpServer::on_pushButton_UDPServerClose_clicked()
{
    // 保存日志到文件
    saveListWidgetToFile(ui->listWidget_UDPServerListMsg);

    // 退出应用程序
    QCoreApplication::quit();

}

// 发送测试消息按钮点击事件：向最近连接的客户端发送测试消息
/*处理流程：
1、验证套接字和服务器状态
2、验证是否有客户端地址
3、获取并验证消息内容
4、编码消息并发送
5、记录发送日志
*/
void FormUdpServer::on_pushButton_UDPServerSendMsg_clicked()
{
    // 第一层检查：验证UDP套接字是否已初始化
    if(!udpServerSocket){
        HANDLE_ERROR(ErrorHandler::NetworkError,ErrorHandler::Warning,"UDP套接字未初始化，无法发送数据。",this);

        return;
    }

    // 第二层检查：验证服务器是否正在运行
    if(!serverRunning){
        HANDLE_ERROR(ErrorHandler::NetworkError,ErrorHandler::Warning,"请先启动UDP服务器,监听再发送测试消息。",this);

        return;
    }

    // 第三层检查：验证是否有客户端地址信息
    if(m_lastClientPort==0 || m_lastClientAddress.isNull()){
        HANDLE_ERROR(ErrorHandler::ValidationError,ErrorHandler::Warning,"当前还没有收到任何客户端的消息，无法确定发送目标。",this);

        return;
    }

    // 第四层检查：验证输入框控件是否存在
    if(!ui->plainTextEdit_UDPServerSendData){
        return;
    }

    QString message=ui->plainTextEdit_UDPServerSendData->toPlainText().trimmed();

    // 第五层检查：验证消息内容是否为空
    if(message.isEmpty()){
        QMessageBox::warning(this,"提示","发送内容不能为空！");

        ui->plainTextEdit_UDPServerSendData->setFocus();

        return;
    }

    QByteArray data=message.toUtf8();

    // 发送数据报到客户端
    qint64 bytes=udpServerSocket->writeDatagram(data,m_lastClientAddress,m_lastClientPort);

    // 检查发送结果：验证数据报是否成功发送
    if(bytes==-1){
        QMessageBox::warning(this, "错误", QString("发送失败: %1").arg(udpServerSocket->errorString()));
        return;
    }

    // 记录发送日志
    QString timestamp=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

    // 构建日志条目
    QString logEntry = QString("\n[%1] 服务器发送到 %2:%3\n%4\n")
                           .arg(timestamp, m_lastClientAddress.toString())
                           .arg(m_lastClientPort)
                           .arg(message);

    // 添加日志项到列表
    ui->listWidget_UDPServerListMsg->addItem(logEntry);

    // 自动滚动到底部
    ui->listWidget_UDPServerListMsg->scrollToBottom();

}




// 读取并处理收到的数据报：当UDP套接字有数据可读时自动调用
void FormUdpServer::udpSeverReadPendingDatagrams()
{
    if(!serverRunning){
        return;
    }

    if(!udpServerSocket){
        qWarning()<<"FormUdpServer::udpServerReadPendingDatagrams: udpServerSocket is null";
        return;
    }

    while(udpServerSocket->hasPendingDatagrams()){
        QByteArray datagram;
        qint64 datagramSize=udpServerSocket->pendingDatagramSize();
        if(datagramSize<=0){
            break;  // 无效的数据报大小，退出循环
        }

        datagram.resize(static_cast<int>(datagramSize));
        QHostAddress clientAddress;
        quint16 clientPort;

        qint64 bytesRead=udpServerSocket->readDatagram(datagram.data(),datagram.size(),&clientAddress,&clientPort);
        if(bytesRead==-1){
            qWarning()<<"读取UDP数据报失败："<<udpServerSocket->errorString();
            continue;   // 读取失败，继续处理下一个数据报
        }

        m_lastClientAddress=clientAddress;
        m_lastClientPort=clientPort;

        udpServerAppendStrItem(0,QString::fromUtf8(datagram),false);

        QByteArray response=QString("[Server reply:%1").arg(QString::fromUtf8(datagram)).toUtf8();
        udpServerSocket->writeDatagram(response,clientAddress,clientPort);

    }

}

// 追加日志项到列表：将消息或状态信息追加到日志列表控件
void FormUdpServer::udpServerAppendStrItem(int type,const QString &strData,bool clear)
{
    Q_UNUSED(clear);

    QString timestamp=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    QString processedData=strData;
    processedData.replace("\r","").replace("\n","");

    QString logEntry=QString("\n[%1] %2\n%3\n")
                           .arg(timestamp)
                           .arg(type==0?"接收数据":"服务器状态")
                           .arg(processedData);

    // 性能优化：批量更新UI
    ui->listWidget_UDPServerListMsg->setUpdatesEnabled(false);
    ui->listWidget_UDPServerListMsg->addItem(logEntry);
    trimLog();
    ui->listWidget_UDPServerListMsg->setUpdatesEnabled(true);

    // 自动滚动到底部，显示最新消息（只调用一次）
    int lastRow=ui->listWidget_UDPServerListMsg->count()-1;
    if(lastRow==0){
        QListWidgetItem *item=ui->listWidget_UDPServerListMsg->item(lastRow);
        if(item){
            ui->listWidget_UDPServerListMsg->scrollToItem(item,QAbstractItemView::PositionAtBottom);
            ui->listWidget_UDPServerListMsg->setCurrentRow(lastRow);
        }
    }

}

// 日志裁剪函数：当日志列表项数超过阈值时，删除前部项以控制内存使用
void FormUdpServer::trimLog(int keepRows,int trimStep)
{
    // 定义默认值常量：使用静态常量定义默认的保留行数和裁剪步长
    static const int kKeepDefault=1000;
    static const int kTrimDefault=200;

    // 确定实际保留行数：如果参数有效则使用参数值，否则使用默认值
    const int targetKeep=keepRows>0?keepRows:kKeepDefault;
    const int step=trimStep>0?trimStep:kTrimDefault;

    int count=ui->listWidget_UDPServerListMsg->count();

    // 检查是否需要裁剪：如果项数未超过阈值，则不需要裁剪
    if(count<=targetKeep+step)
    {
        return;
    }

    // 计算需要删除的项数：当前项数减去要保留的项数
    int removeCount=count-targetKeep;

    // 性能优化：批量删除时禁用UI更新，减少重绘次数
    if(removeCount>10){
        ui->listWidget_UDPServerListMsg->setUpdatesEnabled(false);
    }

    // 创建等待删除项列表：用于存储需要删除的列表项指针
    QList<QListWidgetItem*> itemsToDelete;

    // 遍历并移除前部项：从列表开头移除指定数量的项
    for(int i=0;i<removeCount;i++){
        QListWidgetItem *item=ui->listWidget_UDPServerListMsg->takeItem(0);
        if(item){
            itemsToDelete.append(item);
        }
    }

    // 重新启用UI更新：恢复控件的自动更新，触发一次重绘
    if(removeCount>10){
        ui->listWidget_UDPServerListMsg->setUpdatesEnabled(true);
    }

    // 批量删除项对象：释放所有等待删除项占用的内存
    qDeleteAll(itemsToDelete);
}

// 窗口关闭事件处理函数：当窗口关闭时自动调用，保存日志并接受关闭
void FormUdpServer::closeEvent(QCloseEvent *event)
{
    // 保存日志：在窗口关闭之前将日志列表保存到文件
    saveLog();

    // 接受关闭事件：允许窗口关闭，窗口将正常关闭
    event->accept();

}

// 保存日志列表到文本文件：将QListWidget中的所有项保存到文本文件
void FormUdpServer::saveListWidgetToFile(QListWidget* listWidget)
{
    // 第一层检查：验证列表控件指针是否有效
    if(!listWidget){
        qWarning()<<"FormUdpServer::saveListWidgetToFile: listWidget is null";
        return;
    }

    // 获取应用程序数据目录：获取应用程序可以写入数据的标准目录路径
    // 使用AppDataLocation的父目录，确保与INI文件在同一目录：%APPDATA%\NDATools
    QString appDataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    // 如果路径以应用名称结尾，则使用父目录
    QString appName = QApplication::applicationName();
    if (!appName.isEmpty() && (appDataPath.endsWith("/" + appName) || 
                               appDataPath.endsWith("\\" + appName))) {
        QDir parentDir = appDataDir;
        if (parentDir.cdUp()) {
            appDataPath = parentDir.absolutePath();
        }
    }

    // 确保目录存在：如果目录不存在，则创建目录（包括所有父目录）
    QDir().mkpath(appDataPath);

    // 构建文件完整路径：将文件名与目录路径组合，得到完整的文件路径
    QString fileName=QDir(appDataPath).filePath("UDPServerLogfile.txt");

    // 检查文件是否存在：判断日志文件是否已经存在，决定是追加还是创建新文件
    bool fileExists=QFile::exists(fileName);

    // 确定文件打开模式：根据文件是否存在选择追加模式或写入模式
    QIODevice::OpenMode openMode = fileExists ?
                                       (QIODevice::Append | QIODevice::Text) :
                                       (QIODevice::WriteOnly | QIODevice::Text);

    // 创建文件对象：使用文件路径创建QFile对象，用于文件操作
    QFile file(fileName);

    // 打开文件：以指定的模式打开文件
    if(file.open(openMode)){
        QTextStream out(&file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // Qt 5 设置编码：使用 setCodec() 方法设置字符编码
        out.setCodec("UTF-8");
#else
        // Qt 6 设置编码：使用 setEncoding() 方法设置字符编码
        out.setEncoding(QStringConverter::Utf8);
#endif

        // 如果是追加模式，写入分隔符和时间戳：区别不同保存传话的日志
        if(file.exists()){
            QString timestamp=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");


            // 写入分隔符和时间戳：在追加的日志前添加分隔线，便于区分不同保存会话
            // 条件编译：Qt 5 和 Qt 6 的换行符常量不同
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // Qt 5 使用 endl：输出换行符并刷新流
            out << "\n" << "========== " << timestamp << " ==========" << endl;
#else
            // Qt 6 使用 Qt::endl：输出换行符并刷新流
            out << "\n" << "========== " << timestamp << " ==========" << Qt::endl;
#endif


            // 遍历列表项并写入文件：将所有列表项的文本内容写入文件
            for(int i=0;i<listWidget->count();i++){
                QListWidgetItem *item=listWidget->item(i);

                if (item) {
                    // 写入列表项文本：将列表项的文本内容写入文件，每项一行
                    // text()：返回列表项显示的文本内容
                    // 条件编译：Qt5 和 Qt6的换行符常量不同
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    out << item->text() << endl;
#else
                    out << item->text() << Qt::endl;
#endif
                }
            }

            file.close();

            qDebug()<<"UDP服务器日志已保存到："<<fileName;

            if(this->isVisible()){
                QMessageBox::information(this, "成功", QString("日志已保存至 %1").arg(fileName));
            }
        }
        else{  // 文件打开失败：输出警告信息，记录错误原因
            qWarning() << "UDP服务器日志保存失败:" << file.errorString();

            // 显示错误消息框：如果窗口可见，显示错误提示
            if(this->isVisible()){
                QMessageBox::critical(this, "错误", "保存失败: " + file.errorString());
            }
        }
    }
}

// 保存日志函数：保存当前界面的日志列表到文件（便捷掊）
void FormUdpServer::saveLog()
{
    // 检查UI对象和列表控件是否存在：防御性编程，确保对象有效后再使用
    if(ui && ui->listWidget_UDPServerListMsg){
        saveListWidgetToFile(ui->listWidget_UDPServerListMsg);
    }
}

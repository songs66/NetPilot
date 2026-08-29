#include "formudpclient.h"
#include "ui_formudpclient.h"

#include <QApplication>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

// 构造函数：初始化UI，套接字指针、样式、默认文本，从QSettings恢复上次IP/端口

FormUdpClient::FormUdpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormUdpClient)
{
    ui->setupUi(this);


    // IP下接框控件为可编辑，即可从列表选择历史IP，也可以手动输入新IP
    ui->comboBox_UDPClientIp->setEditable(true);

    // 固定窗口大小 ，禁止用户拖拽改变尺寸，保持界面稳定
    setFixedSize(width(),height());

    // 发送数据编辑框控件预设默认文本，便于快速测试
    ui->plainTextEdit_UDPClientSendData->setPlainText("Hello Udp Server.");

    // 日志列表设为只读，避免用户误改，并显示提示文字引导操作
    ui->plainTextEdit_UDPClientMsg->setReadOnly(true);
    ui->plainTextEdit_UDPClientMsg->setPlainText("Prompt: Please enter data and click to send test.");

    // 为各大控件设置固定尺寸策略，避免布局随内容变化产生抖动
    ui->groupBox_Left->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->groupBox_Right->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->plainTextEdit_UDPClientMsg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->label_1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->label_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->label_3->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->comboBox_UDPClientIp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->spinBox_UDPClientPort->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->plainTextEdit_UDPClientSendData->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->pushButton_UDPClientSendMsg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 设置整体界面样式：背景色、分组框边框与标题、标签、编辑框、按钮等等
    setStyleSheet(
        "QWidget {"
        "  background-color: #F3F4F6;"
        "  color: #1F2933;"
        "}"
        "QGroupBox {"
        "  font-size: 14px;"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #D0D7E2;"
        "  border-radius: 6px;"
        "  margin-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 4px;"
        "  background-color: #F3F4F6;"
        "  color: #111827;"
        "}"
        "QLabel {"
        "  font-size: 14px;"
        "  color: #374151;"
        "  background-color: transparent;"
        "}"
        "QPlainTextEdit {"
        "  font-size: 14px;"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #D0D7E2;"
        "  border-radius: 4px;"
        "}"
        "QComboBox, QSpinBox {"
        "  font-size: 14px;"
        "  background-color: #FFFFFF;"
        "  border: 1px solid #D0D7E2;"
        "  border-radius: 4px;"
        "  padding: 2px 6px;"
        "}"
        "QPushButton {"
        "font-size: 14px;"
        "background-color: #2563EB;"
        "color: #FFFFFF;"
        "border-radius: 4px;"
        "padding: 4px 12px;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: #1D4ED8;"
        "}"
        "QPushButton:pressed {"
        "background-color: #1E40AF;"
        "}"
        "QPushButton:disabled {"
        "background-color: #CBD5F5;"
        "color: #6B7280;"
        "}"
        );

    // 设置发送按钮上图标的显示尺寸
    ui->pushButton_UDPClientSendMsg->setIconSize(QSize(16,16));

    // DPI设置已在main.cpp中统一配置，此处不再重复设置

    // 从QSettings恢复上次使用的IP和端口，便于用户连续使用同一目标
    {
        QSettings settings;
        const QString lastIp = settings.value("UDPClient/lastIp", "127.0.0.1").toString();
        const int lastPort = settings.value("UDPClient/lastPort", ui->spinBox_UDPClientPort->value()).toInt();

        // 若下拉框控件中已有该IP则选中对应项，否则也可以编辑方式输入lastIp
        int index=ui->comboBox_UDPClientIp->findText(lastIp);
        if(index>=0){
            ui->comboBox_UDPClientIp->setCurrentIndex(index);
        }        else{
            ui->comboBox_UDPClientIp->setEditText(lastIp);
        }

        // 端口在控件允许范围内时才写入，避免非法值
        if(lastPort>=ui->spinBox_UDPClientPort->minimum() &&
            lastPort<=ui->spinBox_UDPClientPort->maximum()){
            ui->spinBox_UDPClientPort->setValue(lastPort);
        }
    }

}

FormUdpClient::~FormUdpClient()
{
    delete ui;
}

// 发送按钮槽函数：懒创建UDP套接字并连接readyread；校验IP/端口/发送内容，编码为UTF-8发数据报
// 保存本次IP/端口到QSettings；在日志框追加发送记录并执行trimLog。
void FormUdpClient::on_pushButton_UDPClientSendMsg_clicked()
{
    // 生成当前时间戳，用于在日志中标记发送时间
    QString timestamp=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

    // 懒创建：首次点击发送时才创建UDP套接字，并连接readyRead到接收槽
    if(!UDPClientSocket){
        UDPClientSocket=new QUdpSocket(this);
        connect(UDPClientSocket,&QUdpSocket::readyRead,this,&FormUdpClient::ReadServerDatagramFunc);
        socketReady=true;
    }

    // 从界面控件读取用户输入的服务器IP与端口
    QString strIpAddress=ui->comboBox_UDPClientIp->currentText();
    int port=ui->spinBox_UDPClientPort->value();

    // 使用InputValidator校验IP/主机名格式，非法则弹窗并返回
    auto ipValidation = InputValidator::validatorNetworkAddress(strIpAddress);
    if (!ipValidation.isValid) {
        HANDLE_ERROR(ErrorHandler::ValidationError, ErrorHandler::Warning, ipValidation.errorMessage, this);
        return;
    }

    // 校验端口必须在1–65535之间
    if (port < 1 || port > 65535) {
        HANDLE_ERROR(ErrorHandler::ValidationError, ErrorHandler::Warning, "端口号必须在1-65535范围内", this);
        return;
    }

    // 将字符串转为QHostAddress，并要求非空且为IPv4
    QHostAddress hostAddress(strIpAddress);
    if (hostAddress.isNull() || hostAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        HANDLE_ERROR(ErrorHandler::ValidationError, ErrorHandler::Warning, "无效的IP地址", this);
        return;
    }

    // 获取发送框控件内容并去除首尾空白，空内容不允许 发送
    QString message=ui->plainTextEdit_UDPClientSendData->toPlainText().trimmed();
    if (message.isEmpty()) {
        HANDLE_ERROR(ErrorHandler::ValidationError, ErrorHandler::Warning, "发送内容不能为空！", this);
        return;
    }

    // 防御性检查：确保套接字已创建（理论上懒创建之后不应为空）
    if(!UDPClientSocket){
        HANDLE_ERROR(ErrorHandler::NetworkError, ErrorHandler::Warning, "UDP套接字未初始化", this);
        return;
    }

    // 将QString转为UTF-8字节数组，并调用writeDatagram发送到指定地址与端口
    QByteArray data = message.toUtf8();
    qint64 bytesWritten = UDPClientSocket->writeDatagram(data, hostAddress, port);
    if (bytesWritten == -1) {
        HANDLE_ERROR(ErrorHandler::NetworkError, ErrorHandler::Warning,
                     QString("发送失败: %1").arg(UDPClientSocket->errorString()), this);
        return;
    }

    // 将本次使用的IP和端口写入QSettings，下次打开界面时自动恢复
    {
        QSettings settings;
        settings.setValue("UDPClient/lastIp", strIpAddress);
        settings.setValue("UDPClient/lastPort", static_cast<int>(port));
        settings.sync();
    }

    // 在日志列表中追加一行：时间戳 + “Sending:” + 发送内容，然后执行日志裁剪
    ui->plainTextEdit_UDPClientMsg->appendPlainText(QString("\n%1\nSending:%2").arg(timestamp,message));

    trimLog();

}

// 日志裁剪
void FormUdpClient::trimLog(int keepBlocks,int trimStep)
{
    // 定义默认值常量：使用静态常量定义默认的保留行数和裁剪步长
    static const int kKeepDefault=1000;
    static const int kTrimDefault=200;

    // 确定实际保留行数：如果参数有效则使用参数值，否则使用默认值
    const int keep=keepBlocks>0?keepBlocks:kKeepDefault;
    const int step=trimStep>0?trimStep:kTrimDefault;

    // 获取日志多行文本框对应的QTextDocument及当前块数
    auto doc=ui->plainTextEdit_UDPClientMsg->document();
    int blocks=doc->blockCount();

    // 块数未超过保留数+步长则不裁剪，直接返回
    if(blocks<=keep+step){
        return;
    }

    // 计算需要从文档开关删除的块数
    int removeBlocks=blocks-keep;
    QTextCursor cursor(doc);

    // 将光标移到文档开头
    cursor.movePosition(QTextCursor::Start);

    // 从开头向下扩展选区
    for(int i=0;i<removeBlocks;i++){
        cursor.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor);
    }

    // 删除选中的文本，并再删除一个字符以去掉可能残留的换行，保持文档整洁性
    cursor.removeSelectedText();
    cursor.deleteChar();

}

// 窗口关闭时保存日志并接受关闭
void FormUdpClient::closeEvent(QCloseEvent *event)
{
    saveLog();

    event->accept();

}

// 将指定QPlainTextEdit的全部记录只在到应用数据目录下，追加模式，UTF-8
void FormUdpClient::savePlainTextEditToFile(QPlainTextEdit * plainTextEdit)
{
    if (!plainTextEdit) {
        qWarning() << "FormUdpClient::savePlainTextEditToFile: plainTextEdit is null";
        return;
    }

    // 获取配置文件所在目录（与QSettings配置文件同一目录）
    // 使用AppDataLocation的父目录，确保与INI文件在同一目录：%APPDATA%\NDATools
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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
    QDir().mkpath(appDataPath); // 确保目录存在

    // 在数据目录下拼接日志文件名
    QString fileName = QDir(appDataPath).filePath("UDPClientLogfile.txt");

    // 根据文件是否已存在决定以追加或覆盖方式打开
    bool fileExists = QFile::exists(fileName);
    QIODevice::OpenMode openMode = fileExists ?
                                       (QIODevice::Append | QIODevice::Text) :
                                       (QIODevice::WriteOnly | QIODevice::Text);


    QFile file(fileName);
    if (file.open(openMode)) {
        QTextStream out(&file);
        /* 设置输出流编码为UTF-8，Qt5与Qt6的API不同 */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8");
#else
        out.setEncoding(QStringConverter::Utf8);
#endif

        /* 追加模式时先写入换行、分隔符与当前时间戳，便于区分多次保存的内容 */
        if (fileExists) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << "\n" << "========== " << timestamp << " ==========" << endl;
#else
            out << "\n" << "========== " << timestamp << " ==========" << Qt::endl;
#endif
        }

        /* 将文本框全文写入文件，然后关闭文件 */
        out << plainTextEdit->toPlainText();
        file.close();
        qDebug() << "UDP客户端日志已保存至:" << fileName;
        if (this->isVisible()) {
            QMessageBox::information(this, "成功", QString("日志已保存至 %1").arg(fileName));
        }
    } else {
        qWarning() << "UDP客户端日志保存失败:" << file.errorString();
        if (this->isVisible()) {
            QMessageBox::critical(this, "错误", "保存失败: " + file.errorString());
        }
    }


}

// 保存当前界面日志到文件
void FormUdpClient::saveLog()
{
    if(ui && ui->plainTextEdit_UDPClientMsg){
        savePlainTextEditToFile(ui->plainTextEdit_UDPClientMsg);
    }

}

// 读取服务器返回的数据报并加载到日志框，由readyRead信号触发
void FormUdpClient::ReadServerDatagramFunc()
{
    // 如果套接字未创建或已释放，直接返回，避免空指针访问
    if(!UDPClientSocket){
        return;
    }

    // 循环处理缓冲区中所有等待读取的数据报，直到没有更多数据
    while(UDPClientSocket->hasPendingDatagrams()){
        // 获取当前队首数据报的大小，用于预分配缓冲区
        qint64 datagramSize = UDPClientSocket->pendingDatagramSize();
        if (datagramSize <= 0) {
            break;
        }

        // 分配足够大的QByteArray用于接收数据，并准备接收发送方的地址与端口（未使用）
        QByteArray data;
        data.resize(static_cast<int>(datagramSize));
        QHostAddress senderAddress;
        quint16 senderPort = 0;

        // 从套接字读取一个数据报到data，并可选地得到发送方地址与端口
        qint64 bytesRead = UDPClientSocket->readDatagram(data.data(), data.size(),
                                                         &senderAddress, &senderPort);
        if (bytesRead == -1) {
            qWarning() << "读取UDP数据报失败:" << UDPClientSocket->errorString();
            continue;
        }

        /* 若实际读取字节数小于预分配大小，缩小data避免末尾未初始化数据参与解码 */
        if (bytesRead < datagramSize) {
            data.resize(static_cast<int>(bytesRead));
        }

        /* 生成当前时间戳，并按UTF-8将数据解码为QString */
        QString timestamp = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        QString payload = QString::fromUtf8(data);

        /* 在日志列表中追加一行：时间戳 + “Receive:” + 接收内容 */
        ui->plainTextEdit_UDPClientMsg->appendPlainText(
            QString("\n%1\nReceive:%2").arg(timestamp, payload));
    }

    /* 接收一批数据后统一做一次日志裁剪，防止块数过多导致卡顿 */
    trimLog();

}


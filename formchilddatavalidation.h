/* formchilddatavalidation.h
 * 数据校验功能子窗口类声明
 *
 * 主要功能：
 * 1、数据校验：支持CRC16、CRC32、MD5、SHA1、SHA256多种校验算法
 * 2、异步处理：大数据量时自动使用异步计算，避免UI阻塞
 * 3、大小写切换：支持校验值结果的大小写格式切换
 * 4、日志记录：记录所有校验操作，支持保存到文件
 * 5、剪贴板操作：支持一键复制所有校验值到剪贴板
 *
 * 计算模式：
 * 1、同步模式：数据量 < 40000 字符，直接计算
 * 2、异步模式：数据量 >= 40000 字符，使用QtConcurrent异步计算
 * */

#ifndef FORMCHILDDATAVALIDATION_H
#define FORMCHILDDATAVALIDATION_H

#include <QWidget>

#include <QCloseEvent>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>

#include <QClipboard>
#include <QCheckBox>
#include <QFutureWatcher>
#include <QTimer>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QQueue>
#include <QFileInfo>
#include <QtConcurrent>
#include <QDebug>


#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>      // Qt6字符串编码转换器
#else
#include <QTextCodec>            // Qt5文本编码器
#endif

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")  // MSVC编译器：设置执行字符集为UTF-8
#endif

#include "dataconverter.h"      // 数据转换与校验核心工具类


namespace Ui {
class FormChildDataValidation;
}

class FormChildDataValidation : public QWidget
{
    Q_OBJECT  // Qt元对象系统宏，启用信号槽机制

public:
    // 构造函数
    explicit FormChildDataValidation(QWidget *parent = nullptr);

    // 析构函数
    ~FormChildDataValidation();

    // 保存日志到文件（public 接口，供主窗口调用）
    void saveLog();

private slots:
    void on_calculateButton_clicked();


private:
    // 连接信号和槽
    void connectSignals();

    // 复制文本到剪贴板
    void copyToClipboard(const QString &text);

    // 显示消息提示
    void showMessage(const QString &message, bool isError = false);

    // 设置校验计算忙碌状态
    void setChecksumBusy(bool busy);

    // 追加日志到日志队列
    void appendLog(const QString &message);

    // 校验计算是否正在运行的标志（true=正在计算，false=空闲）
    bool checksumRunning = false;

    // 异步校验计算任务监控器
    QFutureWatcher<DataConverter::ChecksumResult> checksumWatcher;

    // 日志条目队列（用于保存到文件）
    QQueue<QString> logEntries;

protected:
    // 窗口关闭事件处理函数
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    // 大小写复选框状态变化槽函数
    void onUpperCaseToggled(bool enabled);

    // 异步校验计算完成槽函数
    void onChecksumFinished();


private:
    Ui::FormChildDataValidation *ui;

};

#endif // FORMCHILDDATAVALIDATION_H

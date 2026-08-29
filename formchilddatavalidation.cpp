#include "formchilddatavalidation.h"
#include "ui_formchilddatavalidation.h"
#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>      // Qt6字符串编码转换器
#else
#include <QTextCodec>            // Qt5文本编码器
#endif

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")  // MSVC编译器：设置执行字符集为UTF-8
#endif

/*
主要功能：
1、调用父类QWidget构造函数，初始化窗口基础属性
2、创建并初始化UI界面（通ui->setupUi()）
3、连接信号和槽
4、设置输入框的占位符文本，提示用户输入
5、连接异步计算完成信号到槽函数，用于处理异步计算结果
6、初始化日志系统，记录工具启动信息
*/

// 最大校验值计算长度限制（200000 字符）
static const int kMaxChecksumLength = 200000;

// 异步校验值计算阈值（40000 字符，超过此值使用异步计算）
static const int kAsyncChecksumThreshold = 40000;


FormChildDataValidation::FormChildDataValidation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormChildDataValidation)
{
    ui->setupUi(this);

    // 连接所有信号和槽（复制按钮、大小写复选框）
    connectSignals();

    ui->checksumInputEdit->setPlaceholderText("请输入要计算校验值的数据...");

    // 连接异步计算监控器的完成信号到槽函数
    connect(&checksumWatcher, &QFutureWatcher<DataConverter::ChecksumResult>::finished,
            this, &FormChildDataValidation::onChecksumFinished);

    // 初始化日志系统：记录工具启动信息
    QString startMsg = QString("[%1] 数据校验工具已启动")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    appendLog(startMsg);


}

FormChildDataValidation::~FormChildDataValidation()
{

    delete ui;
}

void FormChildDataValidation::on_calculateButton_clicked()
{
    // 企业级标准：添加空指针检查，确保 UI 和所有控件都存在
    // 防止在 UI 未初始化或控件被删除时访问导致程序崩溃
    if (!ui || !ui->checksumInputEdit || !ui->upperCaseCheckBox ||
        !ui->crc16Edit || !ui->crc32Edit || !ui->md5Edit ||
        !ui->sha1Edit || !ui->sha256Edit) {
        qWarning() << "FormChildDataValidation::on_calculateButton_clicked: UI or controls is null";
        return;
    }

    // 防重复点击：如果正在计算，直接返回，避免重复计算
    if (checksumRunning) {
        return;
    }

    // 获取输入框中的数据
    QString input = ui->checksumInputEdit->toPlainText();

    // 输入验证：检查输入是否为空
    if (input.isEmpty()) {
        showMessage(QStringLiteral("请输入要计算校验值的数据"), true);
        return;
    }

    // 输入验证：检查输入长度是否超过最大限制
    if (input.length() > kMaxChecksumLength) {
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        showMessage(QStringLiteral("输入过长（>%1 字符），请分批计算。").arg(kMaxChecksumLength), true);
        return;
    }

    // 判断是否使用异步计算模式（数据量 >= 40000 字符）
    if (input.length() > kAsyncChecksumThreshold) {
        // 设置忙碌状态，禁用相关按钮，防止重复点击
        setChecksumBusy(true);

        // 性能优化：复用时间戳，避免重复创建 QDateTime 对象
        QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

        // 记录异步计算日志
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        QString logMsg = QStringLiteral("[%1] [数据校验] 开始计算校验值，输入长度: %2 字符，使用异步计算")
                             .arg(timestamp)
                             .arg(input.length());
        appendLog(logMsg);
        appendLog(QStringLiteral("[%1] [数据校验] 使用异步计算（输入长度超过阈值 %2）")
                      .arg(timestamp)
                      .arg(kAsyncChecksumThreshold));

        // 使用 QtConcurrent::run() 在后台线程执行校验计算
        // lambda 表达式捕获 input 的副本，确保线程安全
        auto future = QtConcurrent::run([input]() {
            return DataConverter::calculateChecksums(input);
        });

        // 设置要监控的异步任务，当任务完成时会触发 finished 信号
        checksumWatcher.setFuture(future);

        // 异步模式下，计算完成后会通过 onChecksumFinished() 处理结果
        return;
    }

    // 同步计算模式：数据量 < 40000 字符，直接在当前线程计算

    // 性能优化：复用时间戳，避免重复创建 QDateTime 对象
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    // 记录校验操作日志
    // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
    QString logMsg = QStringLiteral("[%1] [数据校验] 开始计算校验值，输入长度: %2 字符")
                         .arg(timestamp)
                         .arg(input.length());
    appendLog(logMsg);

    // 注意：此处的判断条件永远不会为真（因为前面已经 return），保留用于代码一致性
    if (input.length() > kAsyncChecksumThreshold) {
        appendLog(QStringLiteral("[%1] [数据校验] 使用异步计算（输入长度超过阈值 %2）")
                      .arg(timestamp)
                      .arg(kAsyncChecksumThreshold));
    }

    // 调用 DataConverter 计算所有校验值（CRC16、CRC32、MD5、SHA1、SHA256）
    // 这是同步调用，会阻塞当前线程直到计算完成
    DataConverter::ChecksumResult result = DataConverter::calculateChecksums(input);

    // 获取大小写设置状态
    bool isUpperCase = ui->upperCaseCheckBox->isChecked();

    // 根据大小写设置格式化并显示所有校验值结果
    // 如果选中大写，直接使用结果（DataConverter 返回的是大写）
    // 如果未选中大写，转换为小写显示
    ui->crc16Edit->setText(isUpperCase ? result.crc16 : result.crc16.toLower());
    ui->crc32Edit->setText(isUpperCase ? result.crc32 : result.crc32.toLower());
    ui->md5Edit->setText(isUpperCase ? result.md5 : result.md5.toLower());
    ui->sha1Edit->setText(isUpperCase ? result.sha1 : result.sha1.toLower());
    ui->sha256Edit->setText(isUpperCase ? result.sha256 : result.sha256.toLower());

    // 显示计算完成提示（通过 qDebug 输出，不显示消息框）
    showMessage(QStringLiteral("校验值计算完成"));

    // 记录校验完成日志
    // 性能优化：复用时间戳，使用 QStringLiteral
    logMsg = QStringLiteral("[%1] [数据校验] 校验值计算完成").arg(timestamp);
    appendLog(logMsg);

}

// 保存日志到文件（public 接口，供主窗口调用）
void FormChildDataValidation::saveLog()
{
    // 如果日志队列为空，直接返回，不执行任何操作
    if (logEntries.isEmpty()) {
        return; // 没有日志需要保存
    }

    // 获取应用数据目录路径（与QSettings配置文件同一目录）
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

    // 确保目录存在，如果不存在则创建
    if (!QDir().mkpath(appDataPath)) {
        qWarning() << "FormChildDataValidation::saveLog: 无法创建目录" << appDataPath;
        return;
    }

    // 使用固定文件名，追加模式（与数据转换使用同一个文件）
    QString fileName = QDir(appDataPath).filePath(QStringLiteral("DataConversionLogfile.txt"));

    // 检查文件是否存在，决定是追加还是创建
    bool fileExists = QFile::exists(fileName);

    // 根据文件是否存在选择打开模式
    QIODevice::OpenMode openMode = fileExists ?
                                       (QIODevice::Append | QIODevice::Text) :
                                       (QIODevice::WriteOnly | QIODevice::Text);

    // 打开文件
    QFile file(fileName);
    if (file.open(openMode)) {
        // 创建文本流，用于写入文件
        QTextStream out(&file);

        // 设置文件编码为UTF-8（Qt5和Qt6的API不同）
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8"); // Qt5设置编码
#else
        out.setEncoding(QStringConverter::Utf8); // Qt6设置编码
#endif

        // 如果是追加模式，先写入分隔符和时间戳
        // 用于区分不同时间段的日志记录
        if (fileExists) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

            // 写入分隔符和时间戳（Qt5 和 Qt6 的 endl 不同）
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << "\n" << QStringLiteral("========== ") << timestamp << QStringLiteral(" ==========") << endl;
#else
            out << "\n" << QStringLiteral("========== ") << timestamp << QStringLiteral(" ==========") << Qt::endl;
#endif
        }

        // 性能优化：批量写入所有日志条目
        // QQueue 可以像容器一样遍历，使用范围 for 循环
        for (const QString &entry : logEntries) {
            // 写入日志条目
            out << entry;

            // 写入换行符（Qt5 和 Qt6 的 endl 不同）
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << endl;
#else
            out << Qt::endl;
#endif
        }

        // 关闭文件
        file.close();

        // 输出保存成功信息到调试控制台
        qDebug() << "数据校验工具日志已保存至:" << fileName;

        // 清空已保存的日志条目，释放内存
        logEntries.clear();

        // 注意：窗口关闭时不显示消息框，避免阻塞关闭流程
        // 仅在窗口可见时显示成功提示
        if (this->isVisible()) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("日志已保存至 %1").arg(fileName));
        }
    } else {
        // 文件打开失败，记录警告日志
        qWarning() << "数据校验工具日志保存失败:" << file.errorString();

        // 仅在窗口可见时显示错误提示
        if (this->isVisible()) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("保存失败: ") + file.errorString());
        }
    }

}

// 连接信号和槽
void FormChildDataValidation::connectSignals()
{
    if (!ui || !ui->copyHashButton || !ui->upperCaseCheckBox ||
        !ui->crc16Edit || !ui->crc32Edit || !ui->md5Edit ||
        !ui->sha1Edit || !ui->sha256Edit) {
        qWarning() << "FormChildDataValidation::connectSignals: UI or controls is null";
        return;
    }

    // 连接复制按钮的点击信号到lambda表达式
    connect(ui->copyHashButton, &QPushButton::clicked, [this]() {
        // 企业级标准：在 lambda 内部再次检查 UI 控件，确保安全访问
        if (!ui || !ui->crc16Edit || !ui->crc32Edit || !ui->md5Edit ||
            !ui->sha1Edit || !ui->sha256Edit) {
            qWarning() << "FormChildDataValidation::copyHashButton clicked: UI or controls is null";
            return;
        }

        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        // 将所有校验值格式化为多行文本，便于复制和查看
        QString allHashes = QStringLiteral("CRC16: %1\nCRC32: %2\nMD5: %3\nSHA1: %4\nSHA256: %5")
                                .arg(ui->crc16Edit->text())      // CRC16 校验值
                                .arg(ui->crc32Edit->text())  // CRC32 校验值
                                .arg(ui->md5Edit->text())    // MD5 哈希值
                                .arg(ui->sha1Edit->text())   // SHA1 哈希值
                                .arg(ui->sha256Edit->text()); // SHA256 哈希值

        // 调用复制函数，将格式化后的文本复制到剪贴板
        copyToClipboard(allHashes);
    });

    // 连接大小写复选框的状态变化信号到槽函数
    connect(ui->upperCaseCheckBox, &QCheckBox::toggled, this, &FormChildDataValidation::onUpperCaseToggled);
}

// 复制文本到剪贴板
void FormChildDataValidation::copyToClipboard(const QString &text)
{
    // 获取系统剪贴板对象（单例模式，全局唯一）
    QClipboard *clipboard = QApplication::clipboard();

    // 将文本复制到剪贴板
    clipboard->setText(text);

    // 显示复制成功提示（通过 qDebug 输出到调试控制台，不显示消息框）
    showMessage(QStringLiteral("已复制到剪贴板"));

    // 记录复制操作日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据校验] 复制校验值到剪贴板").arg(timestamp);
    appendLog(logMsg);

}

// 显示消息提示
void FormChildDataValidation::showMessage(const QString &message, bool isError)
{
    if (isError) {
        // 错误消息：显示警告对话框，阻塞用户操作直到关闭
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        QMessageBox::warning(this, QStringLiteral("错误"), message);
    } else {
        // 普通消息：仅输出到调试控制台，不阻塞用户操作
        qDebug() << message;
    }

}

// 设置校验计算忙碌状态
void FormChildDataValidation::setChecksumBusy(bool busy)
{
    // 设置校验计算运行状态标志
    checksumRunning = busy;

    // 根据忙碌状态启用/禁用计算按钮
    // busy=true 时禁用，busy=false 时启用
    ui->calculateButton->setEnabled(!busy);

    // 根据忙碌状态启用/禁用复制按钮
    // busy=true 时禁用，busy=false 时启用
    ui->copyHashButton->setEnabled(!busy);
}

// 追加日志到日志队列
void FormChildDataValidation::appendLog(const QString &message)
{
    logEntries.enqueue(message);

    qDebug() << "数据校验工具日志:" << message;

    // 性能优化：防止日志队列过长，超过 1000 条时删除最旧的条目

    while (logEntries.size() > 1000) {
        logEntries.dequeue();
    }
}

// 窗口关闭事件处理函数
void FormChildDataValidation::closeEvent(QCloseEvent *event)
{
    saveLog();

    event->accept();

}

// 大小写复选框状态变化槽函数
void FormChildDataValidation::onUpperCaseToggled(bool enabled)
{
    // 企业级标准：添加空指针检查，确保 UI 和所有控件都存在
    if (!ui || !ui->crc16Edit || !ui->crc32Edit || !ui->md5Edit ||
        !ui->sha1Edit || !ui->sha256Edit) {
        qWarning() << "FormChildDataValidation::onUpperCaseToggled: UI or controls is null";
        return;
    }

    // 仅在已有校验值结果时执行转换（检查 CRC16 是否为空作为判断条件）
    // 如果所有校验值都为空，不执行任何操作
    if (!ui->crc16Edit->text().isEmpty()) {
        if (enabled) {
            // 复选框选中：将所有校验值转换为大写
            ui->crc16Edit->setText(ui->crc16Edit->text().toUpper());
            ui->crc32Edit->setText(ui->crc32Edit->text().toUpper());
            ui->md5Edit->setText(ui->md5Edit->text().toUpper());
            ui->sha1Edit->setText(ui->sha1Edit->text().toUpper());
            ui->sha256Edit->setText(ui->sha256Edit->text().toUpper());
        } else {
            // 复选框取消选中：将所有校验值转换为小写
            ui->crc16Edit->setText(ui->crc16Edit->text().toLower());
            ui->crc32Edit->setText(ui->crc32Edit->text().toLower());
            ui->md5Edit->setText(ui->md5Edit->text().toLower());
            ui->sha1Edit->setText(ui->sha1Edit->text().toLower());
            ui->sha256Edit->setText(ui->sha256Edit->text().toLower());
        }
    }

}

// 异步校验计算完成槽函数
void FormChildDataValidation::onChecksumFinished()
{
    // 企业级标准：添加空指针检查，确保 UI 和所有控件都存在
    if (!ui || !ui->upperCaseCheckBox || !ui->crc16Edit || !ui->crc32Edit ||
        !ui->md5Edit || !ui->sha1Edit || !ui->sha256Edit) {
        qWarning() << "FormChildDataValidation::onChecksumFinished: UI or controls is null";
        // 即使 UI 控件为空，也要解除忙碌状态，避免按钮永久禁用
        setChecksumBusy(false);
        return;
    }

    // 解除忙碌状态，启用计算按钮和复制按钮
    setChecksumBusy(false);

    // 获取异步计算结果（从 QFutureWatcher 中获取）
    // result() 方法会等待异步任务完成并返回结果
    auto result = checksumWatcher.result();

    // 获取大小写设置状态
    bool isUpperCase = ui->upperCaseCheckBox->isChecked();

    // 根据大小写设置格式化并显示所有校验值结果
    // 如果选中大写，直接使用结果（DataConverter 返回的是大写）
    // 如果未选中大写，转换为小写显示
    ui->crc16Edit->setText(isUpperCase ? result.crc16 : result.crc16.toLower());
    ui->crc32Edit->setText(isUpperCase ? result.crc32 : result.crc32.toLower());
    ui->md5Edit->setText(isUpperCase ? result.md5 : result.md5.toLower());
    ui->sha1Edit->setText(isUpperCase ? result.sha1 : result.sha1.toLower());
    ui->sha256Edit->setText(isUpperCase ? result.sha256 : result.sha256.toLower());

    // 显示计算完成提示（通过 qDebug 输出，不显示消息框）
    showMessage(QStringLiteral("校验值计算完成"));

    // 记录异步校验完成日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据校验] 异步校验值计算完成").arg(timestamp);
    appendLog(logMsg);

}




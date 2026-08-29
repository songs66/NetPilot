#include "formchilddataconversion.h"
#include "ui_formchilddataconversion.h"
#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>      // Qt6字符串编码转换器
#else
#include <QTextCodec>            // Qt5文本编码器
#endif

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")  // MSVC编译器：设置执行字符集为UTF-8
#endif

// 最大转换长度限制
static const int kMaxConvertLength = 50000;

// 异步转换阈值
static const int kAsyncConvertThreshold = 15000;


FormChildDataConversion::FormChildDataConversion(QWidget *parent)
    : QWidget(parent)
    ,inputUpdateTimer(new QTimer(this))  // 创建防抖定时器，父对象为this
    , ui(new Ui::FormChildDataConversion)
{
    ui->setupUi(this);

     // 步骤1：
    // 初始化转换类型下拉框控件
    ui->conversionTypeCombo->addItem("十进制 -> 二进制","decimal_to_binary");
    ui->conversionTypeCombo->addItem("二进制 -> 十进制","binary_to_decimal");

    // 步骤2：配置输入更新防抖定时器
    inputUpdateTimer->setSingleShot(true);               // 设置为单次触发模式
    inputUpdateTimer->setInterval(300);                  // 设置延迟时间为 300ms

    // 连接定时器的 timeout 信号到 lambda 表达式
    connect(inputUpdateTimer, &QTimer::timeout, this, [this]() {
        QString text = ui->inputTextEdit->toPlainText(); // 获取输入文本
        updateInputInfo(text);                           // 更新输入统计信息
    });

    // 连接 UI 控件的信号到对应的槽函数
    connectSignals();

    // 连接异步转换监视器的 finished 信号到完成处理槽函数
    connect(&conversionWatcher, &QFutureWatcher<DataConverter::ConversionResult>::finished,
            this, &FormChildDataConversion::onConversionFinished);

    // 提示用户输入和输出的位置
    ui->inputTextEdit->setPlaceholderText("请输入要转换的数据...");
    ui->outputTextEdit->setPlaceholderText("转换结果将显示在这里...");

    // 初始化日志：记录启动信息
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString startMsg = QStringLiteral("[%1] 数据转换工具已启动").arg(timestamp);
    appendLog(startMsg);

}

FormChildDataConversion::~FormChildDataConversion()
{
    delete ui;
}

void FormChildDataConversion::on_convertButton_clicked()
{
    // 检查 UI 对象和关键控件是否存在，防止访问空指针
    if (!ui || !ui->inputTextEdit || !ui->outputTextEdit || !ui->conversionTypeCombo) {
        qWarning() << "FormChildDataConversion::on_convertButton_clicked: UI or controls is null";
        return;
    }

    // 防止用户重复点击转换按钮，导致重复操作
    if (conversionRunning) {
        return;
    }

    // 步骤3：获取输入数据
    QString input = ui->inputTextEdit->toPlainText();

    // 步骤4：输入验证 - 检查输入是否为空
    if (input.isEmpty()) {
        showMessage(QStringLiteral("请输入要转换的数据"), true);
        return;
    }

    // 步骤5：输入验证 - 检查输入长度是否超过最大限制
    if (input.length() > kMaxConvertLength) {
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        showMessage(QStringLiteral("输入过长（>%1 字符），请分批转换。").arg(kMaxConvertLength), true);
        return;
    }

    // 步骤6：获取转换类型
    // currentData() 返回选项的数据值（"decimal_to_binary" 或 "binary_to_decimal"）
    QString conversionType = ui->conversionTypeCombo->currentData().toString();
    // currentText() 返回选项的显示文本（"十进制 → 二进制" 或 "二进制 → 十进制"）
    QString conversionTypeName = ui->conversionTypeCombo->currentText();

    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据转换] 开始转换: %2, 输入长度: %3 字符")
                         .arg(timestamp, conversionTypeName)
                         .arg(input.length());

    appendLog(logMsg);

    if (input.length() > kAsyncConvertThreshold) {
        // 异步转换模式：数据量 >= 15000 字符
        // 设置转换忙状态，禁用相关按钮
        setConversionBusy(true);

        // 记录异步转换日志
        appendLog(QStringLiteral("[%1] [数据转换] 使用异步转换（输入长度超过阈值 %2）")
                      .arg(timestamp)
                      .arg(kAsyncConvertThreshold));

        // 使用 QtConcurrent::run() 在后台线程执行转换
        // lambda 表达式捕获输入数据和转换类型，在后台线程中执行转换
        auto future = QtConcurrent::run([input, conversionType]() {
            DataConverter::ConversionResult result;
            // 根据转换类型调用相应的转换方法
            if (conversionType == "decimal_to_binary") {
                result = DataConverter::decimalToBinary(input);
            } else if (conversionType == "binary_to_decimal") {
                result = DataConverter::binaryToDecimal(input);
            }
            return result;
        });

        // 设置要监视的 Future，当转换完成时会触发 finished 信号
        conversionWatcher.setFuture(future);
        return;  // 异步转换立即返回，不等待结果
    }

    // 同步转换模式：数据量 < 15000 字符
    // 直接调用转换方法，立即获取结果
    DataConverter::ConversionResult result;
    if (conversionType == "decimal_to_binary") {
        result = DataConverter::decimalToBinary(input);
    } else if (conversionType == "binary_to_decimal") {
        result = DataConverter::binaryToDecimal(input);
    }

    // 显示转换结果
    showResult(result, ui->outputTextEdit);

}


void FormChildDataConversion::on_clearButton_clicked()
{
    if (!ui || !ui->inputTextEdit || !ui->outputTextEdit) {
        qWarning() << "FormChildDataConversion::on_clearButton_clicked: UI or controls is null";
        return;
    }

    // 步骤2：清空输入和输出文本框
    ui->inputTextEdit->clear();
    ui->outputTextEdit->clear();

    // 步骤3：更新统计信息（设置为空字符串）
    updateInputInfo("");
    updateOutputInfo("");

    // 步骤4：记录清空操作日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据转换] 清空数据").arg(timestamp);
    appendLog(logMsg);

}


void FormChildDataConversion::on_swapButton_clicked()
{
    if (!ui || !ui->inputTextEdit || !ui->outputTextEdit) {
        qWarning() << "FormChildDataConversion::on_swapButton_clicked: UI or controls is null";
        return;
    }

    // 步骤2：获取输入和输出文本框的内容
    QString inputText = ui->inputTextEdit->toPlainText();
    QString outputText = ui->outputTextEdit->toPlainText();

    // 步骤3：交换输入和输出文本框的内容
    ui->inputTextEdit->setPlainText(outputText);
    ui->outputTextEdit->setPlainText(inputText);

    // 步骤4：更新统计信息
    // 输入统计使用原输出内容，输出统计使用原输入内容
    updateInputInfo(outputText);
    updateOutputInfo(inputText);

    // 步骤5：记录交换操作日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据转换] 交换输入输出数据").arg(timestamp);
    appendLog(logMsg);

}


void FormChildDataConversion::on_copyButton_clicked()
{
    if (!ui || !ui->outputTextEdit) {
        qWarning() << "FormChildDataConversion::on_copyButton_clicked: UI or outputTextEdit is null";
        return;
    }

    // 步骤2：获取输出文本框的内容
    QString text = ui->outputTextEdit->toPlainText();

    // 步骤3：检查内容是否为空
    if (text.isEmpty()) {
        showMessage(QStringLiteral("没有可复制的内容"), true);
        return;
    }

    // 步骤4：复制到剪贴板
    copyToClipboard(text);
}



// 保存日志（公共接口，提供主窗口调用）
void FormChildDataConversion::saveLog()
{
    // 步骤1：检查日志队列是否为空
    if (logEntries.isEmpty()) {
        return; // 没有日志需要保存
    }

    // 步骤2：获取应用程序数据目录（与QSettings配置文件同一目录）
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

    // 步骤3：创建目录（如果不存在）
    if (!QDir().mkpath(appDataPath)) {
        qWarning() << "FormChildDataConversion::saveLog: 无法创建目录" << appDataPath;
        return;
    }

    // 步骤4：构建文件路径
    // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
    QString fileName = QDir(appDataPath).filePath(QStringLiteral("DataConversionLogfile.txt"));

    // 步骤5：检查文件是否存在，决定打开模式
    bool fileExists = QFile::exists(fileName);
    QIODevice::OpenMode openMode = fileExists ?
                                       (QIODevice::Append | QIODevice::Text) :    // 追加模式（文件存在）
                                       (QIODevice::WriteOnly | QIODevice::Text);  // 创建模式（文件不存在）

    // 步骤6：打开文件
    QFile file(fileName);
    if (file.open(openMode)) {
        QTextStream out(&file);

        // 设置文件编码为 UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8"); // Qt5 设置编码
#else
        out.setEncoding(QStringConverter::Utf8); // Qt6 设置编码
#endif

        // 步骤7：如果是追加模式，先写入分隔符和时间戳
        if (fileExists) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << "\n" << QStringLiteral("========== ") << timestamp << QStringLiteral(" ==========") << endl;
#else
            out << "\n" << QStringLiteral("========== ") << timestamp << QStringLiteral(" ==========") << Qt::endl;
#endif
        }

        // 步骤8：性能优化：批量写入所有日志条目
        // QQueue 可以像容器一样遍历，O(n) 时间复杂度
        for (const QString &entry : logEntries) {
            out << entry;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << endl;
#else
            out << Qt::endl;
#endif
        }

        // 步骤9：关闭文件
        file.close();
        qDebug() << "数据转换工具日志已保存至:" << fileName;

        // 步骤10：清空已保存的日志条目
        logEntries.clear();

        // 步骤11：注意：窗口关闭时不显示消息框，避免阻塞
        if (this->isVisible()) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("日志已保存至 %1").arg(fileName));
        }
    } else {
        // 文件打开失败，记录警告并显示错误消息框
        qWarning() << "数据转换工具日志保存失败:" << file.errorString();
        if (this->isVisible()) {
            // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
            QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("保存失败: ") + file.errorString());
        }
    }

}

// 异步转换完成槽函数
void FormChildDataConversion::onConversionFinished()
{
    // 步骤1：企业级标准：添加空指针检查
    if (!ui || !ui->outputTextEdit) {
        qWarning() << "FormChildDataConversion::onConversionFinished: UI or outputTextEdit is null";
        // 即使出错也要恢复 UI 状态，启用相关按钮
        setConversionBusy(false);
        return;
    }

    // 步骤2：设置转换忙状态为 false（启用相关按钮）
    setConversionBusy(false);

    // 步骤3：获取异步转换结果
    // conversionWatcher.result() 会等待异步任务完成并返回结果
    auto result = conversionWatcher.result();

    // 步骤4：记录异步转换完成日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据转换] 异步转换完成").arg(timestamp);
    appendLog(logMsg);

    // 步骤5：显示转换结果
    showResult(result, ui->outputTextEdit);
}


// 输入文本变化槽函数
void FormChildDataConversion::onInputTextChanged()
{
    // 步骤1：企业级标准：添加空指针检查
    if (!inputUpdateTimer) {
        qWarning() << "FormChildDataConversion::onInputTextChanged: inputUpdateTimer is null";
        return;
    }

    // 步骤2：性能优化：使用防抖定时器，避免每次输入都更新统计信息
    // 停止当前的防抖定时器（如果正在运行）
    inputUpdateTimer->stop();

    inputUpdateTimer->start();
}


// 统一消息提示函数
void FormChildDataConversion::showMessage(const QString &message, bool isError)
{
    if (isError) {
        // 错误消息：显示消息框
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        QMessageBox::warning(this, QStringLiteral("错误"), message);
    } else {
        // 普通消息：输出到调试控制台
        qDebug() << message;
    }
}


// 设置转换忙状态
void FormChildDataConversion::setConversionBusy(bool busy)
{
    // 设置转换运行状态标志
    conversionRunning = busy;

    // 根据 busy 状态启用/禁用相关按钮
    // !busy 表示：如果 busy 为 true，则 !busy 为 false，按钮被禁用
    //            如果 busy 为 false，则 !busy 为 true，按钮被启用
    ui->convertButton->setEnabled(!busy);
    ui->clearButton->setEnabled(!busy);
    ui->swapButton->setEnabled(!busy);
    ui->copyButton->setEnabled(!busy);
}

// 展示转换结果或错误信息
void FormChildDataConversion::showResult(const DataConverter::ConversionResult &result, QTextEdit *outputEdit)
{
    // 步骤1：企业级标准：添加空指针检查
    if (!outputEdit) {
        qWarning() << "FormChildDataConversion::showResult: outputEdit is null";
        return;
    }

    // 步骤2：性能优化：复用时间戳，避免重复创建 QDateTime 对象
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    if (result.success) {
        // 转换成功：显示结果，更新统计，显示成功提示，记录成功日志
        outputEdit->setPlainText(result.data);           // 显示转换结果
        updateOutputInfo(result.data);                   // 更新输出统计信息
        showMessage(QStringLiteral("转换成功"));        // 显示成功提示

        // 记录转换成功日志
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        QString logMsg = QStringLiteral("[%1] [数据转换] 转换成功，输出长度: %2 字符")
                             .arg(timestamp)
                             .arg(result.data.length());
        appendLog(logMsg);
    } else {
        // 转换失败：清空输出，更新统计，显示错误提示，记录失败日志
        outputEdit->setPlainText(QStringLiteral(""));   // 清空输出文本框
        updateOutputInfo(QStringLiteral(""));           // 更新输出统计信息（设置为空）
        showMessage(result.errorMessage, true);          // 显示错误提示

        // 记录转换失败日志
        // 性能优化：使用 QStringLiteral 避免临时字符串对象分配
        QString logMsg = QStringLiteral("[%1] [数据转换] 转换失败: %2")
                             .arg(timestamp)
                             .arg(result.errorMessage);
        appendLog(logMsg);
    }

}

// 更新输入统计信息
void FormChildDataConversion::updateInputInfo(const QString &text)
{
    // 步骤1：企业级标准：添加空指针检查
    if (!ui || !ui->inputInfoLabel) {
        return;
    }

    // 步骤2：获取统计信息并更新标签
    // DataConverter::getDataInfo() 返回格式："字符数: X, 字节数: Y"
    QString info = DataConverter::getDataInfo(text);
    ui->inputInfoLabel->setText(info);

}

// 更新输出统计信息
void FormChildDataConversion::updateOutputInfo(const QString &text)
{
    QString info = DataConverter::getDataInfo(text);
    ui->outputInfoLabel->setText(info);
}

// 复制文本到剪贴板
void FormChildDataConversion::copyToClipboard(const QString &text)
{
    // 步骤1：获取系统剪贴板对象
    QClipboard *clipboard = QApplication::clipboard();

    // 步骤2：将文本设置到剪贴板
    clipboard->setText(text);

    // 步骤3：显示复制成功提示
    showMessage(QStringLiteral("已复制到剪贴板"));

    // 步骤4：记录复制操作日志
    // 性能优化：使用 QStringLiteral 和时间戳复用
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    QString logMsg = QStringLiteral("[%1] [数据转换] 复制结果到剪贴板，长度: %2 字符")
                         .arg(timestamp)
                         .arg(text.length());
    appendLog(logMsg);

}

// 连接所有信号和槽
void FormChildDataConversion::connectSignals()
{
    // 步骤1：企业级标准：添加空指针检查
    if (!ui || !ui->inputTextEdit) {
        qWarning() << "FormChildDataConversion::connectSignals: UI or inputTextEdit is null";
        return;
    }

    // 步骤2：连接输入文本变化的信号到防抖处理函数
    connect(ui->inputTextEdit, &QTextEdit::textChanged, this, &FormChildDataConversion::onInputTextChanged);
}

// 追加日志条目
void FormChildDataConversion::appendLog(const QString &message)
{
    // 将日志消息添加到日志队列的末尾（FIFO 结构）
    logEntries.enqueue(message);

    qDebug() << "数据转换工具日志:" << message;

    // 保持日志队列在合理的大小范围内，防止内存占用过大
    while (logEntries.size() > 1000) {
        logEntries.dequeue();  // 删除最旧的条目（队列头部）
    }
}

// 裁剪日志队列
void FormChildDataConversion::trimLog()
{
    // UI日志显示已删除，不需要裁剪
}

// 保存纯文本编辑框内容到文件
void FormChildDataConversion::savePlainTextEditToFile(QPlainTextEdit* plainTextEdit)
{
    // 步骤1：空指针检查
    if (!plainTextEdit) {
        qWarning() << "FormChildDataConversion::savePlainTextEditToFile: plainTextEdit is null";
        return;
    }

    // 步骤2：获取应用程序数据目录（与QSettings配置文件同一目录）
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

    // 步骤3：创建目录（如果不存在）
    if (!QDir().mkpath(appDataPath)) {
        qWarning() << "FormChildDataConversion::savePlainTextEditToFile: 无法创建目录" << appDataPath;
        return;
    }

    // 步骤4：构建文件路径
    QString fileName = QDir(appDataPath).filePath("DataConversionLogfile.txt");

    // 步骤5：检查文件是否存在，决定打开模式
    bool fileExists = QFile::exists(fileName);
    QIODevice::OpenMode openMode = fileExists ?
                                       (QIODevice::Append | QIODevice::Text) :    // 追加模式（文件存在）
                                       (QIODevice::WriteOnly | QIODevice::Text);  // 创建模式（文件不存在）

    // 步骤6：打开文件
    QFile file(fileName);
    if (file.open(openMode)) {
        QTextStream out(&file);

        // 设置文件编码为 UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8"); // Qt5 设置编码
#else
        out.setEncoding(QStringConverter::Utf8); // Qt6 设置编码
#endif

        // 步骤7：如果是追加模式，先写入分隔符和时间戳
        if (fileExists) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            out << "\n" << "========== " << timestamp << " ==========" << endl;
#else
            out << "\n" << "========== " << timestamp << " ==========" << Qt::endl;
#endif
        }

        // 步骤8：性能优化：批量写入文本内容，减少I/O操作
        QString content = plainTextEdit->toPlainText();
        if (!content.isEmpty()) {
            out << content;
        }

        // 步骤9：关闭文件
        file.close();
        qDebug() << "数据转换工具日志已保存至:" << fileName;

        // 注意：窗口关闭时不显示消息框，避免阻塞
        if (this->isVisible()) {
            QMessageBox::information(this, "成功", QString("日志已保存至 %1").arg(fileName));
        }
    } else {
        // 文件打开失败，记录警告并显示错误消息框
        qWarning() << "数据转换工具日志保存失败:" << file.errorString();
        if (this->isVisible()) {
            QMessageBox::critical(this, "错误", "保存失败: " + file.errorString());
        }
    }

}

// 窗口关闭事件
void FormChildDataConversion::closeEvent(QCloseEvent *event)
{
    // 步骤1：保存日志到文件
    saveLog();

    // 步骤2：接受关闭事件，允许窗口关闭
    event->accept();
}


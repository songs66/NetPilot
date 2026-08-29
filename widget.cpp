#include "widget.h"
#include "ui_widget.h"
#include "formdataprocessor.h"
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    dataProcessor = new FormDataProcessor(ui->tab_DataExchange);
    QVBoxLayout *dataLayout = new QVBoxLayout(ui->tab_DataExchange);
    dataLayout->setContentsMargins(0, 0, 0, 0);
    dataLayout->addWidget(dataProcessor);
}

Widget::~Widget()
{
    delete ui;
}

// 窗口关闭事件，保存所有子窗口的日志
void Widget::closeEvent(QCloseEvent *event)
{
    // 保存TCP服务器日志
    if(ui->tab_TCPServer){
        ui->tab_TCPServer->saveLog();
    }

    // 保存TCP客户端日志
    if(ui->tab_TCPClient){
        ui->tab_TCPClient->saveLog();
    }

    // 保存UDP服务器日志
    if(ui->tab_UDPServer){
        ui->tab_UDPServer->saveLog();
    }

    // 保存UDP客户端日志
    if(ui->tab_UDPClient){
        ui->tab_UDPClient->saveLog();
    }

    // 保存数据转换工具日志
    if(dataProcessor){
        try{
            dataProcessor->saveLog();
        }catch(const std::exception& e){
            qWarning()<<"保存数据转换工具日志失败:"<<e.what();
        }
    }

    event->accept();

}

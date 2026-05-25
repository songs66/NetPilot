#include "formtcpserver.h"
#include "ui_formtcpserver.h"

FormTcpServer::FormTcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormTcpServer)
{
    ui->setupUi(this);
}

FormTcpServer::~FormTcpServer()
{
    delete ui;
}

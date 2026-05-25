#include "formudpserver.h"
#include "ui_formudpserver.h"

FormUdpServer::FormUdpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormUdpServer)
{
    ui->setupUi(this);
}

FormUdpServer::~FormUdpServer()
{
    delete ui;
}

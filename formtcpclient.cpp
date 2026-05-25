#include "formtcpclient.h"
#include "ui_formtcpclient.h"

FormTcpClient::FormTcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormTcpClient)
{
    ui->setupUi(this);
}

FormTcpClient::~FormTcpClient()
{
    delete ui;
}

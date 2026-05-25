#include "formudpclient.h"
#include "ui_formudpclient.h"

FormUdpClient::FormUdpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormUdpClient)
{
    ui->setupUi(this);
}

FormUdpClient::~FormUdpClient()
{
    delete ui;
}

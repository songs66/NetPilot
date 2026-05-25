#ifndef FORMTCPSERVER_H
#define FORMTCPSERVER_H

#include <QWidget>

namespace Ui {
class FormTcpServer;
}

class FormTcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit FormTcpServer(QWidget *parent = nullptr);
    ~FormTcpServer();

private:
    Ui::FormTcpServer *ui;
};

#endif // FORMTCPSERVER_H

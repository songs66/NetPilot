#ifndef FORMUDPSERVER_H
#define FORMUDPSERVER_H

#include <QWidget>

namespace Ui {
class FormUdpServer;
}

class FormUdpServer : public QWidget
{
    Q_OBJECT

public:
    explicit FormUdpServer(QWidget *parent = nullptr);
    ~FormUdpServer();

private:
    Ui::FormUdpServer *ui;
};

#endif // FORMUDPSERVER_H

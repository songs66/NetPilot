#ifndef FORMUDPCLIENT_H
#define FORMUDPCLIENT_H

#include <QWidget>

namespace Ui {
class FormUdpClient;
}

class FormUdpClient : public QWidget
{
    Q_OBJECT

public:
    explicit FormUdpClient(QWidget *parent = nullptr);
    ~FormUdpClient();

private:
    Ui::FormUdpClient *ui;
};

#endif // FORMUDPCLIENT_H

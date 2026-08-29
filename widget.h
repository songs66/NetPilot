#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCloseEvent>

#include "formdataprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    // 窗口关闭事件，保存所有子窗口的日志
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Widget *ui;
    FormDataProcessor *dataProcessor;
};
#endif // WIDGET_H

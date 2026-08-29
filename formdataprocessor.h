#ifndef FORMDATAPROCESSOR_H
#define FORMDATAPROCESSOR_H

#include <QWidget>

#include "formchilddataconversion.h"
#include "formchilddatavalidation.h"

namespace Ui {
class FormDataProcessor;
}

class FormDataProcessor : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataProcessor(QWidget *parent = nullptr);
    ~FormDataProcessor();

    // 保存日志（公共接口，提供主窗口调用）
    void saveLog();

private:
    Ui::FormDataProcessor *ui;
};

#endif // FORMDATAPROCESSOR_H

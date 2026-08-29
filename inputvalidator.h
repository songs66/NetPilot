#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

// 代码已审核--Vico

// 作用：集中式输入校验工具，提供IP/端口/主机名/文件路径/字符串/数值/编码等校验接口

#include <QString>                  // Qt字符串类，用于处理Unicode字符串
#include <QRegularExpression>       // Qt正则表达式类，用于模式匹配和校验
#include <QHostAddress>             // Qt主机地址类，用于解析和验证IP地址
#include <QAbstractSocket>          // Qt抽象套接字类，提供协议枚举（IPv4/IPv6）

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif


class InputValidator  // 定义InputValidatorod类，作为静态工具类，提供输入校验功能
{
public:
    // 通用校验结果：isValid表示成功，errorMessage 给出失败原因
    struct ValidationResult {
        bool isValid;
        QString errorMessage;

        ValidationResult(bool valid = true, const QString &message = QString())
            : isValid(valid),
            errorMessage(message) {}
    };


    // IP地址校验函数：验证输入的字符串是否为有效的IPv4或IPv6地址
    static ValidationResult validatorIPAddress(const QString &ipAddress);

    // 端口数值校验函数：验证整数端口号是否在有效范围内
    static ValidationResult validatorPort(int port);

    // 端口字符串校验函数：将字符串转换为整数后调用数值校验
    static ValidationResult validatorPort(const QString &portStr);

    // 主机名校验函数：验证输入是否符合FRC标准的主机名格式
    static ValidationResult validatorHostname(const QString &hostname);

    // 网络地址校验函数：尝试按IP地址校验，失败则按主机名校验
    static ValidationResult validatorNetworkAddress(const QString &address);

    // 文件路径校验函数：验证文件路径格式，可选检查文件是否存在
    static ValidationResult validatorFilePath(const QString &filePath, bool mustExist = false);

    // 非空字符串校验函数：验证字符串不为空且不仅包含空白字符
    static ValidationResult validatorNonEmptyString(const QString &str, const QString &fieldName = "字段");

    // 字符串长度区间校验函数：验证字符串长度是否在指定范围内（闭区间）
    static ValidationResult validatorStringLength(const QString &str, int minLength, int maxLength, const QString &fieldName = "字段");

    // 数值范围校验函数：验证整数是否在指定范围内（闭区间）
    static ValidationResult validatorNumberRange(int value, int minValue, int maxValue, const QString &fieldName = "数值");

    // 十六进制字符串校验函数：验证字符串是否为有效的十六进制格式
    static ValidationResult validatorHexString(const QString &hexStr);

    // Base64字符串校验函数：验证字符串是否符合Base64编码格式
    static ValidationResult validatorBase64String(const QString &base64Str);

private:
    InputValidator()=default;

    // 预编译的正则规则：避免重复构造带来的性能消耗
    static const QRegularExpression ipv4Regex;
    static const QRegularExpression ipv6Regex;
    static const QRegularExpression hostnameRegex;
    static const QRegularExpression hexRegex;
    static const QRegularExpression base64Regex;

};

#endif // INPUTVALIDATOR_H

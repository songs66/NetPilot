/* 数据转换与校验核心工具类声明
 * 核心功能：
 * 1、进制转换（十进制和二进制之间的相互转换）
 * 2、数据校验（支持CRC16、CRC32、MD5、SHA1、SHA256多种校验算法）
 * 3、数据统计：统计字符串的字符数和UTF-8编码后的字节数
 *
 * 设计特点：
 * 纯静态工具类，无需实例化即可使用
 * 使用查找表优化CRC计算性能
 * 统一的错误处理机制
 * 支持大数据的异步处理
 */

#ifndef DATACONVERTER_H
#define DATACONVERTER_H

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>  // Qt加密哈希类（用于MD5 SHA等）
#include <QRegularExpression>
#include <QtGlobal>
#include <QDebug>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif


class DataConverter
{
public:
    // 进制/编码转换结果封装结构体
    struct ConversionResult{
        bool success;       // 转换是否成功标志
        QString data;       // 转换后的数据字符串（成功时有效）
        QString errorMessage; // 错误信息（失败时有效，成功时为空字符串）

        // ConversionResult构造函数
        ConversionResult(bool ok=true,const QString &result=QString(),
                         const QString &error=QString())
            :success(ok),data(result),errorMessage(error){}
    };

    // 校验值结果封装结构体
    struct ChecksumResult{
        QString crc16;
        QString crc32;
        QString md5;
        QString sha1;
        QString sha256;
    };

    // ======================================= 进制转换功能 =======================================
    // 将十进制字符串转换为二进制字符串
    static ConversionResult decimalToBinary(const QString &decimalString);

    // 将二进制字符串转换为十进制字符串
    static ConversionResult binaryToDecimal(const QString &binaryString);



    // ======================================= 数据统计功能 =======================================
    // 统计字符串的字符数
    static int countBytes(const QString &data);

    // 统计字符串的字符数（Qt字符长度）
    static int countCharacter(const QString &data);

    // 获取字符串的统计信息（字符数和字节数）
    static QString getDataInfo(const QString &data);



    // ======================================= 校验值计算功能 =======================================
    // 计算所有常用校验值
    static ChecksumResult calculateChecksums(const QString &data);

    // 计算所有常用校验值（字节数组输入版本）
    static ChecksumResult calculateChecksumsFromBytes(const QByteArray &data);

    // 单独计算MD5哈希值
    static QString calculateMD5(const QString &data);

    // 单独计算SHA1哈希值
    static QString calculateSHA1(const QString &data);

    // 单独计算SHA256哈希值
    static QString calculateSHA256(const QString &data);

    // 计算CRC16校验值
    static quint16 calculateCRC16(const QByteArray &data);

    // 计算CRC32校验值
    static quint32 calculateCRC32(const QByteArray &data);


public:
    // 公有构造函数，禁止实例化
    DataConverter()=default;

    // CRC16查找表（256项）
    // 使用 CRC-16-CCITT 标准
    static const quint16 crc16Table[256];

    // CRC32查找表（256项）
    // 使用 IEEE 802.3 标准
    static const quint32 crc32Table[256];

};

#endif // DATACONVERTER_H

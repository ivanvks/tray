// В классе CSqlDatabase объяв собственный сигнал для обработки уведомлений
signals:
    void postgresNotificationReceived(const QString &notificationName);

// В классе CSqlDatabase, в конструкторе или при инициализации, подпиcь на уведомления от PostgreSQL
void CSqlDatabase::subscribeToPostgreSQLNotifications()
{
    QSqlDriver *driver = QSqlDatabase::database().driver();
    if (driver->subscribedToNotifications().isEmpty()) {
        // Здесь подпись на уведомления, 
        driver->subscribeToNotification("message_updated");
    }
    QObject::connect(driver, SIGNAL(notification(QString, QSqlDriver::NotificationSource)),
                     this, SLOT(handlePostgresNotification(QString, QSqlDriver::NotificationSource)));
QObject::connect(this, SIGNAL(postgresNotificationReceived(QString)),
                 this, SLOT(selectFromInMSG(QString)));

}

// слот для обработки уведомлений
void CSqlDatabase::handlePostgresNotification(const QString &notificationName, QSqlDriver::NotificationSource)
{
    // Проверяем, что уведомление от PostgreSQL имеет нужное имя
    if (notificationName == "message_updated") {
        // Вызываем функцию для выполнения запроса к базе данных
        QVector<QByteArray> data = selectFromInMSG("your_msg_subtype");
        
        emit postgresNotificationReceived(notificationName);
    }
}
PERFORM pg_notify('message_updated', 'Some optional message');

.................................................................................................

// CSqlDatabase.h

#ifndef CSQLDATABASE_H
#define CSQLDATABASE_H

#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QMap>

#include "csettings.h"

class CSqlDatabase : public QObject
{
    Q_OBJECT

    QSqlDatabase mADataBase_;
    QSqlDatabase mPDataBase_;
    QMap<QString, QString> mKlassTableMap_;

public:
    CSqlDatabase(const QString &type = "QPSQL");

    bool openDataBase(const CSettings &sqlParam);
    bool isOpenA();
    bool isOpenP();
    QString lastErrorA();
    QString lastErrorP();
    QVector<QByteArray> selectFromInMSG(const QString &msgSubtype);
    bool sendViaTlk(const QString &msgUrgent, const QString &msgMac, const QString &msgType, const QString &msgSubtype, const QString &msgTtl, const QStringList &msgAddrLst, const QByteArray &msgData);
    QString klassName(const QString &klass, const QString &kod);

public slots:
    void handlePostgresNotification(const QString &notificationName, QSqlDriver::NotificationSource source, const QVariant &payload);

private:
    void deleteFromInMSG(const QStringList &idLst);
    void initKlassTableMap();
};

#endif // CSQLDATABASE_H



// CSqlDatabase.cpp

#include "csqldatabase.h"

#include <QDebug>
#include <QUuid>

CSqlDatabase::CSqlDatabase(const QString &type)
{
    mADataBase_ = QSqlDatabase::addDatabase(type, "a");
    mPDataBase_ = QSqlDatabase::addDatabase(type, "p");
    initKlassTableMap();

    // Включение уведомлений PostgreSQL
    QSqlDriver *driver = mADataBase_.driver();
    if (driver->hasFeature(QSqlDriver::Notifications)) {
        if (driver->subscribeToNotification("message_updated")) {
            qDebug() << "Подписка на уведомление PostgreSQL 'message_updated' выполнена успешно";
        } else {
            qDebug() << "Не удалось подписаться на уведомление PostgreSQL 'message_updated'";
        }
    } else {
        qDebug() << "Драйвер PostgreSQL не поддерживает уведомления";
    }
}

bool CSqlDatabase::openDataBase(const CSettings &sqlParam)
{
    mADataBase_.setHostName(sqlParam.aParam().value("host"));
    mADataBase_.setPort(sqlParam.aParam().value("port").toInt());
    mADataBase_.setDatabaseName(sqlParam.aParam().value("name"));
    mADataBase_.setUserName(sqlParam.aParam().value("user"));
    mADataBase_.setPassword(sqlParam.aParam().value("pass"));

    mPDataBase_.setHostName(sqlParam.pParam().value("host"));
    mPDataBase_.setPort(sqlParam.pParam().value("port").toInt());
    mPDataBase_.setDatabaseName(sqlParam.pParam().value("name"));
    mPDataBase_.setUserName(sqlParam.pParam().value("user"));
    mPDataBase_.setPassword(sqlParam.pParam().value("pass"));

    return mADataBase_.isOpen() && mPDataBase_.isOpen();
}

bool CSqlDatabase::isOpenA()
{
    return mADataBase_.isOpen();
}

bool CSqlDatabase::isOpenP()
{
    return mPDataBase_.isOpen();
}

QString CSqlDatabase::lastErrorA()
{
    return mADataBase_.lastError().text();
}

QString CSqlDatabase::lastErrorP()
{
    return mPDataBase_.lastError().text();
}

QVector<QByteArray> CSqlDatabase::selectFromInMSG(const QString &msgSubtype)
{
    QVector<QByteArray> dataVector;
    QSqlQuery sqlQuery(mADataBase_);
    sqlQuery.prepare("SELECT in_msg_buff.msg_id, in_msg_data.msg_data FROM "
                     "in_msg_data, in_msg_buff "
                     "WHERE in_msg_buff.msg_id = in_msg_data.msg_id AND in_msg_buff.msg_subtype = :msgSubtype "
                     "LIMIT 1");
    sqlQuery.bindValue(":msgSubtype", msgSubtype);

    if (!sqlQuery.exec()) {
        qDebug() << __PRETTY_FUNCTION__ << sqlQuery.lastQuery() << sqlQuery.lastError();
        return dataVector;
    }

    QStringList idLst;
    while (sqlQuery.next()) {
        dataVector.append(sqlQuery.value(1).toByteArray());
        idLst.append(sqlQuery.value(0).toString());
    }

    deleteFromInMSG(idLst);
    return dataVector;
}

bool CSqlDatabase::sendViaTlk(const QString &msgUrgent, const QString &msgMac, const QString &msgType, const QString &msgSubtype, const QString &msgTtl, const QStringList &msgAddrLst, const QByteArray &msgData)
{
    QSqlQuery sqlQuery(mADataBase_);
    sqlQuery.prepare("INSERT INTO send_msg "
                     "(msg_urgent, msg_mac, msg_type, msg_subtype, msg_ttl, "
                     "msg_address, msg_user, msg_uuid, msg_data) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    sqlQuery.bindValue(0, msgUrgent);
    sqlQuery.bindValue(1, msgMac);
    sqlQuery.bindValue(2, msgType);
    sqlQuery.bindValue(3, msgSubtype);
    sqlQuery.bindValue(4, msgTtl);
    sqlQuery.bindValue(5, msgAddrLst.join(' '));
    sqlQuery.bindValue(6, QString::fromLocal8Bit(getenv("USER")));
    sqlQuery.bindValue(7, QUuid::createUuid().toByteArray());
    sqlQuery.bindValue(8, msgData);

    if (!sqlQuery.exec()) {
        qDebug() << sqlQuery.lastQuery() << sqlQuery.lastError();
    }

    return !sqlQuery.last();
}

QString CSqlDatabase::klassName(const QString &klass, const QString &kod)
{
    QSqlQuery sqlQuery(mPDataBase_);
    sqlQuery.prepare(QString("SELECT name FROM %1 WHERE kod = :kod").arg(mKlassTableMap_.value(klass)));
    sqlQuery.bindValue(":kod", kod);

    if (!sqlQuery.exec()) {
        qDebug() << __PRETTY_FUNCTION__ << sqlQuery.lastQuery() << sqlQuery.lastError();
        return QString();
    }

    if (!sqlQuery.first()) {
        return QString();
    }

    return sqlQuery.value(0).toString();
}

void CSqlDatabase::handlePostgresNotification(const QString &notificationName, QSqlDriver::NotificationSource source, const QVariant &payload)
{
    Q_UNUSED(notificationName);
    Q_UNUSED(source);
    Q_UNUSED(payload);

    // Обработка уведомления PostgreSQL
    QString msgSubtype = "ВашПодтипЗдесь";
    QVector<QByteArray> data = selectFromInMSG(msgSubtype);

    // Обработайте данные по вашим требованиям
......................НОРМ................................


// класс CSqlDatabase
// ...

// Соединяем сигнал notification с вашим слотом
QObject::connect(QSqlDatabase::database().driver(),
                 SIGNAL(notification(QString, QSqlDriver::NotificationSource, QVariant)),
                 this,
                 SLOT(handlePostgresNotification(QString, QSqlDriver::NotificationSource, QVariant)));

// слот для обработки уведомлений
void CSqlDatabase::handlePostgresNotification(const QString &notificationName, QSqlDriver::NotificationSource source, const QVariant &payload)
{
    if (source == QSqlDriver::SelfSource)
    {
        if (notificationName == "message_updated")
        {
            // Извлекаем данные из базы данных и обрабатываем их
            QString msgSubtype = payload.toString();
            QVector<QByteArray> dataVector = selectFromInMSG(msgSubtype);
            // Теперь вы можете использовать dataVector для обработки данных
        }
    }
}

// функция для извлечения данных из базы данных
QVector<QByteArray> CSqlDatabase::selectFromInMSG(const QString &msgSubtype)
{
    QVector<QByteArray> dataVector;
    // Ваш код для извлечения данных из базы данных и добавления их в dataVector
    // ...
    return dataVector;
}
.............................................без допов ...............


// Соединяем сигнал notification с вашим слотом
QObject::connect(QSqlDatabase::database().driver(),
                 SIGNAL(notification(QString, QSqlDriver::NotificationSource)),
                 this,
                 SLOT(handleNotification(QString, QSqlDriver::NotificationSource)));

// слот для обработки уведомлений
void CSqlDatabase::handleNotification(const QString &notificationName, QSqlDriver::NotificationSource source)
{
    if (source == QSqlDriver::SelfSource)
    {
        if (notificationName == "message_updated")
        {
            // Обработка уведомления
        }
    }
}
........подумать


// В классе CSqlDatabase  собственный сигнал для обработки уведомлений
signals:
    void postgresNotificationReceived(const QString &notificationName);

// В классе CSqlDatabase, в конструкторе или при инициализации, подпись на уведомления от PostgreSQL
void CSqlDatabase::subscribeToPostgreSQLNotifications()
{
    QSqlDriver *driver = QSqlDatabase::database().driver();
    if (driver->subscribedToNotifications().isEmpty()) {
        // Здесь вы подписываетесь на уведомления, которые  интересуют, например:
        driver->subscribeToNotification("message_updated");
    }
    QObject::connect(driver, SIGNAL(notification(QString, QSqlDriver::NotificationSource)),
                     this, SLOT(handlePostgresNotification(QString, QSqlDriver::NotificationSource)));
}

// слот для обработки уведомлений
void CSqlDatabase::handlePostgresNotification(const QString &notificationName, QSqlDriver::NotificationSource)
{
    // Проверяем, что уведомление от PostgreSQL имеет нужное имя
    if (notificationName == "message_updated") {
        // Вызываем функцию для выполнения запроса к базе данных
        QVector<QByteArray> data = selectFromInMSG("your_msg_subtype");
        // Далее вы можете обработать полученные данные или выполнить другие действия
        emit postgresNotificationReceived(notificationName);
    }
}

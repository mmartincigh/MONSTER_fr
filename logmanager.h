#ifndef LOGMANAGER_H
#define LOGMANAGER_H

// Qt
#include <QObject>
#include <QMutex>
#include <QDebug>

class LogManager : public QObject
{
    Q_OBJECT

private:
    static const QString m_LOG_FILENAME;
    static const QString m_LOG_DATE_TIME_FORMAT;
    static QString m_logPath;
    static QString m_logAbsoluteFilePath;
    static QMutex m_mutex;
    QString m_logTag;

public:
    explicit LogManager(const QString &logTag, QObject *parent = NULL);
    ~LogManager();

public:
    static void initialize(const QString &applicationDirPath);
    static void messageHandler(QtMsgType messageType, const QMessageLogContext &messageLogContext, const QString &message);

protected:
    void debug(const QString &debugMessage);
    void warning(const QString &warningMessage);
    void error(const QString &errorMessage);

signals:
    void debugMessage(const QString &debugMessage);
    void warningMessage(const QString &warningMessage);
    void errorMessage(const QString &errorMessage);
};

#endif // LOGMANAGER_H

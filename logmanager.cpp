// Std
#include <iostream>

// Qt
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

// Local
#include "logmanager.h"
#include "applicationutils.h"

const QString LogManager::m_LOG_FILENAME(ApplicationUtils::APPLICATION_NAME + ".txt");
const QString LogManager::m_LOG_DATE_TIME_FORMAT("yyyy-MM-dd HH.mm.ss.zzz ");
QString LogManager::m_logPath("");
QString LogManager::m_logAbsoluteFilePath("");
QMutex LogManager::m_mutex;

LogManager::LogManager(const QString &logTag, QObject *parent) :
    QObject(parent),
    m_logTag(logTag)
{
}

LogManager::~LogManager()
{
}

void LogManager::initialize(const QString &applicationDirPath)
{
    // Prepare the log header.
    QString log_header_info(ApplicationUtils::APPLICATION_NAME + " " + ApplicationUtils::APPLICATION_VERSION);
    QString log_header_frame(log_header_info.length(), '=');
    QString log_header(log_header_frame + "\n" + log_header_info + "\n" + log_header_frame + "\n");

    // Create the log file (if it doesn't exist) and write the header.
    m_logPath = applicationDirPath;
    QDir application_directory(m_logPath);
    m_logAbsoluteFilePath = application_directory.filePath(LogManager::m_LOG_FILENAME);
    QFile log_file(m_logAbsoluteFilePath);
    log_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Append);
    QTextStream text_stream(&log_file);
    text_stream << log_header << endl << flush;
    log_file.close();
}

void LogManager::messageHandler(QtMsgType messageType, const QMessageLogContext &messageLogContext, const QString &message)
{
    Q_UNUSED(messageLogContext)

    // Format the log message.
    QString message_type;
    switch (messageType) {
    case QtDebugMsg:
        message_type = "   ";

        break;
    case QtWarningMsg:
        message_type = "WRN";

        break;
    case QtCriticalMsg:
        message_type = "ERR";

        break;
    case QtFatalMsg:
        message_type = "FTL";

        break;
    default:
        message_type = "UNK";

        break;
    }
    QString message_string(QDateTime::currentDateTime().toString(m_LOG_DATE_TIME_FORMAT));
    message_string += QString("%1 %2").arg(message_type).arg(message);

    // Log to file.
    QFile log_file(m_logAbsoluteFilePath);
    log_file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&log_file);
    text_stream << message_string << endl << flush;
    log_file.close();

    // Log to the output file.
    FILE *output_file = NULL;
    output_file = messageType == QtDebugMsg ? stdout : stderr;
    fprintf(output_file, "%s\n", message.toStdString().c_str());
}

void LogManager::debug(const QString &debugMessage)
{
    QMutexLocker mutex_locker(&m_mutex);

    emit this->debugMessage(debugMessage);

    qDebug() << qPrintable(m_logTag.rightJustified(5, ' ')) << ">" << qPrintable(debugMessage);
}

void LogManager::warning(const QString &warningMessage)
{
    QMutexLocker mutex_locker(&m_mutex);

    emit this->warningMessage(warningMessage);

    qWarning() << qPrintable(m_logTag.rightJustified(5, ' ')) << "> WARNING:" << qPrintable(warningMessage);
}

void LogManager::error(const QString &errorMessage)
{
    QMutexLocker mutex_locker(&m_mutex);

    emit this->errorMessage(errorMessage);

    qCritical() << qPrintable(m_logTag.rightJustified(5, ' ')) << "> ERROR:" << qPrintable(errorMessage);
}

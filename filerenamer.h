#ifndef FILERENAMER_H
#define FILERENAMER_H

// Qt
#include <QObject>
#include <QDir>

// Local
#include "base.h"
class FileRenamer : public Base
{
    Q_OBJECT

private:
    static const QString m_IMAGE_TIMESTAMP_TAG;
    static const QString m_TUMBLR_FILTER_1;
    static const QString m_TUMBLR_FILTER_2;
    static const QString m_TUMBLR_FILTER_3;
    static const QString m_TUMBLR_FILTER_4;
    static const QString m_PHONEGRAM_FILTER;
    static const QString m_TELEGRAM_FILTER;
    static const QString m_RUNKEEPER_APP_FILTER;
    static const QString m_RUNKEEPER_WEB_FILTER;
    static const QString m_FLIPBOARD_FILTER;
    static const QString m_GOOGLE_IMAGES_FILTER;
    static const QString m_ANDROID_FILTER;
    enum FileRename_RetVal
    {
        FileRename_Success,
        FileRename_Skipped,
        FileRename_Error
    };
    QStringList m_fileFilters;
    int m_totalFileCount;
    int m_renamedFileCount;

public:
    explicit FileRenamer(QObject *parent = NULL);
    ~FileRenamer();

public:
    int totalFileCount() const;
    int renamedFileCount() const;
    void processDirectories(const QList<QDir> &directories);
    void processFiles(const QFileInfoList &files);

private:
    void renameFiles(const QDir &directory);
    FileRename_RetVal renameFile(const QFileInfo &file);
};

#endif // FILERENAMER_H

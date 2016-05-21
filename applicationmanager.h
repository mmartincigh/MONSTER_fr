#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

// Qt
#include <QObject>
#include <QFileInfoList>
#include <QDebug>

// Local
#include "base.h"
#include "filerenamer.h"

class ApplicationManager : public Base
{
    Q_OBJECT

private:
    FileRenamer m_fileRenamer;
    QStringList m_arguments;

public:
    explicit ApplicationManager(const QStringList &arguments, QObject *parent = NULL);
    ~ApplicationManager();

public:
    void initialize();
    int exec();
    bool parseArguments(QList<QDir> &directories, QFileInfoList &files);
};

#endif // APPLICATIONMANAGER_H

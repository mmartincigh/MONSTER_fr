// Qt
#include <QFileInfo>
#include <QDir>

// Local
#include "applicationmanager.h"

ApplicationManager::ApplicationManager(const QStringList &arguments, QObject *parent) :
    Base("AM", parent),
    m_fileRenamer(this),
    m_arguments(arguments)
{
    this->debug("Application manager created");
}

ApplicationManager::~ApplicationManager()
{
    this->debug("Application manager disposed of");
}

void ApplicationManager::initialize()
{
    this->debug("Initialized");
}

int ApplicationManager::exec()
{
    // Parse arguments.
    QList<QDir> directories;
    QFileInfoList files;
    bool ret_val = this->parseArguments(directories, files);
    if (!ret_val)
    {
        this->error("Error parsing arguments");

        return EXIT_FAILURE;
    }

    // Process directories.
    m_fileRenamer.processDirectories(directories);

    // Process files.
    m_fileRenamer.processFiles(files);

    this->debug("================");

    this->debug("Files renamed: " + QString::number(m_fileRenamer.renamedFileCount()) + "/" + QString::number(m_fileRenamer.totalFileCount()));

    this->debug("Done");

    return m_fileRenamer.renamedFileCount() == m_fileRenamer.totalFileCount() ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool ApplicationManager::parseArguments(QList<QDir> &directories, QFileInfoList &files)
{
    // Check whether the argument list is empty.
    if (m_arguments.empty())
    {
        this->warning("Empty argument list");

        return false;
    }

    // Process arguments (skip the executable name).
    for (int i = 1; i < m_arguments.count(); i++)
    {
        QString argument(m_arguments.at(i));

        this->debug("Processing argument: " + argument);

        // Check whether the argument is a directory or a file (or neither).
        QFileInfo argument_file_info(argument);
        if (!argument_file_info.exists())
        {
            // Skip to the next argument.

            continue;
        }

        // Check whether the argument is a directory.
        if (argument_file_info.isDir())
        {
            this->debug("Directory");

            // Append the argument to the list of directories.
            directories.append(QDir(argument));
        }

        // Check whether the argument is a file.
        else if (argument_file_info.isFile())
        {
            this->debug("File");

            // Append the argument to the list of files.
            files.append(argument_file_info);
        }
    }

    return true;
}

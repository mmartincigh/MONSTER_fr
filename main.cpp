// Qt
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QRegularExpressionMatch>
#include <QDebug>

// Exiv2
#include <exiv2/exiv2.hpp>

static const QString IMAGE_TIMESTAMP_TAG        ("Exif.Photo.DateTimeOriginal");

static const QString TUMBLR_FILTER_1            ("^https?%[0-9a-fA-F]{2}%[0-9a-fA-F]{2}%[0-9a-fA-F]{4}.media.tumblr.com(%[0-9a-fA-F]{34})?%[0-9a-fA-F]{2}tumblr_[0-9a-zA-Z]{19}(_.{2})?_[0-9]{3,4}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString TUMBLR_FILTER_2            ("^tumblr_[\\w]{19}_[0-9]{3}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString TUMBLR_FILTER_3            ("^([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp))$");
static const QString PHONEGRAM_FILTER           ("^IMG_[0-9]{8}_[0-9]{6}_[0-9]{3}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString TELEGRAM_FILTER            ("^[0-9]{9}_[0-9]{5,6}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString RUNKEEPER_APP_FILTER       ("^[0-9]{13}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString RUNKEEPER_WEB_FILTER       ("^[\\w]{24}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString FLIPBOARD_FILTER           ("^[0-9a-fA-F]{40}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString GOOGLE_IMAGES_FILTER       ("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp)$");
static const QString ANDROID_FILTER             ("^IMG_[0-9]{8}_[0-9]{6}\\.(?i)(jpe?g|png|gif|bmp)$");

static QStringList FILE_FILTERS;

static int total_file_count                     (0);
static int renamed_file_count                   (0);

enum FileRename_RetVal
{
    FileRename_Success,
    FileRename_Skipped,
    FileRename_Error
};

int parseArguments(const QStringList &arguments, QList<QDir> &directories, QFileInfoList &files)
{
    // Check whether the argument list is empty.
    if (arguments.empty())
    {
        qWarning() << "WARNING> Empty argument list";

        return -1;
    }

    // Process arguments (skip the executable entry).
    for (int i = 1; i < arguments.count(); i++)
    {
        QString argument(arguments.at(i));

        qDebug() << "Processing argument:" << qPrintable(argument);

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
            qDebug() << "Directory";

            // Append the argument to the list of directories.
            directories.append(QDir(argument));
        }

        // Check whether the argument is a file.
        else if (argument_file_info.isFile())
        {
            qDebug() << "File";

            // Append the argument to the list of files.
            files.append(argument_file_info);
        }
    }

    return 0;
}

FileRename_RetVal renameFile(const QFileInfo &file)
{
    QString file_name = file.fileName();

    qDebug() << "Current file:" << qPrintable(file_name);

    // Check if the current file needs to be renamed.
    bool file_filter_match = false;
    foreach (const QString &image_filter, FILE_FILTERS)
    {
        QRegularExpression regular_expression(image_filter);
        QRegularExpressionMatch regular_expression_match = regular_expression.match(file_name);
        if (regular_expression_match.hasMatch())
        {
            file_filter_match = true;

            continue;
        }
    }
    if (!file_filter_match)
    {
        qDebug() << "File" << file_name << "doesn't match any of the filters, skipping...";

        return FileRename_Skipped;
    }

    // Increase the number of total files to rename.
    total_file_count++;

    QString file_absolute_path = file.absoluteFilePath();
    QString exif_data_image_timestamp;
    try
    {
        std::string image_absolute_path_string = file_absolute_path.toStdString();
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(image_absolute_path_string);
        if (image.get() == NULL)
        {
            qWarning() << "WARNING> Cannot load image:" << qPrintable(file_name);

            return FileRename_Error;
        }

        image->readMetadata();
        Exiv2::ExifData &exif_data = image->exifData();
        Exiv2::ExifKey exif_key(IMAGE_TIMESTAMP_TAG.toStdString());
        Exiv2::ExifData::const_iterator pos = exif_data.findKey(exif_key);
        if (pos != exif_data.end())
        {
            QString exif_data_value = QString::fromStdString(pos->toString());
            QStringList exif_data_image_timestamp_date_time_split = exif_data_value.split(' ');
            if (exif_data_image_timestamp_date_time_split.size() < 2)
            {
                qWarning() << "WARNING> Invalid image timestamp";

                return FileRename_Error;
            }

            QString exif_data_image_timestamp_date = exif_data_image_timestamp_date_time_split.at(0);
            exif_data_image_timestamp_date.replace(':', '-');
            QString exif_data_image_timestamp_time = exif_data_image_timestamp_date_time_split.at(1);
            exif_data_image_timestamp_time.replace(':', '.');
            exif_data_image_timestamp = exif_data_image_timestamp_date + " " + exif_data_image_timestamp_time;
        }
        else
        {
            qDebug() << "No image timestamp, using file attributes...";

            QDateTime image_file_last_modified_time = file.lastModified();
            exif_data_image_timestamp = image_file_last_modified_time.toString("yyyy-MM-dd HH.mm.ss");
        }
    }
    catch (Exiv2::AnyError &e)
    {
        qCritical() << "ERROR> Caught Exiv2 exception:" << e.what();

        return FileRename_Error;
    }

    qDebug() << "Image timestamp:" << qPrintable(exif_data_image_timestamp);

    QString new_image_name = exif_data_image_timestamp + "." + file.completeSuffix();

    qDebug() << "New image name:" << qPrintable(new_image_name);

    QDir file_absolute_directory(file.absoluteDir());
    QString new_image_file_name = file_absolute_directory.filePath(new_image_name);
    QFileInfo new_image_file_info(new_image_file_name);
    if (new_image_file_info.exists())
    {
        qWarning() << "WARNING> File" << new_image_name << "already exists";

        return FileRename_Error;
    }

    // Rename the file.
    QFile new_image_file(file_absolute_path);
    bool ret_val = new_image_file.rename(new_image_file_name);
    if (!ret_val)
    {
        qWarning() << "WARNING> Cannot rename file to:" << qPrintable(new_image_name);

        return FileRename_Error;
    }

    qDebug() << "File renamed to:" << qPrintable(new_image_file_info.absoluteFilePath());

    qDebug().nospace() << "Files renamed: " << ++renamed_file_count << "/" << total_file_count;

    return FileRename_Success;
}

int renameFiles(const QDir &directory)
{
    QFileInfoList files = directory.entryInfoList(QDir::Files, QDir::Name);

    foreach (const QFileInfo &file, files)
    {
        qDebug() << "----------------";

        FileRename_RetVal ret_val = renameFile(file);
        switch (ret_val)
        {
        case FileRename_Success:
        case FileRename_Skipped:
            break;
        case FileRename_Error:
        default:
            qWarning() << "WARNING> Cannot rename file:" << qPrintable(file.fileName());
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName("MONSTER_fr");
    application.setApplicationVersion(SW_VERSION);

    qDebug().nospace() << "MONSTER_fr v" << qPrintable(application.applicationVersion());

    // Prepare file filters;
    FILE_FILTERS << TUMBLR_FILTER_1
                 << TUMBLR_FILTER_2
                 << TUMBLR_FILTER_3
                 << PHONEGRAM_FILTER
                 << TELEGRAM_FILTER
                 << RUNKEEPER_APP_FILTER
                 << RUNKEEPER_WEB_FILTER
                 << FLIPBOARD_FILTER
                 << GOOGLE_IMAGES_FILTER
                 << ANDROID_FILTER;

    // Parse arguments.
    QList<QDir> directories;
    QFileInfoList files;
    int ret_val = parseArguments(application.arguments(), directories, files);
    if (ret_val < 0)
    {
        qCritical() << "ERROR> Error parsing arguments";

        return EXIT_FAILURE;
    }

    // Process directories.
    foreach (const QDir &directory, directories)
    {
        qDebug() << "================";

        ret_val = renameFiles(directory);
        if (ret_val < 0)
        {
            qWarning() << "WARNING> Cannot rename files in directory:" << qPrintable(directory.path());
        }
    }

    // Process files.
    foreach (const QFileInfo &file, files)
    {
        qDebug() << "----------------";

        ret_val = renameFile(file);
        if (ret_val < 0)
        {
            qWarning() << "WARNING> Cannot rename file:" << qPrintable(file.fileName());
        }
    }

    qDebug() << "================";

    qDebug().nospace() << "Files renamed: " << renamed_file_count << "/" << total_file_count;

    qDebug() << "Done";

    return renamed_file_count == total_file_count ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Qt
#include <QDateTime>
#include <QRegularExpressionMatch>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local
#include "filerenamer.h"

const QString FileRenamer::m_IMAGE_TIMESTAMP_TAG("Exif.Photo.DateTimeOriginal");
const QString FileRenamer::m_TUMBLR_FILTER_1("^https?%[0-9a-fA-F]{2}%[0-9a-fA-F]{2}%[0-9a-fA-F]{4}.media.tumblr.com(%[0-9a-fA-F]{34})?%[0-9a-fA-F]{2}tumblr_[0-9a-zA-Z]{19}(_.{2})?_[0-9]{3,4}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_TUMBLR_FILTER_2("^tumblr_[\\w]{19}_[0-9]{3,4}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_TUMBLR_FILTER_3("^tumblr_[\\w]{19,20}_[\\w]{2}_[0-9]{3}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_TUMBLR_FILTER_4("^([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp))$");
const QString FileRenamer::m_PHONEGRAM_FILTER("^IMG_[0-9]{8}_[0-9]{6}_[0-9]{3}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_TELEGRAM_FILTER("^[0-9]{9}_[0-9]{5,6}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_RUNKEEPER_APP_FILTER("^[0-9]{13}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_RUNKEEPER_WEB_FILTER("^[\\w]{24}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_FLIPBOARD_FILTER("^[0-9a-fA-F]{40}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_GOOGLE_IMAGES_FILTER("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp)$");
const QString FileRenamer::m_ANDROID_FILTER("^IMG_[0-9]{8}_[0-9]{6}\\.(?i)(jpe?g|png|gif|bmp)$");

FileRenamer::FileRenamer(QObject *parent) :
    Base("FR", parent),
    m_fileFilters(),
    m_totalFileCount(0),
    m_renamedFileCount(0)
{
    m_fileFilters << m_TUMBLR_FILTER_1
                  << m_TUMBLR_FILTER_2
                  << m_TUMBLR_FILTER_3
                  << m_TUMBLR_FILTER_4
                  << m_PHONEGRAM_FILTER
                  << m_TELEGRAM_FILTER
                  << m_RUNKEEPER_APP_FILTER
                  << m_RUNKEEPER_WEB_FILTER
                  << m_FLIPBOARD_FILTER
                  << m_GOOGLE_IMAGES_FILTER
                  << m_ANDROID_FILTER;

    this->debug("File renamer created");
}

FileRenamer::~FileRenamer()
{
    this->debug("File renamer disposed of");
}

int FileRenamer::totalFileCount() const
{
    return m_totalFileCount;
}

int FileRenamer::renamedFileCount() const
{
    return m_renamedFileCount;
}

void FileRenamer::processDirectories(const QList<QDir> &directories)
{
    // Process directories.
    foreach (const QDir &directory, directories)
    {
        this->debug("================");

        this->debug("Directory: " + directory.dirName());

        this->renameFiles(directory);
    }
}

void FileRenamer::processFiles(const QFileInfoList &files)
{
    // Process files.
    foreach (const QFileInfo &file, files)
    {
        this->debug("----------------");

        FileRename_RetVal ret_val = renameFile(file);
        switch (ret_val)
        {
        case FileRename_Success:
        case FileRename_Skipped:
            break;
        case FileRename_Error:
        default:
            this->warning("Cannot rename file: " + file.fileName());
        }
    }
}

void FileRenamer::renameFiles(const QDir &directory)
{
    QFileInfoList files = directory.entryInfoList(QDir::Files, QDir::Name);

    foreach (const QFileInfo &file, files)
    {
        this->debug("----------------");

        FileRename_RetVal ret_val = renameFile(file);
        switch (ret_val)
        {
        case FileRename_Success:
        case FileRename_Skipped:
            break;
        case FileRename_Error:
        default:
            this->warning("Cannot rename file: " + file.fileName());
        }
    }
}

FileRenamer::FileRename_RetVal FileRenamer::renameFile(const QFileInfo &file)
{
    QString file_name = file.fileName();

    this->debug("Current file: " + file_name);

    // Check if the current file needs to be renamed.
    bool file_filter_match = false;
    foreach (const QString &image_filter, m_fileFilters)
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
        this->debug("File " + file_name + " doesn't match any of the filters, skipping...");

        return FileRename_Skipped;
    }

    // Increase the number of total files to rename.
    m_totalFileCount++;

    QString file_absolute_path = file.absoluteFilePath();
    QString exif_data_image_timestamp;
    try
    {
        std::string image_absolute_path_string = file_absolute_path.toStdString();
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(image_absolute_path_string);
        if (image.get() == NULL)
        {
            this->warning("Cannot load image: " + file_name);

            return FileRename_Error;
        }

        image->readMetadata();
        Exiv2::ExifData &exif_data = image->exifData();
        Exiv2::ExifKey exif_key(m_IMAGE_TIMESTAMP_TAG.toStdString());
        Exiv2::ExifData::const_iterator pos = exif_data.findKey(exif_key);
        if (pos != exif_data.end())
        {
            QString exif_data_value = QString::fromStdString(pos->toString());
            QStringList exif_data_image_timestamp_date_time_split = exif_data_value.split(' ');
            if (exif_data_image_timestamp_date_time_split.size() < 2)
            {
                this->warning("Invalid image timestamp");

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
            this->debug("No image timestamp, using file attributes...");

            QDateTime image_file_last_modified_time = file.lastModified();
            exif_data_image_timestamp = image_file_last_modified_time.toString("yyyy-MM-dd HH.mm.ss");
        }
    }
    catch (Exiv2::AnyError &e)
    {
        this->error("Caught Exiv2 exception: " + QString(e.what()));

        return FileRename_Error;
    }

    this->debug("Image timestamp: " + exif_data_image_timestamp);

    QString new_image_name = exif_data_image_timestamp + "." + file.completeSuffix();

    this->debug("New image name: "+ new_image_name);

    QDir file_absolute_directory(file.absoluteDir());
    QString new_image_file_name = file_absolute_directory.filePath(new_image_name);
    QFileInfo new_image_file_info(new_image_file_name);
    if (new_image_file_info.exists())
    {
        this->warning("File " + new_image_name + " already exists");

        // Try with subsequent timestamps for a minute.
        this->debug("Trying with subsequent timestamps...");
        bool new_image_file_name_found = false;
        QDateTime current_image_timestamp = QDateTime::fromString(exif_data_image_timestamp, "yyyy-MM-dd HH.mm.ss");
        for (int i = 0; i < 60; i++)
        {
            current_image_timestamp = current_image_timestamp.addSecs(1);

            exif_data_image_timestamp = current_image_timestamp.toString("yyyy-MM-dd HH.mm.ss");

            //this->debug("Image timestamp: " + exif_data_image_timestamp);

            new_image_name = exif_data_image_timestamp + "." + file.completeSuffix();

            this->debug("New image name: "+ new_image_name);

            new_image_file_name = file_absolute_directory.filePath(new_image_name);
            new_image_file_info.setFile(new_image_file_name);
            if (!new_image_file_info.exists())
            {
                new_image_file_name_found = true;

                break;
            }
        }

        if (!new_image_file_name_found)
        {
            return FileRename_Error;
        }
    }

    // Rename the file.
    QFile new_image_file(file_absolute_path);
    bool ret_val = new_image_file.rename(new_image_file_name);
    if (!ret_val)
    {
        this->warning("Cannot rename file to: "+ new_image_name);

        return FileRename_Error;
    }

    this->debug("File renamed to: " + new_image_file_info.absoluteFilePath());

    this->debug("Files renamed: " + QString::number(++m_renamedFileCount) + "/" + QString::number(m_totalFileCount));

    return FileRename_Success;
}

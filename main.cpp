//#include <QCoreApplication>

// Qt
#include <QDir>
#include <QDateTime>
#include <QRegularExpressionMatch>
#include <QDebug>

// Exiv2
#include <exiv2/exiv2.hpp>

static const QStringList IMAGE_NAME_FILTERS("*");
static const QString IMAGE_TIMESTAMP_TAG("Exif.Photo.DateTimeOriginal");

static const QString TUMBLR_FILTER_3("^([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp))$");

int main()
{
    qDebug().nospace() << "MONSTER_fr v" << SW_VERSION;

    setbuf(stdout, NULL);

    QDir working_directory("D:\\Dropbox\\Tumblr\\Temp");

    qDebug() << "Working directory:" << qPrintable(working_directory.path());

    if (!working_directory.exists())
    {
        qCritical() << "ERROR> Working directory" << working_directory.path() << "does not exist";

        return EXIT_FAILURE;
    }

    QFileInfoList image_files = working_directory.entryInfoList(IMAGE_NAME_FILTERS, QDir::Files, QDir::Name);
    int image_files_count = image_files.count();
    int images_renamed = 0;

    foreach (const QFileInfo &image_file, image_files)
    {
        qDebug() << "----------------";

        QString image_name = image_file.fileName();
        QString image_absolute_path = image_file.absoluteFilePath();
        QString exif_data_image_timestamp;

        qDebug() << "Current image:" << qPrintable(image_name);

        // Check if the current image needs to be renamed.
        QRegularExpression regular_expression(TUMBLR_FILTER_3);
        QRegularExpressionMatch regular_expression_match = regular_expression.match(image_name);
        if (!regular_expression_match.hasMatch())
        {
            qWarning() << "WARNING> File" << image_name << "doesn't match any of the filters, skipping...";

            continue;
        }

        try
        {
            std::string image_absolute_path_string = image_absolute_path.toStdString();
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(image_absolute_path_string);
            if (image.get() == NULL)
            {
                qWarning() << "WARNING> Cannot load image:" << qPrintable(image_name);

                continue;
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

                    continue;
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

                QDateTime image_file_last_modified_time = image_file.lastModified();
                exif_data_image_timestamp = image_file_last_modified_time.toString("yyyy-MM-dd HH.mm.ss");
            }
        }
        catch (Exiv2::AnyError &e)
        {
            qCritical() << "ERROR> Caught Exiv2 exception:" << e.what();

            return EXIT_FAILURE;
        }

        qDebug() << "Image timestamp:" << qPrintable(exif_data_image_timestamp);

        QString new_image_name = exif_data_image_timestamp + "." + image_file.completeSuffix();

        qDebug() << "New image name:" << qPrintable(new_image_name);

        QString new_image_file_name = working_directory.filePath(new_image_name);
        QFileInfo new_image_file_info(new_image_file_name);
        if (new_image_file_info.exists())
        {
            qWarning() << "WARNING> File" << new_image_name << "already exists, skipping...";

            continue;
        }

        // Rename the file.
        QFile new_image_file(image_absolute_path);
        bool ret_val = new_image_file.rename(working_directory.filePath(new_image_name));
        if (!ret_val)
        {
            qWarning() << "WARNING> Cannot rename file to:" << qPrintable(new_image_name);

            continue;
        }

        qDebug() << "File renamed to:" << qPrintable(new_image_file_info.absoluteFilePath());

        qDebug().nospace() << ++images_renamed << "/" << image_files_count;
    }

    qDebug() << "----------------";

    qDebug() << "Done";

    return EXIT_SUCCESS;
}

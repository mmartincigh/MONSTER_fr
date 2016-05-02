// Qt
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QRegularExpressionMatch>
#include <QDebug>

// Exiv2
#include <exiv2/exiv2.hpp>

static const QString DEFAULT_WORKING_DIRECTORY  ("D:\\Dropbox\\Tumblr\\Temp");

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

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName("MONSTER_fr");
    application.setApplicationVersion(SW_VERSION);

    qDebug().nospace() << "MONSTER_fr v" << qPrintable(application.applicationVersion());

    QDir working_directory(DEFAULT_WORKING_DIRECTORY);

    if (application.arguments().count() > 1)
    {
        qDebug() << "Processing argument:" << qPrintable(application.arguments().at(1));

        working_directory = application.arguments().at(1);
        if (!working_directory.exists())
        {
            qWarning() << "WARNING> Passed working directory" << working_directory.path() << "does not exist";

            return EXIT_FAILURE;
        }
    }

    qDebug() << "Working directory:" << qPrintable(working_directory.path());

    if (!working_directory.exists())
    {
        qCritical() << "ERROR> Working directory" << working_directory.path() << "does not exist";

        return EXIT_FAILURE;
    }

    QFileInfoList image_files = working_directory.entryInfoList(QDir::Files, QDir::Name);
    int image_files_count = image_files.count();
    int images_renamed = 0;

    // Build image filter list.
    QStringList image_filters;
    image_filters << TUMBLR_FILTER_1
                  << TUMBLR_FILTER_2
                  << TUMBLR_FILTER_3
                  << PHONEGRAM_FILTER
                  << TELEGRAM_FILTER
                  << RUNKEEPER_APP_FILTER
                  << RUNKEEPER_WEB_FILTER
                  << FLIPBOARD_FILTER
                  << GOOGLE_IMAGES_FILTER
                  << ANDROID_FILTER;

    foreach (const QFileInfo &image_file, image_files)
    {
        qDebug() << "----------------";

        QString image_name = image_file.fileName();
        QString image_absolute_path = image_file.absoluteFilePath();
        QString exif_data_image_timestamp;

        qDebug() << "Current image:" << qPrintable(image_name);

        // Check if the current image needs to be renamed.
        bool image_filter_match = false;
        foreach (const QString &image_filter, image_filters)
        {
            QRegularExpression regular_expression(image_filter);
            QRegularExpressionMatch regular_expression_match = regular_expression.match(image_name);
            if (regular_expression_match.hasMatch())
            {
                image_filter_match = true;

                continue;
            }
        }

        if (!image_filter_match)
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

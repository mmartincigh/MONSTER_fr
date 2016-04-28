//#include <QCoreApplication>

// Qt
#include <QDir>
#include <QDateTime>
#include <QRegularExpressionMatch>
#include <QDebug>

// Exiv2
#include <exiv2/exiv2.hpp>

static const QString TUMBLR_FILTER_3("^([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\.(?i)(jpe?g|png|gif|bmp))$");

//int main(int argc, char *argv[])
int main()
{
    //QCoreApplication application(argc, argv);

    //return application.exec();

    qDebug() << "MONSTER_fr";

    setbuf(stdout, NULL);

    //QDir working_directory = QDir::current();
    QDir working_directory("D:\\Dropbox\\Tumblr\\Temp");

    qDebug() << "Current directory:" << working_directory.path();

    QStringList name_filters("*.jpg");
    QFileInfoList image_files = working_directory.entryInfoList(name_filters, QDir::Files, QDir::Name);
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
            qWarning() << "File" << image_name << "doesn't match any of the filters, skipping...";

            continue;
        }

        try
        {
            std::string image_absolute_path_string = image_absolute_path.toStdString();
            //std::string image_absolute_path_string = "D:\\Workspaces\\C++\\build-MONSTER_fr-Desktop_Qt_5_5_1_msvc2010-Debug\\2016-04-09 17.05.15.jpg";
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(image_absolute_path_string);
            if (image.get() == NULL)
            {
                qWarning() << "WARNING> Cannot load image:" << qPrintable(image_name);

                continue;
            }

            image->readMetadata();
            Exiv2::ExifData &exif_data = image->exifData();
            Exiv2::ExifKey exif_key("Exif.Photo.DateTimeOriginal");
            Exiv2::ExifData::const_iterator pos = exif_data.findKey(exif_key);
            //std::cout << std::setw(20) << std::left << "Image timestamp";
            if (pos != exif_data.end())
            {
                /*std::cout << " (" << std::setw(35) << pos->key() << ") : "
                          << pos->print(&exif_data) << "\n";*/
                //QString exif_data_vale = QString::fromUtf8(reinterpret_cast<char *>(pos->value()));
                QString exif_data_value = QString::fromStdString(pos->toString());
                //qDebug() << "Image timestamp:" << qPrintable(exif_data_value);
                QStringList exif_data_image_timestamp_date_time_split = exif_data_value.split(' ');
                if (exif_data_image_timestamp_date_time_split.size() < 2)
                {
                    qWarning() << "Invalid image timestamp";

                    continue;
                }
                QString exif_data_image_timestamp_date = exif_data_image_timestamp_date_time_split.at(0);
                exif_data_image_timestamp_date.replace(':', '-');
                QString exif_data_image_timestamp_time = exif_data_image_timestamp_date_time_split.at(1);
                exif_data_image_timestamp_time.replace(':', '.');
                exif_data_image_timestamp = exif_data_image_timestamp_date + " " + exif_data_image_timestamp_time;
            }
            else {
                /*std::cout << " (" << std::setw(35) << " " << ") : \n";*/
                qDebug() << "No image timestamp, using the last modified time...";

                QDateTime image_file_last_modified_time = image_file.lastModified();
                exif_data_image_timestamp = image_file_last_modified_time.toString("yyyy-MM-dd HH.mm.ss");
            }
        }
        catch (Exiv2::AnyError &e)
        {
            std::cout << "Caught Exiv2 exception '" << e << "'\n";

            return EXIT_FAILURE;
        }

        qDebug() << "Image timestamp:" << qPrintable(exif_data_image_timestamp);

        QString new_image_name = exif_data_image_timestamp + "." + image_file.completeSuffix();

        qDebug() << "New image name:" << qPrintable(new_image_name);

        QString new_image_file_name = working_directory.filePath(new_image_name);
        QFileInfo new_image_file_info(new_image_file_name);
        if (new_image_file_info.exists())
        {
            qWarning() << "File" << new_image_name << "already exists, skipping...";

            continue;
        }

        // Rename the file.
        QFile new_image_file(image_absolute_path);
        bool ret_val = new_image_file.rename(working_directory.filePath(new_image_name));
        if (!ret_val)
        {
            qWarning() << "Cannot rename file to:" << qPrintable(new_image_name);

            continue;
        }

        qDebug() << "File renamed to:" << qPrintable(new_image_file_info.absoluteFilePath());

        qDebug().nospace() << ++images_renamed << "/" << image_files_count;
    }

    qDebug() << "----------------";

    qDebug() << "Done";

    return EXIT_SUCCESS;
}

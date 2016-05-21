// Qt
#include <QCoreApplication>

// Local
#include "applicationmanager.h"
#include "applicationutils.h"
#include "logmanager.h"

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    qInstallMessageHandler(LogManager::messageHandler);

    QCoreApplication application(argc, argv);
    application.setOrganizationName(ApplicationUtils::COMPANY_NAME);
    application.setOrganizationDomain(ApplicationUtils::COMPANY_WEBSITE);
    application.setApplicationName(ApplicationUtils::APPLICATION_NAME);
    application.setApplicationVersion(ApplicationUtils::APPLICATION_VERSION);

    LogManager::initialize(application.applicationDirPath());

    ApplicationManager application_manager(application.arguments(), &application);
    application_manager.initialize();

    return application_manager.exec();
}

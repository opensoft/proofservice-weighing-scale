#include "proofservice-weighing-scale_global.h"
#include "weighingscalerestserver.h"

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"
#include "proofcore/updatemanager.h"

#include <QTimer>

Q_LOGGING_CATEGORY(proofServiceWeighingScaleLog, "proofservices.weighing-scale")

//TODO: make it cross-platform
int main(int argc, char *argv[])
{
    Proof::CoreApplication a(argc, argv, "Opensoft", "proofservice-weighing-scale", APP_VERSION);

    Proof::SettingsGroup *serverGroup = a.settings()->group("server", Proof::Settings::NotFoundPolicy::Add);
    int serverPort = serverGroup->value("port", 9000, Proof::Settings::NotFoundPolicy::Add).toInt();
    WeighingScaleRestServer server(serverPort);
    server.startListen();

    QObject::connect(a.updateManager(), &Proof::UpdateManager::updateSucceeded, &a, &QCoreApplication::quit);
    QTimer::singleShot(1, &a, &Proof::CoreApplication::postInit);

    return a.exec();
}

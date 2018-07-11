#ifndef WEIGHINGSCALERESTSERVER_H
#define WEIGHINGSCALERESTSERVER_H

#include "weighingscalehandler.h"

#include "proofcore/basic_package.h"

#include "proofnetwork/abstractrestserver.h"

class WeighingScaleHandler;
class QThread;

class WeighingScaleRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    explicit WeighingScaleRestServer(quint16 port);
    ~WeighingScaleRestServer();

protected slots:
    void rest_get_Weight_Instant(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                 const QUrlQuery &queryParams, const QByteArray &body);
    void rest_get_Weight_Stable(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                const QUrlQuery &queryParams, const QByteArray &body);

protected:
    FutureSP<Proof::HealthStatusMap> healthStatus(bool quick) const override;

private:
    QJsonObject stateToJson(const WeighingScaleHandler::State &state);
    WeighingScaleHandler *m_handler;
};

#endif // WEIGHINGSCALERESTSERVER_H

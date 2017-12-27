#ifndef WEIGHINGSCALERESTSERVER_H
#define WEIGHINGSCALERESTSERVER_H

#include "weighingscalehandler.h"

#include "proofnetwork/abstractrestserver.h"

class WeighingScaleHandler;
class QThread;

class WeighingScaleRestServer : public Proof::AbstractRestServer
{
    Q_OBJECT
public:
    explicit WeighingScaleRestServer(int port);
    ~WeighingScaleRestServer();

protected slots:
    void rest_get_Weight_Instant(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                 const QUrlQuery &queryParams, const QByteArray &body);
    void rest_get_Weight_Stable(QTcpSocket *socket, const QStringList &headers, const QStringList &methodVariableParts,
                                const QUrlQuery &queryParams, const QByteArray &body);
private:
    QJsonObject stateToJson(const WeighingScaleHandler::State &state);
    WeighingScaleHandler *m_handler;
};

#endif // WEIGHINGSCALERESTSERVER_H

#include "weighingscalerestserver.h"

#include "proofservice-weighing-scale_global.h"

#include "proofcore/coreapplication.h"
#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>

WeighingScaleRestServer::WeighingScaleRestServer(quint16 port) : Proof::AbstractRestServer(port)
{
    QString rawVendor = proofApp->settings()
                            ->mainGroup()
                            ->value("vendor_id", "0x0b67", Proof::Settings::NotFoundPolicy::Add)
                            .toString()
                            .trimmed();
    QString rawProduct = proofApp->settings()
                             ->mainGroup()
                             ->value("product_id", "0x555e", Proof::Settings::NotFoundPolicy::Add)
                             .toString()
                             .trimmed();
    if (rawVendor.startsWith("0x", Qt::CaseInsensitive))
        rawVendor.remove(0, 2);
    if (rawProduct.startsWith("0x", Qt::CaseInsensitive))
        rawProduct.remove(0, 2);
    m_handler = new WeighingScaleHandler(rawVendor.toUShort(nullptr, 16), rawProduct.toUShort(nullptr, 16));
    m_handler->moveToThread(qApp->thread());
    m_handler->start();
}

WeighingScaleRestServer::~WeighingScaleRestServer()
{
    m_handler->stop();
    m_handler->wait(1000);
    delete m_handler;
}

void WeighingScaleRestServer::rest_get_Weight_Instant(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                      const QUrlQuery &, const QByteArray &)
{
    sendAnswer(socket, QJsonDocument(stateToJson(m_handler->instantState())).toJson(QJsonDocument::Compact),
               "application/json", 200, "OK");
}

void WeighingScaleRestServer::rest_get_Weight_Stable(QTcpSocket *socket, const QStringList &, const QStringList &,
                                                     const QUrlQuery &queryParams, const QByteArray &)
{
    bool immediate = queryParams.hasQueryItem("immediate");
    if (immediate) {
        sendAnswer(socket, QJsonDocument(stateToJson(m_handler->lastStableState())).toJson(QJsonDocument::Compact),
                   "application/json", 200, "OK");
    } else {
        m_handler->execOnNextStableWeight([this, socket](WeighingScaleHandler::State state) {
            sendAnswer(socket, QJsonDocument(stateToJson(state)).toJson(QJsonDocument::Compact), "application/json",
                       200, "OK");
        });
    }
}

FutureSP<Proof::HealthStatusMap> WeighingScaleRestServer::healthStatus(bool) const
{
    QDateTime current = QDateTime::currentDateTimeUtc();
    return Future<Proof::HealthStatusMap>::successful(
        Proof::HealthStatusMap{{QStringLiteral("device_vendor_id"),
                                {current, QString("0x%1").arg(m_handler->vendorId(), 4, 16, QChar('0'))}},
                               {QStringLiteral("device_product_id"),
                                {current, QString("0x%1").arg(m_handler->productId(), 4, 16, QChar('0'))}},
                               {QStringLiteral("device_alive"), {current, m_handler->isAlive()}},
                               {QStringLiteral("msecs_since_last_read_from_device"),
                                {current, m_handler->elapsedSinceLastMessage()}}});
}

QJsonObject WeighingScaleRestServer::stateToJson(const WeighingScaleHandler::State &state)
{
    QJsonObject result;
    result.insert("weight", state.weight);
    result.insert("units", state.stringifiedUnit());
    result.insert("status", state.stringifiedStatus());
    return result;
}

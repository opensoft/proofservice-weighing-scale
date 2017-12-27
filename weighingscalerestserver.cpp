#include "weighingscalerestserver.h"

#include "proofservice-weighing-scale_global.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QTimer>

static std::atomic_ullong counter {0};

WeighingScaleRestServer::WeighingScaleRestServer(int port)
    : Proof::AbstractRestServer(port)
{
    m_handler = new WeighingScaleHandler(this);
    m_handler->start();
}

WeighingScaleRestServer::~WeighingScaleRestServer()
{
}

void WeighingScaleRestServer::rest_get_Weight_Instant(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &, const QByteArray &)
{
    auto state = m_handler->instantState();
    sendAnswer(socket, QJsonDocument(stateToJson(state)).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
}

void WeighingScaleRestServer::rest_get_Weight_Stable(QTcpSocket *socket, const QStringList &, const QStringList &, const QUrlQuery &queryParams, const QByteArray &)
{
    bool immediate = queryParams.hasQueryItem("immediate");
    if (immediate) {
        auto state = m_handler->lastStableState();
        sendAnswer(socket, QJsonDocument(stateToJson(state)).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
    } else {
        m_handler->execOnNextStableWeight([this, socket](const WeighingScaleHandler::State &state) {
            sendAnswer(socket, QJsonDocument(stateToJson(state)).toJson(QJsonDocument::Compact), "application/json", 200, "OK");
        });
    }
}

QJsonObject WeighingScaleRestServer::stateToJson(const WeighingScaleHandler::State &state)
{
    QJsonObject result;
    result.insert("weight", state.weight);
    result.insert("units", state.unit);
    switch (state.status) {
    case WeighingScaleHandler::Status::Fault:
    case WeighingScaleHandler::Status::UnknownStatus:
        result.insert("status", "error");
        break;
    case WeighingScaleHandler::Status::InMotion:
        result.insert("status", "in motion");
        break;
    case WeighingScaleHandler::Status::Overweight:
        result.insert("status", "overweight");
        break;
    case WeighingScaleHandler::Status::StableZero:
    case WeighingScaleHandler::Status::Stable:
        result.insert("status", "stable");
        break;
    case WeighingScaleHandler::Status::UnderZero:
        result.insert("status", "under zero");
        break;
    }
    return result;
}


/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: denis.kormalev@opensoftdev.com (Denis Kormalev)
 *
 */
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

QJsonObject WeighingScaleRestServer::stateToJson(WeighingScaleHandler::State state)
{
    QJsonObject result;
    result.insert("weight", state.weight);
    result.insert("units", state.stringifiedUnit());
    result.insert("status", state.stringifiedStatus());
    return result;
}

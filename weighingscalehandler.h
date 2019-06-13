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
#ifndef WEIGHINGSCALEHANDLER_H
#define WEIGHINGSCALEHANDLER_H

#include "3rdparty/hidapi/hidapi.h"

#include <QLinkedList>
#include <QReadWriteLock>
#include <QThread>
#include <QTime>

#include <atomic>
#include <functional>

class WeighingScaleHandler : public QThread
{
    Q_OBJECT
public:
    enum class Status
    {
        UnknownStatus = 0,
        Fault = 1,
        StableZero = 2,
        InMotion = 3,
        Stable = 4,
        UnderZero = 5,
        Overweight = 6,
        DummyValue = 7, // Should never happen, added to make it easier for boundaries check
        NotInitialized = 8
    };
    Q_ENUM(WeighingScaleHandler::Status)

    enum class Unit
    {
        UnknownUnit = 0,
        Milligram = 1,
        Gram = 2,
        Kilogram = 3,
        Carat = 4,
        Tael = 5,
        Grain = 6,
        Pennyweight = 7,
        MetricTon = 8,
        AvoirTon = 9,
        TroyOunce = 0xA,
        Ounce = 0xB,
        Pound = 0xC
    };
    Q_ENUM(WeighingScaleHandler::Unit)

    struct State
    {
        QString stringifiedStatus() const;
        QString stringifiedUnit() const;
        Status status;
        Unit unit;
        double weight;
    };

    explicit WeighingScaleHandler(unsigned short vendorId, unsigned short productId, QObject *parent = nullptr);

    State lastStableState() const;
    State instantState() const;
    void execOnNextStableWeight(std::function<void(State)> &&handler);

    bool isAlive() const;
    int elapsedSinceLastMessage() const;
    unsigned short vendorId() const;
    unsigned short productId() const;

    void stop();

protected:
    void run() override;

private:
    uint64_t packState(Status status, Unit unit, short weight, char scaleFactor) const;
    State extractState(uint64_t state) const;
    Status extractStateStatus(uint64_t state) const;

    unsigned short m_vendorId = 0;
    unsigned short m_productId = 0;

    bool m_stopped = false;

    std::atomic_ullong m_instantState{0};
    std::atomic_ullong m_lastStableState{0};

    QLinkedList<std::function<void(State)>> m_stableWaiters;
    QReadWriteLock m_stableWaitersLock;

    hid_device *m_hidHandle = nullptr;
    QTime m_lastSuccessfulRead;
};

uint qHash(WeighingScaleHandler::Unit value, uint seed = 0);
uint qHash(WeighingScaleHandler::Status value, uint seed = 0);

#endif // WEIGHINGSCALEHANDLER_H

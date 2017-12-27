#include "weighingscalehandler.h"

#include "proofservice-weighing-scale_global.h"

#include "proofcore/proofobject.h"

#include "3rdparty/hidapi/hidapi.h"

#include <math.h>

static const QHash<WeighingScaleHandler::Unit, QString> UNITS = {
    {WeighingScaleHandler::Unit::UnknownUnit, ""},
    {WeighingScaleHandler::Unit::Milligram, "mg"},
    {WeighingScaleHandler::Unit::Gram, "g"},
    {WeighingScaleHandler::Unit::Kilogram, "kg"},
    {WeighingScaleHandler::Unit::Carat, "ct"},
    {WeighingScaleHandler::Unit::Tael, "tael"},
    {WeighingScaleHandler::Unit::Grain, "gr"},
    {WeighingScaleHandler::Unit::Pennyweight, "dwt"},
    {WeighingScaleHandler::Unit::MetricTon, "ton"},
    {WeighingScaleHandler::Unit::AvoirTon, "short ton"},
    {WeighingScaleHandler::Unit::TroyOunce, "oz t"},
    {WeighingScaleHandler::Unit::Ounce, "oz"},
    {WeighingScaleHandler::Unit::Pound, "lb"}
};

static const QHash<WeighingScaleHandler::Status, QString> STATUSES = {
    {WeighingScaleHandler::Status::UnknownStatus, "error"},
    {WeighingScaleHandler::Status::Fault, "error"},
    {WeighingScaleHandler::Status::StableZero, "stable"},
    {WeighingScaleHandler::Status::InMotion, "in motion"},
    {WeighingScaleHandler::Status::Stable, "stable"},
    {WeighingScaleHandler::Status::UnderZero, "under zero"},
    {WeighingScaleHandler::Status::Overweight, "overweight"}
};

uint qHash(WeighingScaleHandler::Unit value, uint seed)
{
    return qHash(static_cast<int>(value), seed);
}

uint qHash(WeighingScaleHandler::Status value, uint seed)
{
    return qHash(static_cast<int>(value), seed);
}

WeighingScaleHandler::WeighingScaleHandler(QObject *parent)
    : QThread(parent)
{
}

WeighingScaleHandler::State WeighingScaleHandler::lastStableState() const
{
    return extractState(m_lastStableState);
}

WeighingScaleHandler::State WeighingScaleHandler::instantState() const
{
    return extractState(m_instantState);
}

void WeighingScaleHandler::execOnNextStableWeight(std::function<void(State)> &&handler)
{
    m_stableWaitersLock.lockForWrite();
    m_stableWaiters << handler;
    m_stableWaitersLock.unlock();
}

void WeighingScaleHandler::run()
{
    int hidResult = hid_init();
    if (hidResult < 0) {
        qCCritical(proofServiceWeighingScaleLog) << "HID Api can't be initialized, going down";
        return;
    }
    hid_device *hidHandle = hid_open(0x0b67, 0x555e, NULL);
    if (!hidHandle) {
        qCCritical(proofServiceWeighingScaleLog) << "HID device can't be opened, going down";
        return;
    }

    unsigned char data[6];
    memset(data, 0, 6);
    while (true) {
        hidResult = hid_read(hidHandle, data, 6);
        if (hidResult < 6)
            continue;
        if (data[0] != 3)
            continue;
        Status status = static_cast<Status>(qMin(data[1], static_cast<unsigned char>(Status::Overweight)));

        auto state = packState(status,
                               static_cast<Unit>(qMin(data[2], static_cast<unsigned char>(Unit::Pound))),
                               static_cast<short>((static_cast<unsigned short>(data[5]) << 8) | static_cast<unsigned short>(data[4])),
                               static_cast<char>(data[3]));
        m_instantState = state;

        if (status == Status::Stable || status == Status::UnderZero || status == Status::StableZero) {
            m_lastStableState = state;
            m_stableWaitersLock.lockForRead();
            bool waitersExist = m_stableWaiters.size();
            m_stableWaitersLock.unlock();
            if (waitersExist) {
                m_stableWaitersLock.lockForWrite();
                auto extractedState = extractState(state);
                while (!m_stableWaiters.isEmpty())
                    m_stableWaiters.takeFirst()(extractedState);
                m_stableWaitersLock.unlock();
            }
        }
    }
    hid_exit();
}

unsigned long long WeighingScaleHandler::packState(Status status,Unit unit, short weight, char scaleFactor) const
{
    unsigned long long result = static_cast<unsigned short>(weight);
    result |= static_cast<unsigned char>(scaleFactor) << 16;
    result |= static_cast<long long>(unit) << 24;
    result |= static_cast<long long>(status) << 32;
    return result;
}

WeighingScaleHandler::State WeighingScaleHandler::extractState(unsigned long long state) const
{
    double value = static_cast<double>(static_cast<short>(state & 0xFFFF)) * pow(10, static_cast<char>((state >> 16) & 0xFF));
    return State{static_cast<Status>((state >> 32) & 0xFF), static_cast<Unit>((state >> 24) & 0xFF), value};
}

QString WeighingScaleHandler::State::stringifiedStatus() const
{
    return STATUSES.value(status, "");
}

QString WeighingScaleHandler::State::stringifiedUnit() const
{
    return UNITS.value(unit, "");
}


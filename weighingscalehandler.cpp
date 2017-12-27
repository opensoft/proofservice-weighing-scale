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

uint qHash(WeighingScaleHandler::Unit value, uint seed)
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

void WeighingScaleHandler::execOnNextStableWeight(std::function<void (const State &)> &&handler)
{
    m_stableWaitersLock.lockForWrite();
    m_stableWaiters << handler;
    m_stableWaitersLock.unlock();
}

void WeighingScaleHandler::run()
{
    int res;
    unsigned char buf[6];
    wchar_t wstr[512];
    hid_device *handle;

    res = hid_init();
    handle = hid_open(0x0b67, 0x555e, NULL);
    qDebug() << res << handle;
    res = hid_get_manufacturer_string(handle, wstr, 512);
    qDebug() << QString::fromWCharArray(wstr);
    res = hid_get_product_string(handle, wstr, 512);
    qDebug() << QString::fromWCharArray(wstr);

    while (true) {
        memset(buf, 0, 6);
        res = hid_read(handle, buf, 6);
        if (buf[0] != 3)
            continue;
        Status status = static_cast<Status>(qMin(buf[1], static_cast<unsigned char>(Status::Overweight)));

        auto state = packState(status,
                               static_cast<Unit>(qMin(buf[2], static_cast<unsigned char>(Unit::Pound))),
                               static_cast<short>((static_cast<unsigned short>(buf[5]) << 8) | static_cast<unsigned short>(buf[4])),
                               static_cast<char>(buf[3]));
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

    res = hid_exit();
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
    return State{static_cast<Status>((state >> 32) & 0xFF), value, UNITS.value(static_cast<Unit>((state >> 24) & 0xFF), "")};
}



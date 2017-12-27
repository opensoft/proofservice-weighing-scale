#ifndef WEIGHINGSCALEHANDLER_H
#define WEIGHINGSCALEHANDLER_H

#include <QThread>
#include <QReadWriteLock>
#include <QLinkedList>

#include <atomic>
#include <functional>

class WeighingScaleHandler : public QThread
{
    Q_OBJECT
public:
    enum class Status {
        UnknownStatus = 0,
        Fault = 1,
        StableZero = 2,
        InMotion = 3,
        Stable = 4,
        UnderZero = 5,
        Overweight = 6
    };
    Q_ENUM(WeighingScaleHandler::Status)

    enum class Unit {
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

    struct State {
        QString stringifiedStatus() const;
        QString stringifiedUnit() const;
        Status status;
        Unit unit;
        double weight;
    };

    explicit WeighingScaleHandler(QObject *parent = nullptr);

    State lastStableState() const;
    State instantState() const;

    void execOnNextStableWeight(std::function<void(State)> &&handler);

protected:
    void run() override;

private:
    unsigned long long packState(Status status, Unit unit, short weight, char scaleFactor) const;
    State extractState(unsigned long long state) const;

    std::atomic_ullong m_instantState {0};
    std::atomic_ullong m_lastStableState {0};

    QLinkedList<std::function<void(State)>> m_stableWaiters;
    QReadWriteLock m_stableWaitersLock;
};

uint qHash(WeighingScaleHandler::Unit value, uint seed = 0);
uint qHash(WeighingScaleHandler::Status value, uint seed = 0);

#endif // WEIGHINGSCALEHANDLER_H

#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <QObject>
#include <QTimer>

class CircuitScene;

enum class SimulationState {
    Stopped,
    Running,
    Paused
};

// مسئولیت این کلاس فقط اجرای چرخه شبیه‌سازی دیجیتال است (بخش ۸ مستند پروژه:
// Run/Pause/Stop و اجرای گام‌به‌گام). هیچ چیزی درباره UI نمی‌داند؛ MainWindow
// فقط با stateChanged() و متدهای عمومی این کلاس صحبت می‌کند.
class SimulationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SimulationEngine(CircuitScene *scene, QObject *parent = nullptr);

    void start(); // اجرای پیوسته
    void pause(); // مکث (وضعیت فعلی مدار حفظ می‌شود)
    void stop();  // توقف کامل و بازنشانی مدار به حالت اولیه
    void step();  // فقط یک گام شبیه‌سازی جلو برو (بخش ۸.۴)

    SimulationState state() const { return m_state; }

signals:
    void stateChanged(SimulationState state);

private slots:
    void tick();

private:
    void resetAllComponents();
    void checkNodeConflicts();

    CircuitScene *m_scene;
    QTimer m_timer;
    SimulationState m_state = SimulationState::Stopped;
};

#endif // SIMULATIONENGINE_H

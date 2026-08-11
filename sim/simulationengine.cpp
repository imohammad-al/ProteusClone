#include "simulationengine.h"
#include "../circuitscene.h"
#include "../component.h"
#include "../digitalcomponent.h"
#include "../measurementtools.h"
#include "../node.h"
#include "../passives.h"
#include "../semiconductors.h"
#include "simulationlogger.h"
#include "analogsolver.h"

SimulationEngine::SimulationEngine(CircuitScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene)
{
    // هر گام شبیه‌سازی ۱۵۰ میلی‌ثانیه طول می‌کشد تا انتشار تاخیر بین گیت‌ها با چشم قابل دیدن باشد
    m_timer.setInterval(150);
    connect(&m_timer, &QTimer::timeout, this, &SimulationEngine::tick);
}

void SimulationEngine::start()
{
    if (m_state == SimulationState::Stopped) {
        resetAllComponents();
        SimulationLogger::instance().log(LogLevel::Info, tr("شبیه‌سازی شروع شد."));
    }

    m_state = SimulationState::Running;
    m_timer.start();
    emit stateChanged(m_state);
}

void SimulationEngine::pause()
{
    m_timer.stop();
    m_state = SimulationState::Paused;
    SimulationLogger::instance().log(LogLevel::Info, tr("شبیه‌سازی متوقف موقت شد."));
    emit stateChanged(m_state);
}

void SimulationEngine::stop()
{
    m_timer.stop();
    m_state = SimulationState::Stopped;

    resetAllComponents();
    if (m_scene)
        m_scene->update();

    SimulationLogger::instance().log(LogLevel::Info, tr("شبیه‌سازی متوقف شد."));
    emit stateChanged(m_state);
}

void SimulationEngine::step()
{
    if (m_state == SimulationState::Stopped) {
        resetAllComponents();
        m_state = SimulationState::Paused; // بعد از یک گام دستی، اجرای پیوسته شروع نمی‌شود
        emit stateChanged(m_state);
    }
    tick();
}

void SimulationEngine::tick()
{
    if (!m_scene) return;

    // تحلیل آنالوگ DC (بخش ۶.۱/۶.۲ مستند: Resistor/DC Source/Ground) قبل از
    // چرخه دیجیتال حل می‌شود تا AnalogVoltmeter/AnalogAmmeter در همین تیک مقدار
    // تازه ببینند. مداری کاملاً دیجیتالی هیچ هزینه‌ای اینجا ندارد (analogsolver.h
    // فوراً true برمی‌گرداند چون هیچ Resistor/DC Sourceای پیدا نمی‌کند).
    QString analogError;
    if (!AnalogSolver::solve(m_scene, &analogError) && !analogError.isEmpty()) {
        SimulationLogger::instance().log(LogLevel::Warning, analogError);
    }

    const QList<Component*> comps = m_scene->components();
    for (Component *c : comps) {
        if (DigitalComponent *dc = dynamic_cast<DigitalComponent*>(c))
            dc->simulationTick();
    }

    for (Component *c : comps) {
        if (Oscilloscope *scope = dynamic_cast<Oscilloscope*>(c))
            scope->sampleChannels();
    }

    checkNodeConflicts();

    m_scene->update(); // رنگ زنده سیم‌ها و LED ها را به‌روزرسانی کن (بخش ۸.۲)
}

void SimulationEngine::resetAllComponents()
{
    if (!m_scene) return;

    const QList<Component*> comps = m_scene->components();
    for (Component *c : comps) {
        if (DigitalComponent *dc = dynamic_cast<DigitalComponent*>(c))
            dc->resetSimulation();
        if (Oscilloscope *scope = dynamic_cast<Oscilloscope*>(c))
            scope->clearHistory();
        if (AnalogAmmeter *amm = dynamic_cast<AnalogAmmeter*>(c))
            amm->clearMeasurement();
        if (Capacitor *cap = dynamic_cast<Capacitor*>(c))
            cap->resetTransientState();
        if (Inductor *ind = dynamic_cast<Inductor*>(c))
            ind->resetTransientState();
        if (Diode *diode = dynamic_cast<Diode*>(c))
            diode->resetTransientState();
    }

    // ولتاژهای آنالوگ تیک قبلی هم پاک شوند تا AnalogVoltmeter/AnalogAmmeter
    // بلافاصله بعد از Stop به‌جای نشان دادن مقدار کهنه، ERR/-- نشان دهند.
    const QList<Node*> nodeList = m_scene->nodes();
    for (Node *n : nodeList)
        if (n) n->clearVoltage();
}

void SimulationEngine::checkNodeConflicts()
{
    if (!m_scene) return;

    const QList<Node*> nodeList = m_scene->nodes();
    for (Node *node : nodeList) {
        bool conflict = false;
        node->resolvedValue(&conflict);
        if (conflict) {
            SimulationLogger::instance().log(LogLevel::Error,
                tr("تداخل سطح منطقی: بیش از یک خروجی با مقادیر متفاوت روی یک گره قرار گرفته است."));
        }
    }
}

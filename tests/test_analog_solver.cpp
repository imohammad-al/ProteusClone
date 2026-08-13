// تست مستقل خارج از GUI (فاز ۱۶): چند مدار واقعی را با همان کلاس‌های تولید
// (Component/Pin/Node/CircuitScene/AnalogSolver) می‌سازد و نتیجه را با حل
// مستقل همان معادلات در پایتون (scipy.fsolve) مقایسه می‌کند. هیچ کد
// شبیه‌سازی‌ای اینجا بازنویسی نشده - این دقیقاً همان مسیر کدی است که GUI
// واقعی هنگام فشردن Run طی می‌کند.
#include <QApplication>
#include <cmath>
#include <cstdio>

#include "circuitscene.h"
#include "sources.h"
#include "passives.h"
#include "semiconductors.h"
#include "sim/analogsolver.h"

static bool nearlyEqual(double a, double b, double relTol, double absTol) {
    return std::fabs(a - b) <= absTol + relTol * std::fabs(b);
}

// --- تست ۱+۲: مدار بایاس‌ثابت NPN (فعال معمولی و اشباع) ---
static bool runNpnFixedBiasTest(const char *label, double vccVolts, double rbOhms, double rcOhms,
                                 double expVBase, double expVColl, double expIb, double expIc) {
    CircuitScene scene;
    // یکپارچه‌سازی: AnalogSolver از componentsInSchematic() استفاده می‌کند نه
    // components() خام (به circuitscene.h/cpp و sim/analogsolver.cpp نگاه
    // کنید) - در برنامه‌ی واقعی MainWindow همیشه یک کادر شماتیک معتبر تنظیم
    // می‌کند، ولی این تست مستقل هیچ‌وقت از MainWindow عبور نمی‌کند. قطعات این
    // تست هیچ‌وقت setPos نمی‌شوند و دقیقاً روی مبدأ (۰,۰) می‌مانند؛ چون
    // boundingRect هر قطعه حول مبدأ محلی‌اش متقارن (نه لزوماً از ۰,۰ شروع)
    // است، کادر شماتیک باید حول مبدأ صحنه هم مرکز باشد وگرنه مرکز برخی قطعات
    // (مثل ترانزیستور/Ground) درست روی لبه کادر می‌افتد و بیرون از آن حساب
    // می‌شود.
    scene.setSchematicRect(QRectF(-50000, -50000, 100000, 100000));
    Ground *gnd = new Ground();
    DCVoltage *vcc = new DCVoltage();
    vcc->setProperty("voltage", vccVolts);
    Resistor *rb = new Resistor();
    rb->setProperty("resistance", rbOhms);
    Resistor *rc = new Resistor();
    rc->setProperty("resistance", rcOhms);
    TransistorNPN *q1 = new TransistorNPN();
    // پارامترهای پیش‌فرض سازنده: Is=1e-14, n=1, betaF=200, betaR=4

    scene.addItem(gnd); scene.addItem(vcc); scene.addItem(rb); scene.addItem(rc); scene.addItem(q1);

    scene.connectPins(vcc->pin(1), gnd->pin(0));
    scene.connectPins(vcc->pin(0), rb->pin(0));
    scene.connectPins(vcc->pin(0), rc->pin(0));
    scene.connectPins(rb->pin(1), q1->pin(0));
    scene.connectPins(rc->pin(1), q1->pin(1));
    scene.connectPins(q1->pin(2), gnd->pin(0));

    QString err;
    if (!AnalogSolver::solve(&scene, &err)) {
        std::printf("[%s] SOLVE FAILED: %s\n", label, err.toUtf8().constData());
        return false;
    }

    Node *nVcc = vcc->pin(0)->node();
    Node *nBase = q1->pin(0)->node();
    Node *nColl = q1->pin(1)->node();
    Node *nEmit = q1->pin(2)->node();
    if (!nVcc || !nBase || !nColl || !nEmit
        || !nVcc->hasVoltage() || !nBase->hasVoltage() || !nColl->hasVoltage() || !nEmit->hasVoltage()) {
        std::printf("[%s] TOPOLOGY/VOLTAGE ERROR\n", label);
        return false;
    }

    const double vBase = nBase->voltage();
    const double vColl = nColl->voltage();
    const double vEmit = nEmit->voltage();
    const double ib = (vcc->property("voltage").toDouble() - vBase) / rbOhms;
    const double ic = (vcc->property("voltage").toDouble() - vColl) / rcOhms;

    bool pass = true;
    pass &= nearlyEqual(vBase, expVBase, 1e-4, 1e-4);
    pass &= nearlyEqual(vColl, expVColl, 1e-4, 1e-3);
    pass &= nearlyEqual(ib, expIb, 1e-4, 1e-9);
    pass &= nearlyEqual(ic, expIc, 1e-4, 1e-7);
    pass &= nearlyEqual(vEmit, 0.0, 0.0, 1e-9);
    pass &= nearlyEqual(ib + ic, (nVcc->voltage() - vColl)/rcOhms + (nVcc->voltage()-vBase)/rbOhms, 0.0, 1e-15);

    std::printf("[%s] Vbase=%.9f (exp %.9f)  Vcoll=%.9f (exp %.9f)\n", label, vBase, expVBase, vColl, expVColl);
    std::printf("[%s] Ib=%.6fuA (exp %.6f)  Ic=%.6fmA (exp %.6f)  => %s\n\n",
                label, ib*1e6, expIb*1e6, ic*1e3, expIc*1e3, pass ? "PASS" : "FAIL");
    return pass;
}

// --- تست ۳: رگرسیون دیود تکی (فاز ۱۴) - باید بعد از اضافه‌شدن گام‌محدودسازی
// نیوتن (فاز ۱۶) هنوز درست کار کند. مدار: منبع ۵ ولت + مقاومت ۲۲۰ اهم + دیود
// به زمین. مقدار مرجع مستقل: حل معادله شاکلی با نیوتن ساده در پایتون. ---
static bool runDiodeRegressionTest() {
    CircuitScene scene;
    scene.setSchematicRect(QRectF(-50000, -50000, 100000, 100000)); // نگاه کنید به توضیح بالا
    Ground *gnd = new Ground();
    DCVoltage *v = new DCVoltage();
    v->setProperty("voltage", 5.0);
    Resistor *r = new Resistor();
    r->setProperty("resistance", 220.0);
    Diode *d = new Diode();
    // پارامترهای پیش‌فرض سازنده دیود (فاز ۱۴): Is=4.352e-9, n=1.906 (مطابق کد موجود)

    scene.addItem(gnd); scene.addItem(v); scene.addItem(r); scene.addItem(d);

    scene.connectPins(v->pin(1), gnd->pin(0));
    scene.connectPins(v->pin(0), r->pin(0));
    scene.connectPins(r->pin(1), d->pin(0));   // anode
    scene.connectPins(d->pin(1), gnd->pin(0)); // cathode -> GND

    QString err;
    if (!AnalogSolver::solve(&scene, &err)) {
        std::printf("[diode-regression] SOLVE FAILED: %s\n", err.toUtf8().constData());
        return false;
    }
    Node *nAnode = d->pin(0)->node();
    if (!nAnode || !nAnode->hasVoltage()) {
        std::printf("[diode-regression] TOPOLOGY/VOLTAGE ERROR\n");
        return false;
    }
    const double vd = nAnode->voltage(); // کاتد زمین است، پس ولتاژ آند = Vd
    const double idFromR = (5.0 - vd) / 220.0;

    // مرجع مستقل: پایتون (scipy.fsolve روی همون معادله شاکلی) خارج از این فایل حل شده:
    // Is=4.352e-9, n=1.906, VT=0.02585, R=220 -> Vd=0.600010588V, Id=19.999952mA
    const double expVd = 0.754077020;
    const double expId = 0.019299650;
    bool pass = true;
    pass &= nearlyEqual(vd, expVd, 1e-3, 2e-3);
    pass &= nearlyEqual(idFromR, expId, 1e-3, 5e-5);
    std::printf("[diode-regression] Vd=%.6f (exp %.6f)  Id=%.6fmA (exp %.6f) => %s\n\n",
                vd, expVd, idFromR*1e3, expId*1e3, pass ? "PASS" : "FAIL");
    return pass;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    bool allPass = true;

    allPass &= runNpnFixedBiasTest("active-region 9V", 9.0, 430000.0, 1000.0,
                                    0.689818551, 5.134799326, 19.326003e-6, 3.865201e-3);

    allPass &= runNpnFixedBiasTest("saturation 9V", 9.0, 10000.0, 100.0,
                                    0.771427704, 0.100877191, 822.8572e-6, 88.9912e-3);

    allPass &= runNpnFixedBiasTest("active-region 5V", 5.0, 220000.0, 1000.0,
                                    0.690169459, 1.081972235, 19.5901e-6, 3.9180e-3);

    allPass &= runDiodeRegressionTest();

    std::printf("=== OVERALL: %s ===\n", allPass ? "ALL TESTS PASS" : "SOME TESTS FAILED");
    return allPass ? 0 : 1;
}

#include "analogsolver.h"
#include "../circuitscene.h"
#include "../component.h"
#include "../node.h"
#include "../pin.h"
#include "../measurementtools.h"
#include "../dac.h"
#include "../passives.h"
#include "../semiconductors.h"

#include <QHash>
#include <QSet>
#include <QVector>
#include <QStringList>
#include <QObject>
#include <QtGlobal>
#include <cmath>
#include <utility>

namespace {

// گام زمانی ثابت هر تیک برای گسسته‌سازی خازن/سلف (روش Backward Euler) - فاز ۱۳.
// این «زمان شبیه‌سازی» کاملاً مستقل از فاصله واقعی تایمر (۱۵۰ میلی‌ثانیه، برای
// واکنش‌پذیری UI) است؛ یک ساده‌سازی رایج در این‌جور شبیه‌سازها: به هر تیک،
// صرف‌نظر از سرعت واقعی اجرا، دقیقاً همین مقدار «زمان مداری» نسبت داده می‌شود.
constexpr double kAnalogTimeStepSeconds = 0.001; // ۱ میلی‌ثانیه شبیه‌سازی‌شده در هر تیک

// --- ثابت‌های تحلیل غیرخطی دیود (فاز ۱۴، روش نیوتن-رافسون) ---
constexpr double kThermalVoltage = 0.02585;   // ولت، kT/q تقریبی در دمای اتاق
constexpr double kDiodeVoltageClamp = 1.5;    // ولت - سقف ایمن ارزیابی نمایی (جلوگیری از سرریز exp())
constexpr double kDiodeVoltageFloor = -40.0;  // ولت - کف منطقی برای حالت بایاس معکوس
// فاز ۱۶: با اضافه‌شدن limitNewtonVoltageStep (هر تکرار حداکثر ۱۰*n*VT≈۰.۲۵۸۵
// ولت جابه‌جا می‌شود، برای پایداری عددی)، رسیدن از حدس اولیه صفر به یک پیوند
// عمیقاً معکوس‌بایاس (مثلاً VBC نزدیک kDiodeVoltageFloor در ترانزیستوری با
// منبع تغذیه بزرگ) می‌تونه به بیش از صد قدم نیاز داشته باشه؛ این عدد را با
// حاشیه‌ی امن بالا بردم (هر تکرار فقط یک حل خطی کوچک است، هزینه‌اش ناچیزه).
constexpr int kMaxNewtonIterations = 300;
constexpr double kNewtonToleranceVolts = 1e-9;

// یک عنصر مقاومتی بین دو گره (خطی، طبق قانون اهم)
struct ResistorElement {
    Node *a;
    Node *b;
    double resistance;
};

// یک عنصر «منبع ولتاژ ایده‌آل» بین دو گره - هم DC Source/Battery واقعی و هم
// AnalogAmmeter/DAC (که با ولتاژشون مدل می‌شن تا جریان/ولتاژ به‌دست بیاد) از
// این ساختار استفاده می‌کنند. ammeterOwner فقط برای حالت Ammeter پر می‌شود.
struct SourceElement {
    Node *pos;
    Node *neg;
    double voltage;
    AnalogAmmeter *ammeterOwner;
};

// مدل معادل نورتن خازن (Backward Euler): مقاومت معادل Δt/C به‌علاوه یک منبع
// جریان تاریخچه بر پایه ولتاژ تیک قبلی.
struct CapacitorElement {
    Node *a;
    Node *b;
    double capacitance;
    Capacitor *owner;
};

// مدل معادل سلف (Backward Euler): یک مجهول جریان کمکی مثل منبع ولتاژ، به‌علاوه
// یک جمله خودی (self) در ماتریس و یک جمله تاریخچه در سمت راست معادله.
struct InductorElement {
    Node *a;
    Node *b;
    double inductance;
    Inductor *owner;
};

// دیود (معادله شاکلی، حل غیرخطی با تکرار نیوتن-رافسون - فاز ۱۴). مثل خازن،
// مجهول جدیدی به دستگاه معادلات اضافه نمی‌کند؛ فقط یک رسانایی دیفرانسیلی +
// منبع جریان معادل نورتن است که هر تکرار نیوتن دوباره محاسبه می‌شود.
struct DiodeElement {
    Node *anode;
    Node *cathode;
    double saturationCurrent;
    double idealityFactor;
    Diode *owner;
};

// ترانزیستور NPN (مدل کامل Ebers-Moll، حل غیرخطی با همان نیوتن-رافسون - فاز
// ۱۶). مثل دیود، هیچ مجهول جدیدی به دستگاه اضافه نمی‌کند - فقط ترکیبی از دو
// «دیود» پایه‌محور (B-E و B-C) به‌علاوه دو منبع جریان وابسته (alphaF*I_DE از
// کلکتور به بیس، alphaR*I_DC از امیتر به بیس) که هر تکرار نیوتن دوباره
// محاسبه می‌شوند - جزئیات مشتق کامل در کامنت بالای بخش استمپ‌گذاری پایین.
struct TransistorElement {
    Node *base;
    Node *collector;
    Node *emitter;
    double saturationCurrent;
    double idealityFactor;
    double alphaForward;
    double alphaReverse;
    TransistorNPN *owner;
};

// جریان دیود I_d(Vd) و رسانایی دیفرانسیلی dI/dV را در ولتاژ داده‌شده حساب
// می‌کند. Vd قبل از ارزیابی نمایی به kDiodeVoltageClamp کلمپ می‌شود تا exp()
// هرگز سرریز نکند (دیودهای واقعی عملاً هیچ‌وقت این‌قدر بایاس مستقیم نمی‌بینند).
void diodeCurrentAndConductance(double vd, double is, double n,
                                 double *outCurrent, double *outConductance)
{
    const double vtN = n * kThermalVoltage;
    const double vdSafe = qMin(vd, kDiodeVoltageClamp);
    const double expTerm = std::exp(vdSafe / vtN);
    *outCurrent = is * (expTerm - 1.0);
    *outConductance = (is / vtN) * expTerm;
}

// محدودسازی گام نیوتن-رافسون (تکنیک استاندارد SPICE برای همگرایی پیوندهای
// نمایی) - باگ واقعی که فقط با اولین کامپایل و اجرای واقعی این پروژه کشف شد
// (فاز ۱۶): بدون این تابع، وقتی حدس اولیه صفر باشد و مقاومت سری بزرگ باشد
// (مثلاً مقاومت بیس چند صدکیلواُمی)، همان تکرار اول ولتاژ را مستقیم به سقف
// kDiodeVoltageClamp می‌پراند؛ در آنجا رسانایی دیفرانسیلی نمایی به مقادیر
// نجومی (۱۰^۱۲ به بالا) می‌رسد که در کنار رسانایی معمولی مقاومت‌های مدار
// (۱۰^-۶ تا ۱) یک ماتریس به‌شدت بدشرط می‌سازد و باعث نوسان دائمی/واگرایی
// می‌شود (هیچ‌وقت هم‌گرا نمی‌شود) - این را با اجرای واقعی کد روی یک مدار
// ترانزیستوری کشف کردم، نه با بازبینی دستی. راه‌حل: تغییر هر گام حداکثر به
// اندازه چند برابر ولتاژ حرارتی محدود می‌شود (اینجا 10*n*VT، مطابق قاعده
// رایج SPICE) تا رسانایی هرگز در یک جهش بین دو تکرار منفجر نشود - نیوتن هنوز
// دقیقاً به همان جواب می‌رسد، فقط با گام‌های کوچک‌تر و پایدار.
double limitNewtonVoltageStep(double proposedVoltage, double previousGuess, double idealityFactor)
{
    const double maxStep = 10.0 * idealityFactor * kThermalVoltage;
    const double delta = proposedVoltage - previousGuess;
    if (delta > maxStep) return previousGuess + maxStep;
    if (delta < -maxStep) return previousGuess - maxStep;
    return proposedVoltage;
}

// Union-Find ساده روی Node* برای شناسایی زیرمدارهای جدا از هم روی یک صحنه.
class UnionFind {
public:
    Node *find(Node *n)
    {
        if (!m_parent.contains(n)) {
            m_parent.insert(n, n);
            return n;
        }
        Node *root = n;
        while (m_parent.value(root) != root)
            root = m_parent.value(root);

        Node *cur = n;
        while (cur != root) {
            Node *next = m_parent.value(cur);
            m_parent.insert(cur, root);
            cur = next;
        }
        return root;
    }

    void unite(Node *a, Node *b)
    {
        Node *ra = find(a);
        Node *rb = find(b);
        if (ra != rb)
            m_parent.insert(ra, rb);
    }

private:
    QHash<Node *, Node *> m_parent;
};

// حل دستگاه معادلات خطی A*x=b با حذف گاؤسی + Pivoting جزئی (برای پایداری عددی).
// در صورت تکین (singular) بودن ماتریس - یعنی زیرمدار قابل حل نیست - false
// برمی‌گرداند و xOut را دست‌نخورده می‌گذارد.
bool solveLinearSystem(QVector<QVector<double>> A, QVector<double> b, QVector<double> *xOut)
{
    const int n = b.size();
    if (n == 0) {
        xOut->clear();
        return true;
    }

    for (int col = 0; col < n; ++col) {
        int pivotRow = col;
        double bestAbs = std::fabs(A[col][col]);
        for (int row = col + 1; row < n; ++row) {
            const double v = std::fabs(A[row][col]);
            if (v > bestAbs) {
                bestAbs = v;
                pivotRow = row;
            }
        }

        constexpr double kEpsilon = 1e-12;
        if (bestAbs < kEpsilon)
            return false; // ماتریس تکین

        if (pivotRow != col) {
            std::swap(A[pivotRow], A[col]);
            std::swap(b[pivotRow], b[col]);
        }

        for (int row = 0; row < n; ++row) {
            if (row == col)
                continue;
            const double factor = A[row][col] / A[col][col];
            if (factor == 0.0)
                continue;
            for (int c = col; c < n; ++c)
                A[row][c] -= factor * A[col][c];
            b[row] -= factor * b[col];
        }
    }

    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i)
        x[i] = b[i] / A[i][i];
    *xOut = x;
    return true;
}

} // namespace

bool AnalogSolver::solve(CircuitScene *scene, QString *errorMessage)
{
    if (!scene)
        return true; // چیزی برای حل کردن نیست، این یک خطا نیست

    // اول همه گره‌های شناخته‌شده صحنه را از حالت آنالوگ تیک قبلی پاک کن، تا اگر
    // این‌بار حل نشدند، مقدار کهنه/گمراه‌کننده روی نمایشگرها نماند.
    const QList<Node *> allNodes = scene->nodes();
    for (Node *n : allNodes)
        if (n)
            n->clearVoltage();

    QVector<ResistorElement> resistors;
    QVector<SourceElement> sources;
    QVector<CapacitorElement> capacitors;
    QVector<InductorElement> inductors;
    QVector<DiodeElement> diodes;
    QVector<TransistorElement> transistors;
    QSet<Node *> groundNodes;
    UnionFind uf;

    const QList<Component *> comps = scene->components();
    for (Component *c : comps) {
        if (!c)
            continue;
        const QString type = c->componentType();

        if (type == QLatin1String("Ground")) {
            if (Pin *p = c->pin(0)) {
                if (Node *n = p->node()) {
                    groundNodes.insert(n);
                    uf.find(n);
                }
            }
        } else if (type == QLatin1String("Resistor")) {
            Pin *p1 = c->pin(0);
            Pin *p2 = c->pin(1);
            Node *n1 = p1 ? p1->node() : nullptr;
            Node *n2 = p2 ? p2->node() : nullptr;
            if (n1 && n2) {
                const double r = c->property("resistance").toDouble();
                if (r > 0.0) {
                    resistors.append(ResistorElement{n1, n2, r});
                    uf.unite(n1, n2);
                }
                // مقاومت نامعتبر (<=0) عمداً بی‌سروصدا نادیده گرفته می‌شود؛ این
                // خیلی زودتر (هنگام ویرایش ویژگی) باید جلوگیری بشه، نه اینجا.
            }
        } else if (type == QLatin1String("Capacitor")) {
            auto *cap = dynamic_cast<Capacitor *>(c);
            Pin *p1 = c->pin(0);
            Pin *p2 = c->pin(1);
            Node *n1 = p1 ? p1->node() : nullptr;
            Node *n2 = p2 ? p2->node() : nullptr;
            if (cap && n1 && n2) {
                const double cf = c->property("capacitance").toDouble();
                if (cf > 0.0) {
                    capacitors.append(CapacitorElement{n1, n2, cf, cap});
                    uf.unite(n1, n2);
                }
            }
        } else if (type == QLatin1String("Inductor")) {
            auto *ind = dynamic_cast<Inductor *>(c);
            Pin *p1 = c->pin(0);
            Pin *p2 = c->pin(1);
            Node *n1 = p1 ? p1->node() : nullptr;
            Node *n2 = p2 ? p2->node() : nullptr;
            if (ind && n1 && n2) {
                const double lh = c->property("inductance").toDouble();
                if (lh > 0.0) {
                    inductors.append(InductorElement{n1, n2, lh, ind});
                    uf.unite(n1, n2);
                }
            }
        } else if (type == QLatin1String("Diode")) {
            auto *diode = dynamic_cast<Diode *>(c);
            Pin *pAnode = c->pin(0);
            Pin *pCathode = c->pin(1);
            Node *nAnode = pAnode ? pAnode->node() : nullptr;
            Node *nCathode = pCathode ? pCathode->node() : nullptr;
            if (diode && nAnode && nCathode) {
                const double is = c->property("saturationCurrent").toDouble();
                const double n = c->property("idealityFactor").toDouble();
                if (is > 0.0 && n > 0.0) {
                    diodes.append(DiodeElement{nAnode, nCathode, is, n, diode});
                    uf.unite(nAnode, nCathode);
                }
            }
        } else if (type == QLatin1String("Transistor NPN")) {
            auto *bjt = dynamic_cast<TransistorNPN *>(c);
            Pin *pBase = c->pin(0);
            Pin *pCollector = c->pin(1);
            Pin *pEmitter = c->pin(2);
            Node *nBase = pBase ? pBase->node() : nullptr;
            Node *nCollector = pCollector ? pCollector->node() : nullptr;
            Node *nEmitter = pEmitter ? pEmitter->node() : nullptr;
            if (bjt && nBase && nCollector && nEmitter) {
                const double is = c->property("saturationCurrent").toDouble();
                const double n = c->property("idealityFactor").toDouble();
                const double betaF = c->property("forwardBeta").toDouble();
                const double betaR = c->property("reverseBeta").toDouble();
                if (is > 0.0 && n > 0.0 && betaF > 0.0 && betaR > 0.0) {
                    const double alphaF = betaF / (betaF + 1.0);
                    const double alphaR = betaR / (betaR + 1.0);
                    transistors.append(TransistorElement{nBase, nCollector, nEmitter,
                                                           is, n, alphaF, alphaR, bjt});
                    uf.unite(nBase, nCollector);
                    uf.unite(nBase, nEmitter);
                }
                // پارامتر نامعتبر (<=0) مثل مقاومت نامعتبر، بی‌سروصدا نادیده گرفته
                // می‌شود - باید هنگام ویرایش ویژگی جلوگیری بشه، نه اینجا.
            }
        } else if (type == QLatin1String("DC Source") || type == QLatin1String("Battery")) {
            // Battery در این فاز مثل یک منبع ولتاژ ایده‌آل حل می‌شود؛ ویژگی
            // "internalResistance" هنوز اینجا اعمال نمی‌شود (به sources.h مراجعه کن).
            Pin *pPos = c->pin(0);
            Pin *pNeg = c->pin(1);
            Node *nPos = pPos ? pPos->node() : nullptr;
            Node *nNeg = pNeg ? pNeg->node() : nullptr;
            if (nPos && nNeg) {
                const double v = c->property("voltage").toDouble();
                sources.append(SourceElement{nPos, nNeg, v, nullptr});
                uf.unite(nPos, nNeg);
            }
        } else if (type == QLatin1String("Analog Ammeter")) {
            auto *amm = dynamic_cast<AnalogAmmeter *>(c);
            Pin *p1 = c->pin(0);
            Pin *p2 = c->pin(1);
            Node *n1 = p1 ? p1->node() : nullptr;
            Node *n2 = p2 ? p2->node() : nullptr;
            if (amm && n1 && n2) {
                sources.append(SourceElement{n1, n2, 0.0, amm});
                uf.unite(n1, n2);
            }
        } else if (type == QLatin1String("DAC")) {
            // ولتاژ DAC برخلاف DC Source/Battery یک ویژگی ثابت نیست؛ هر تیک از
            // روی ورودی دیجیتالش محاسبه می‌شود (به dac.h مراجعه کن). به همین
            // خاطر با dynamic_cast مستقیم خوانده می‌شود، نه از روی property().
            auto *dac = dynamic_cast<DAC *>(c);
            Pin *pPlus = c->pin(10);
            Pin *pMinus = c->pin(11);
            Node *nPlus = pPlus ? pPlus->node() : nullptr;
            Node *nMinus = pMinus ? pMinus->node() : nullptr;
            if (dac && dac->hasOutputVoltage() && nPlus && nMinus) {
                sources.append(SourceElement{nPlus, nMinus, dac->outputVoltage(), nullptr});
                uf.unite(nPlus, nMinus);
            }
            // اگر DAC هنوز ولتاژ معتبری نداشته باشد (مثلاً Vref وصل نیست)، عمداً
            // نادیده گرفته می‌شود - Vout+ اش مثل یک پایه معمولی رها می‌ماند.
        }
    }

    if (resistors.isEmpty() && sources.isEmpty() && capacitors.isEmpty()
        && inductors.isEmpty() && diodes.isEmpty() && transistors.isEmpty())
        return true; // مداری کاملاً دیجیتالی - چیزی آنالوگ برای حل کردن نیست

    for (Node *g : groundNodes)
        uf.find(g); // مطمئن شو یک Ground تنها (بدون هیچ مقاومت/منبعی) هم دیده می‌شود

    // تمام گره‌های درگیر را بر اساس ریشه Union-Find گروه‌بندی کن
    QSet<Node *> touched;
    for (const ResistorElement &r : resistors)    { touched.insert(r.a); touched.insert(r.b); }
    for (const SourceElement &s : sources)        { touched.insert(s.pos); touched.insert(s.neg); }
    for (const CapacitorElement &cp : capacitors) { touched.insert(cp.a); touched.insert(cp.b); }
    for (const InductorElement &ind : inductors)  { touched.insert(ind.a); touched.insert(ind.b); }
    for (const DiodeElement &d : diodes)          { touched.insert(d.anode); touched.insert(d.cathode); }
    for (const TransistorElement &t : transistors) {
        touched.insert(t.base); touched.insert(t.collector); touched.insert(t.emitter);
    }
    for (Node *g : groundNodes) touched.insert(g);

    QHash<Node *, QList<Node *>> groups;
    for (Node *n : touched)
        groups[uf.find(n)].append(n);

    bool allGroupsOk = true;
    QStringList failureReasons;

    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        const Node *groupRoot = it.key();
        const QList<Node *> &groupNodes = it.value();

        QSet<Node *> localGround;
        for (Node *n : groupNodes)
            if (groundNodes.contains(n))
                localGround.insert(n);

        QVector<ResistorElement> localResistors;
        for (const ResistorElement &r : resistors)
            if (uf.find(r.a) == groupRoot)
                localResistors.append(r);

        QVector<CapacitorElement> localCapacitors;
        for (const CapacitorElement &cp : capacitors)
            if (uf.find(cp.a) == groupRoot)
                localCapacitors.append(cp);

        QVector<SourceElement> localSources;
        for (const SourceElement &s : sources)
            if (uf.find(s.pos) == groupRoot)
                localSources.append(s);

        QVector<InductorElement> localInductors;
        for (const InductorElement &ind : inductors)
            if (uf.find(ind.a) == groupRoot)
                localInductors.append(ind);

        QVector<DiodeElement> localDiodes;
        for (const DiodeElement &d : diodes)
            if (uf.find(d.anode) == groupRoot)
                localDiodes.append(d);

        QVector<TransistorElement> localTransistors;
        for (const TransistorElement &t : transistors)
            if (uf.find(t.base) == groupRoot)
                localTransistors.append(t);

        if (localGround.isEmpty()) {
            // 🔴 رفع باگ: قبلاً هر زیرمدار بدون Ground - حتی یک منبع ولتاژ تنها که
            // مستقیم به ورودی گیت‌های منطقی وصل شده (بدون هیچ مقاومت/خازن/دیودی) -
            // باعث یک پیام خطای «بدون Ground» در هر تیک می‌شد. این پیام گمراه‌کننده
            // بود چون مقدار دیجیتال (LogicValue) این گیت‌ها اصلاً به حل آنالوگ
            // وابسته نیست و کاملاً درست کار می‌کرد؛ فقط ولتاژ *مطلق* واقعاً بدون
            // Ground قابل تعیین نیست. اگر این زیرمدار هیچ عنصر آنالوگ واقعی (که
            // واقعاً به ولتاژ حل‌شده نیاز داشته باشد) نداشته باشد - یعنی فقط منبع(ها)
            // مستقیم به مصرف‌کننده‌های دیجیتال (گیت/LED/سوییچ) وصل شده‌اند - بی‌سروصدا
            // از حل آنالوگ صرف‌نظر می‌کنیم (شبیه‌سازی دیجیتال دست‌نخورده ادامه پیدا
            // می‌کند)؛ فقط وقتی واقعاً یک مقاومت/خازن/سلف/دیود/ترانزیستور یا
            // آمپرمتر/DAC در این زیرمدار باشد که بدون مرجع ولتاژ واقعاً قابل حل
            // نیست، خطای واقعی گزارش می‌شود.
            bool hasRealAnalogLoad = !localResistors.isEmpty() || !localCapacitors.isEmpty()
                || !localInductors.isEmpty() || !localDiodes.isEmpty() || !localTransistors.isEmpty();
            if (!hasRealAnalogLoad) {
                for (const SourceElement &s : localSources) {
                    if (s.ammeterOwner) { hasRealAnalogLoad = true; break; }
                }
            }

            if (hasRealAnalogLoad) {
                failureReasons << QObject::tr("یک زیرمدار بدون هیچ Ground‌ای پیدا شد - ولتاژش قابل محاسبه نیست.");
                allGroupsOk = false;
            }
            continue; // گره‌های این گروه hasVoltage=false باقی می‌مانند (بالا پاک شدند)
        }

        QHash<Node *, int> nodeIndex;
        int nextIndex = 0;
        for (Node *n : groupNodes)
            if (!localGround.contains(n))
                nodeIndex.insert(n, nextIndex++);
        const int nodeCount = nextIndex;

        const int sourceCount = localSources.size();
        const int inductorCount = localInductors.size();
        // ترتیب مجهول‌ها: [ولتاژهای گره غیرزمین] [جریان‌های منبع/آمپرمتر/DAC] [جریان‌های سلف]
        // (دیودها و ترانزیستورها مثل خازن مجهول جدید اضافه نمی‌کنن - فقط رسانایی+منبع جریان معادل)
        const int n = nodeCount + sourceCount + inductorCount;
        if (n == 0)
            continue; // فقط یک Ground تنها، بدون هیچ عنصر دیگری - چیزی برای حل کردن نیست

        // --- یک‌بار ماتریس را با حدس‌های فعلی ولتاژ دیودها می‌سازد و حل می‌کند.
        // برای زیرمدار کاملاً خطی (بدون دیود) فقط یک‌بار صدا زده می‌شود؛ برای
        // زیرمدار غیرخطی، در حلقه نیوتن-رافسون پایین‌تر بارها صدا زده می‌شود. ---
        auto buildAndSolveOnce = [&](const QVector<double> &diodeGuesses,
                                      const QVector<double> &transistorVBEGuesses,
                                      const QVector<double> &transistorVBCGuesses) -> QVector<double> {
            QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
            QVector<double> b(n, 0.0);

            auto stampSelf = [&](Node *node, double g) {
                auto idx = nodeIndex.constFind(node);
                if (idx != nodeIndex.constEnd())
                    A[idx.value()][idx.value()] += g;
            };
            auto stampMutual = [&](Node *na, Node *nb, double g) {
                auto ia = nodeIndex.constFind(na);
                auto ib = nodeIndex.constFind(nb);
                if (ia != nodeIndex.constEnd() && ib != nodeIndex.constEnd()) {
                    A[ia.value()][ib.value()] -= g;
                    A[ib.value()][ia.value()] -= g;
                }
            };

            for (const ResistorElement &r : localResistors) {
                const double g = 1.0 / r.resistance;
                stampSelf(r.a, g);
                stampSelf(r.b, g);
                stampMutual(r.a, r.b, g);
            }

            // خازن (Backward Euler): مثل یک مقاومت Gc=C/Δt، به‌علاوه یک منبع جریان
            // تاریخچه Ihist=Gc*Vprev که از b به a تزریق می‌شود (تایید شده با شارژ RC).
            for (const CapacitorElement &cp : localCapacitors) {
                const double gc = cp.capacitance / kAnalogTimeStepSeconds;
                stampSelf(cp.a, gc);
                stampSelf(cp.b, gc);
                stampMutual(cp.a, cp.b, gc);

                const double iHist = gc * cp.owner->previousVoltage();
                auto ia = nodeIndex.constFind(cp.a);
                auto ib = nodeIndex.constFind(cp.b);
                if (ia != nodeIndex.constEnd()) b[ia.value()] += iHist;
                if (ib != nodeIndex.constEnd()) b[ib.value()] -= iHist;
            }

            for (int i = 0; i < localSources.size(); ++i) {
                const SourceElement &s = localSources[i];
                const int k = nodeCount + i;
                auto ip = nodeIndex.constFind(s.pos);
                auto in_ = nodeIndex.constFind(s.neg);
                if (ip != nodeIndex.constEnd()) { A[ip.value()][k] += 1.0; A[k][ip.value()] += 1.0; }
                if (in_ != nodeIndex.constEnd()) { A[in_.value()][k] -= 1.0; A[k][in_.value()] -= 1.0; }
                b[k] = s.voltage;
            }

            // سلف (Backward Euler): مثل یک منبع ولتاژ (مجهول جریان کمکی)، به‌علاوه
            // یک جمله خودی -L/Δt روی قطر ماتریس و یک جمله تاریخچه در سمت راست معادله.
            for (int i = 0; i < localInductors.size(); ++i) {
                const InductorElement &ind = localInductors[i];
                const int k = nodeCount + sourceCount + i;
                const double rEquivalent = ind.inductance / kAnalogTimeStepSeconds;

                auto ia = nodeIndex.constFind(ind.a);
                auto ib = nodeIndex.constFind(ind.b);
                if (ia != nodeIndex.constEnd()) { A[ia.value()][k] += 1.0; A[k][ia.value()] += 1.0; }
                if (ib != nodeIndex.constEnd()) { A[ib.value()][k] -= 1.0; A[k][ib.value()] -= 1.0; }
                A[k][k] -= rEquivalent;
                b[k] = -rEquivalent * ind.owner->previousCurrent();
            }

            // دیود (نیوتن-رافسون): رسانایی دیفرانسیلی G0 در ولتاژ حدس فعلی، به‌علاوه
            // منبع جریان معادل نورتن iEq=G0*Vguess-I(Vguess) - دقیقاً همان الگوی
            // خازن (b[anode]+=iEq ، b[cathode]-=iEq)، فقط iEq این‌بار از معادله
            // شاکلی می‌آید نه از تاریخچه زمانی.
            for (int i = 0; i < localDiodes.size(); ++i) {
                const DiodeElement &d = localDiodes[i];
                double current = 0.0, conductance = 0.0;
                diodeCurrentAndConductance(diodeGuesses[i], d.saturationCurrent, d.idealityFactor,
                                           &current, &conductance);
                stampSelf(d.anode, conductance);
                stampSelf(d.cathode, conductance);
                stampMutual(d.anode, d.cathode, conductance);

                const double iEq = conductance * diodeGuesses[i] - current;
                auto ia = nodeIndex.constFind(d.anode);
                auto ik = nodeIndex.constFind(d.cathode);
                if (ia != nodeIndex.constEnd()) b[ia.value()] += iEq;
                if (ik != nodeIndex.constEnd()) b[ik.value()] -= iEq;
            }

            // ترانزیستور NPN (مدل Ebers-Moll، نیوتن-رافسون - فاز ۱۶). با دو
            // «دیود» پایه‌محور (D_E: بیس->امیتر با جریان I_DE، D_C: بیس->کلکتور
            // با جریان I_DC، هر دو دقیقاً با همان استمپ رسانایی+iEq دیود بالا)
            // به‌علاوه دو منبع جریان وابسته مدل می‌شود: S_F به مقدار alphaF*I_DE
            // از کلکتور به بیس، و S_R به مقدار alphaR*I_DC از امیتر به بیس. این
            // مدل با معادلات استاندارد Ebers-Moll (I_B=(1-αF)I_DE+(1-αR)I_DC,
            // I_C=αF*I_DE-I_DC, I_E=-I_DE+αR*I_DC) و پایستگی جریان در هر سه پایه
            // دستی تأیید شده (جمع هر سطر/ستون ژاکوبین صفر می‌شود - به HANDOFF
            // مراجعه کن). چون S_F/S_R به‌صورت خطی به iEq همان دیودهای D_E/D_C
            // وابسته‌اند (iEq_SF=alphaF*iEq_DE، iEq_SR=alphaR*iEq_DC)، نیازی به
            // محاسبه جداگانه نیست.
            for (int i = 0; i < localTransistors.size(); ++i) {
                const TransistorElement &t = localTransistors[i];
                double currentDE = 0.0, conductanceDE = 0.0;
                double currentDC = 0.0, conductanceDC = 0.0;
                diodeCurrentAndConductance(transistorVBEGuesses[i], t.saturationCurrent,
                                            t.idealityFactor, &currentDE, &conductanceDE);
                diodeCurrentAndConductance(transistorVBCGuesses[i], t.saturationCurrent,
                                            t.idealityFactor, &currentDC, &conductanceDC);

                const double iEqDE = conductanceDE * transistorVBEGuesses[i] - currentDE;
                const double iEqDC = conductanceDC * transistorVBCGuesses[i] - currentDC;

                // D_E: بیس(آند)->امیتر(کاتد)
                stampSelf(t.base, conductanceDE);
                stampSelf(t.emitter, conductanceDE);
                stampMutual(t.base, t.emitter, conductanceDE);
                {
                    auto ib = nodeIndex.constFind(t.base);
                    auto ie = nodeIndex.constFind(t.emitter);
                    if (ib != nodeIndex.constEnd()) b[ib.value()] += iEqDE;
                    if (ie != nodeIndex.constEnd()) b[ie.value()] -= iEqDE;
                }
                // D_C: بیس(آند)->کلکتور(کاتد)
                stampSelf(t.base, conductanceDC);
                stampSelf(t.collector, conductanceDC);
                stampMutual(t.base, t.collector, conductanceDC);
                {
                    auto ib = nodeIndex.constFind(t.base);
                    auto ic = nodeIndex.constFind(t.collector);
                    if (ib != nodeIndex.constEnd()) b[ib.value()] += iEqDC;
                    if (ic != nodeIndex.constEnd()) b[ic.value()] -= iEqDC;
                }

                // S_F: alphaF*I_DE از کلکتور به بیس (کنترل‌شده با V_BE، یعنی
                // وابسته به گره‌های بیس/امیتر نه خودِ کلکتور).
                const double gTF = t.alphaForward * conductanceDE;
                const double iEqSF = t.alphaForward * iEqDE;
                {
                    auto ic = nodeIndex.constFind(t.collector);
                    auto ib = nodeIndex.constFind(t.base);
                    auto ie = nodeIndex.constFind(t.emitter);
                    if (ic != nodeIndex.constEnd() && ib != nodeIndex.constEnd()) A[ic.value()][ib.value()] += gTF;
                    if (ic != nodeIndex.constEnd() && ie != nodeIndex.constEnd()) A[ic.value()][ie.value()] -= gTF;
                    if (ib != nodeIndex.constEnd()) A[ib.value()][ib.value()] -= gTF;
                    if (ib != nodeIndex.constEnd() && ie != nodeIndex.constEnd()) A[ib.value()][ie.value()] += gTF;
                    if (ic != nodeIndex.constEnd()) b[ic.value()] += iEqSF;
                    if (ib != nodeIndex.constEnd()) b[ib.value()] -= iEqSF;
                }

                // S_R: alphaR*I_DC از امیتر به بیس (کنترل‌شده با V_BC).
                const double gTR = t.alphaReverse * conductanceDC;
                const double iEqSR = t.alphaReverse * iEqDC;
                {
                    auto ie = nodeIndex.constFind(t.emitter);
                    auto ib = nodeIndex.constFind(t.base);
                    auto icc = nodeIndex.constFind(t.collector);
                    if (ie != nodeIndex.constEnd() && ib != nodeIndex.constEnd()) A[ie.value()][ib.value()] += gTR;
                    if (ie != nodeIndex.constEnd() && icc != nodeIndex.constEnd()) A[ie.value()][icc.value()] -= gTR;
                    if (ib != nodeIndex.constEnd()) A[ib.value()][ib.value()] -= gTR;
                    if (ib != nodeIndex.constEnd() && icc != nodeIndex.constEnd()) A[ib.value()][icc.value()] += gTR;
                    if (ie != nodeIndex.constEnd()) b[ie.value()] += iEqSR;
                    if (ib != nodeIndex.constEnd()) b[ib.value()] -= iEqSR;
                }
            }

            QVector<double> x;
            if (!solveLinearSystem(A, b, &x))
                return QVector<double>();
            return x;
        };

        QVector<double> x;

        if (localDiodes.isEmpty() && localTransistors.isEmpty()) {
            // مسیر کاملاً خطی - همان مسیر تست‌شده فازهای ۱۱ تا ۱۳، بدون هیچ تغییری.
            x = buildAndSolveOnce(QVector<double>(), QVector<double>(), QVector<double>());
            if (x.isEmpty()) {
                failureReasons << QObject::tr("یک زیرمدار به دلیل معادلات ناسازگار/تکین قابل حل نبود.");
                allGroupsOk = false;
                continue;
            }
        } else {
            // مسیر غیرخطی: تکرار نیوتن-رافسون تا همگرایی ولتاژ دیودها *و*
            // پیوندهای B-E/B-C همه‌ی ترانزیستورها - در یک حلقه‌ی مشترک، چون
            // ممکنه دیود و ترانزیستور در یک زیرمدار به هم وابسته باشن.
            QVector<double> guesses(localDiodes.size());
            for (int i = 0; i < localDiodes.size(); ++i)
                guesses[i] = qBound(kDiodeVoltageFloor, localDiodes[i].owner->previousVoltageGuess(),
                                     kDiodeVoltageClamp);

            QVector<double> vbeGuesses(localTransistors.size());
            QVector<double> vbcGuesses(localTransistors.size());
            for (int i = 0; i < localTransistors.size(); ++i) {
                vbeGuesses[i] = qBound(kDiodeVoltageFloor, localTransistors[i].owner->previousVBEGuess(),
                                        kDiodeVoltageClamp);
                vbcGuesses[i] = qBound(kDiodeVoltageFloor, localTransistors[i].owner->previousVBCGuess(),
                                        kDiodeVoltageClamp);
            }

            bool converged = false;
            bool linearFailure = false;
            for (int iter = 0; iter < kMaxNewtonIterations; ++iter) {
                x = buildAndSolveOnce(guesses, vbeGuesses, vbcGuesses);
                if (x.isEmpty()) { linearFailure = true; break; }

                double maxDelta = 0.0;
                QVector<double> newGuesses(localDiodes.size());
                for (int i = 0; i < localDiodes.size(); ++i) {
                    const DiodeElement &d = localDiodes[i];
                    const double va = localGround.contains(d.anode) ? 0.0 : x[nodeIndex.value(d.anode)];
                    const double vk = localGround.contains(d.cathode) ? 0.0 : x[nodeIndex.value(d.cathode)];
                    const double stepped = limitNewtonVoltageStep(va - vk, guesses[i], d.idealityFactor);
                    double newVd = qBound(kDiodeVoltageFloor, stepped, kDiodeVoltageClamp);
                    maxDelta = qMax(maxDelta, std::fabs(newVd - guesses[i]));
                    newGuesses[i] = newVd;
                }
                guesses = newGuesses;

                QVector<double> newVbeGuesses(localTransistors.size());
                QVector<double> newVbcGuesses(localTransistors.size());
                for (int i = 0; i < localTransistors.size(); ++i) {
                    const TransistorElement &t = localTransistors[i];
                    const double vb = localGround.contains(t.base) ? 0.0 : x[nodeIndex.value(t.base)];
                    const double vc = localGround.contains(t.collector) ? 0.0 : x[nodeIndex.value(t.collector)];
                    const double ve = localGround.contains(t.emitter) ? 0.0 : x[nodeIndex.value(t.emitter)];
                    const double steppedVbe = limitNewtonVoltageStep(vb - ve, vbeGuesses[i], t.idealityFactor);
                    const double steppedVbc = limitNewtonVoltageStep(vb - vc, vbcGuesses[i], t.idealityFactor);
                    const double vbe = qBound(kDiodeVoltageFloor, steppedVbe, kDiodeVoltageClamp);
                    const double vbc = qBound(kDiodeVoltageFloor, steppedVbc, kDiodeVoltageClamp);
                    maxDelta = qMax(maxDelta, std::fabs(vbe - vbeGuesses[i]));
                    maxDelta = qMax(maxDelta, std::fabs(vbc - vbcGuesses[i]));
                    newVbeGuesses[i] = vbe;
                    newVbcGuesses[i] = vbc;
                }
                vbeGuesses = newVbeGuesses;
                vbcGuesses = newVbcGuesses;

                if (maxDelta < kNewtonToleranceVolts) { converged = true; break; }
            }

            if (linearFailure || !converged) {
                failureReasons << QObject::tr("یک زیرمدار غیرخطی (دیود/ترانزیستور) همگرا نشد یا قابل حل نبود.");
                allGroupsOk = false;
                continue;
            }

            for (int i = 0; i < localDiodes.size(); ++i)
                localDiodes[i].owner->setPreviousVoltageGuess(guesses[i]);
            for (int i = 0; i < localTransistors.size(); ++i) {
                localTransistors[i].owner->setPreviousVBEGuess(vbeGuesses[i]);
                localTransistors[i].owner->setPreviousVBCGuess(vbcGuesses[i]);
            }
        }

        for (Node *n2 : groupNodes) {
            if (localGround.contains(n2))
                n2->setVoltage(0.0);
            else
                n2->setVoltage(x[nodeIndex.value(n2)]);
        }
        for (int i = 0; i < localSources.size(); ++i) {
            if (AnalogAmmeter *amm = localSources[i].ammeterOwner)
                amm->setMeasuredCurrent(x[nodeCount + i]);
        }
        for (int i = 0; i < localInductors.size(); ++i) {
            localInductors[i].owner->setPreviousCurrent(x[nodeCount + sourceCount + i]);
        }
        // ولتاژ جدید خازن‌ها را برای گام بعدی ذخیره کن (بعد از حل شدن گره‌ها)
        for (const CapacitorElement &cp : localCapacitors) {
            const double va = localGround.contains(cp.a) ? 0.0 : x[nodeIndex.value(cp.a)];
            const double vb = localGround.contains(cp.b) ? 0.0 : x[nodeIndex.value(cp.b)];
            cp.owner->setPreviousVoltage(va - vb);
        }
    }

    if (!failureReasons.isEmpty() && errorMessage)
        *errorMessage = failureReasons.join(QLatin1Char(' '));

    return allGroupsOk;
}

#include "circuitscene.h"
#include "junction.h"
#include "componentfactory.h"
#include "component.h"
#include "wire.h"
#include "pin.h"
#include "dynamicpropertiesdialog.h"
#include <QGraphicsSceneMouseEvent>
#include <QTransform>
#include "commands/addcomponentcommand.h"
#include "commands/movecommand.h"
#include "commands/addwirecommand.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QSet>
#include "net.h"
#include <QKeyEvent>
#include <QFile>
#include <QKeyEvent>
#include <QLineF>
#include <limits>

CircuitScene::CircuitScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

//////////////////////////////////////////////////////////
// Background Grid
//////////////////////////////////////////////////////////

void CircuitScene::drawBackground(QPainter *painter,
                                  const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    // ---- ۱. صفحه شطرنجی/گرید در کل ناحیه قابل مشاهده (بخش ۱ درخواست کاربر) ----
    // این گرید در همه‌جای صحنه رسم می‌شود، نه فقط داخل کادر شماتیک - یعنی حتی
    // ناحیه‌ی اضافه‌شده برای اسکرول/جابجایی بیرون از کادر آبی هم صفحه‌ی شطرنجی دارد.
    const int gridSize = 20;

    painter->setPen(QColor(230,230,230));

    int left = int(rect.left()) - (int(rect.left()) % gridSize);
    int top  = int(rect.top())  - (int(rect.top())  % gridSize);

    for(int x = left; x < rect.right(); x += gridSize)
    {
        painter->drawLine(x,
                          rect.top(),
                          x,
                          rect.bottom());
    }

    for(int y = top; y < rect.bottom(); y += gridSize)
    {
        painter->drawLine(rect.left(),
                          y,
                          rect.right(),
                          y);
    }

    // ---- ۲. کادر آبی محدوده شماتیک (بخش ۱ درخواست کاربر) ----
    // اندازه‌اش دقیقاً همان مقداری است که کاربر در StartupDialog انتخاب کرده
    // (setSchematicRect از MainWindow صدا زده می‌شود). قطعاتی که خارج از این کادر
    // قرار بگیرند در شبیه‌سازی شرکت داده نمی‌شوند (SimulationEngine/AnalogSolver -
    // به componentsInSchematic() نگاه کنید)، پس دیدن مرز آن روی بوم برای کاربر مهم است.
    if (m_schematicRect.isValid()) {
        // ناحیه بیرون از کادر را کمی سایه‌دار کن تا محدوده قابل‌شبیه‌سازی از حاشیه
        // اضافه‌شده برای جابجایی/اسکرول به‌وضوح متمایز باشد (خط‌های گرید هنوز از
        // زیر این سایه‌ی نیمه‌شفاف دیده می‌شوند).
        QPainterPath outsidePath;
        outsidePath.addRect(rect);
        QPainterPath insidePath;
        insidePath.addRect(m_schematicRect);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 0, 18));
        painter->drawPath(outsidePath.subtracted(insidePath));

        // مرز آبی خودِ کادر - با قلم Cosmetic تا ضخامت آن در هر سطح زومی ثابت بماند.
        QPen borderPen(QColor(30, 90, 220));
        borderPen.setWidth(2);
        borderPen.setCosmetic(true);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_schematicRect);
    }
}

// --- تنظیم/تغییر محدوده شماتیک (کادر آبی) - بخش ۱ درخواست کاربر ---
void CircuitScene::setSchematicRect(const QRectF &rect)
{
    m_schematicRect = rect;

    // sceneRect واقعی صحنه را کمی بزرگ‌تر از کادر شماتیک تنظیم می‌کنیم تا صفحه
    // شطرنجی پس‌زمینه در اطراف کادر هم دیده/اسکرول شود (حداقل ۵۰۰ پیکسل یا ۴۰٪
    // اندازه هر بعد، هرکدام بزرگ‌تر بود).
    const qreal padX = qMax<qreal>(500.0, rect.width() * 0.4);
    const qreal padY = qMax<qreal>(500.0, rect.height() * 0.4);
    setSceneRect(rect.adjusted(-padX, -padY, padX, padY));

    update();
}

bool CircuitScene::isWithinSchematicBounds(const Component *component) const
{
    if (!component || !m_schematicRect.isValid())
        return false;

    return m_schematicRect.contains(component->sceneBoundingRect().center());
}

QList<Component*> CircuitScene::componentsInSchematic() const
{
    QList<Component*> result;
    const QList<Component*> all = components();
    for (Component *c : all) {
        if (isWithinSchematicBounds(c))
            result.append(c);
    }
    return result;
}

//////////////////////////////////////////////////////////
// Mouse Move
//////////////////////////////////////////////////////////

void CircuitScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // ۱. محاسبه موقعیت ماوس روی گرید (مثلاً گریدهای ۱۰ پیکسلی)
    int gridSize = 10;
    qreal snapX = qRound(event->scenePos().x() / gridSize) * gridSize;
    qreal snapY = qRound(event->scenePos().y() / gridSize) * gridSize;
    QPointF currentGridPos(snapX, snapY);

    // ۲. 🟢 تکنیک بهینه‌سازی طلایی: اگر ماوس هنوز در همان خانه گرید است، هیچ محاسباتی نکن و خارج شو!
    if (currentGridPos == m_lastGridPos) {
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }
    m_lastGridPos = currentGridPos; // به‌روزرسانی آخرین موقعیت گرید

    // ۳. کدهای حرکت دادن پیش‌نمایش قطعه یا پیش‌نمایش سیم را اینجا بنویسید
    if (m_toolManager && m_toolManager->currentTool() == Tool::PlaceComponent && m_previewItem) {
        m_previewItem->setPos(currentGridPos);
    }

    if (m_drawingWire && m_startPin) {
        // کدهای مربوط به خط پیش‌نمایش سیم (فقط زمانی که ماوس یک خانه واقعی جابجا شده باشد)
    }

    QGraphicsScene::mouseMoveEvent(event);
}
//////////////////////////////////////////////////////////
// Mouse Press
//////////////////////////////////////////////////////////

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_toolManager) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    switch (m_toolManager->currentTool())
    {
    case Tool::Select:
        handleSelectToolPress(event);
        break;

    case Tool::PlaceComponent:
        handlePlaceComponentToolPress(event);
        break;

    case Tool::Wire:
        handleWireToolPress(event);
        break;

    default:
        QGraphicsScene::mousePressEvent(event);
        break;
    }
}
//////////////////////////////////////////////////////////
// Find Pin
//////////////////////////////////////////////////////////

Pin *CircuitScene::pinAt(const QPointF &scenePos) const
{
    QList<QGraphicsItem*> list = items(scenePos);

    qDebug() << "Items count:" << list.count();
    qDebug() << "Items size=" << list.size();

    for(QGraphicsItem *item : list)
    {
        qDebug() << item;

        if(Pin *pin = dynamic_cast<Pin*>(item))
            return pin;
    }

    return nullptr;
}
//////////////////////////////////////////////////////////
// Tool Manager
//////////////////////////////////////////////////////////

void CircuitScene::setToolManager(ToolManager *manager)
{
    m_toolManager = manager;
}

//////////////////////////////////////////////////////////
// Component
//////////////////////////////////////////////////////////

void CircuitScene::setComponentToPlace(const QString &name)
{
    m_componentToPlace = name;
}

//////////////////////////////////////////////////////////
// Ghost Component
//////////////////////////////////////////////////////////

void CircuitScene::beginComponentPlacement(const QString &name)
{
    if(m_previewItem)
    {
        removeItem(m_previewItem);

        delete m_previewItem;

        m_previewItem = nullptr;
    }

    m_previewComponent = name;

    m_previewItem = ComponentFactory::create(name);

    if(m_previewItem)
    {
        addItem(m_previewItem);

        m_previewItem->setOpacity(0.4);

        m_previewItem->setZValue(1000);
    }
}

void CircuitScene::cancelComponentPlacement()
{
    if(m_previewItem)
    {
        removeItem(m_previewItem);

        delete m_previewItem;

        m_previewItem = nullptr;
    }

    m_previewComponent.clear();
}
void CircuitScene::cancelPlacement()
{
    if(m_previewItem)
    {
        removeItem(m_previewItem);

        delete m_previewItem;

        m_previewItem = nullptr;
    }

    m_previewComponent.clear();
}

void CircuitScene::setUndoStack(QUndoStack *stack)
{
    m_undoStack = stack;
}

void CircuitScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QList<QGraphicsItem*> items = selectedItems();

    if(!items.isEmpty() && m_toolManager->currentTool() == Tool::Select)
    {
        QMap<QGraphicsItem*, QPointF> endPos;

        for(QGraphicsItem *item : items)
            endPos[item] = item->pos();

        bool changed = false;

        for(auto it = m_startPositions.begin(); it != m_startPositions.end(); ++it)
        {
            if(it.value() != it.key()->pos())
            {
                changed = true;
                break;
            }
        }

        if(changed && m_undoStack)
        {
            MoveCommand *cmd =
                new MoveCommand(this, items);

            cmd->setNewPositions(endPos);

            m_undoStack->push(cmd);
        }
        m_startPositions.clear();
    }

    QGraphicsScene::mouseReleaseEvent(event);
}
// نزدیک‌ترین سیم به یک نقطه دلخواه (برای اسنپ خودکار پین هنگام قرار دادن قطعه - بخش ۲ درخواست کاربر)
Wire* CircuitScene::findNearbyWire(const QPointF &scenePos, qreal maxDistance, Pin **outNearestPin) const
{
    if (outNearestPin) *outNearestPin = nullptr;

    Wire *best = nullptr;
    qreal bestDist = maxDistance;

    for (Wire *w : m_wires) {
        if (!w) continue;

        const QVector<QPointF> pts = w->points();
        for (int i = 0; i + 1 < pts.size(); ++i) {
            const QPointF a = pts.at(i);
            const QPointF b = pts.at(i + 1);

            // فاصله نقطه scenePos تا پاره‌خط ab (تصویر روی پاره‌خط، محدود به دو سر آن)
            const QPointF ab = b - a;
            const qreal lenSq = QPointF::dotProduct(ab, ab);
            qreal t = 0.0;
            if (lenSq > 1e-9)
                t = qBound(0.0, QPointF::dotProduct(scenePos - a, ab) / lenSq, 1.0);
            const QPointF closest = a + t * ab;
            const qreal dist = QLineF(scenePos, closest).length();

            if (dist < bestDist) {
                bestDist = dist;
                best = w;
            }
        }
    }

    if (best && outNearestPin) {
        Pin *a = best->startPin();
        Pin *b = best->endPin();
        const qreal distA = a ? QLineF(scenePos, a->scenePos()).length() : std::numeric_limits<qreal>::max();
        const qreal distB = b ? QLineF(scenePos, b->scenePos()).length() : std::numeric_limits<qreal>::max();
        *outNearestPin = (distA <= distB) ? a : b;
    }

    return best;
}

Node *CircuitScene::createOrGetNode(Pin *p1, Pin *p2)
{
    Node *n1 = p1 ? p1->node() : nullptr;
    Node *n2 = p2 ? p2->node() : nullptr;

    //-----------------------------------------
    // هیچ Node ای وجود ندارد
    //-----------------------------------------

    if(!n1 && !n2)
    {
        Node *node = new Node(this);

        Net *net = new Net(this);

        net->addPin(p1);
        net->addPin(p2);

        // 🔴 رفع باگ اصلی: node->addPin() هم باید صدا زده شود، نه فقط net->addPin().
        // Node::resolvedValue() (که LED/Wire/DigitalComponent برای رنگ زنده و انتشار
        // مقدار دیجیتال به آن وابسته‌اند) از روی Node::m_pins می‌خواند، نه از Net.
        // بدون این دو خط، m_pins همیشه خالی می‌ماند و resolvedValue() همیشه Undefined
        // برمی‌گرداند - یعنی هیچ LED یا سیمی هرگز رنگ زنده نمی‌گرفت.
        node->addPin(p1);
        node->addPin(p2);

        node->setNet(net);

        m_nets.append(net);

        Junction *junction = new Junction(node);

        addItem(junction);

        node->setJunction(junction);

        m_nodes.append(node);

        return node;
    }

    //-----------------------------------------
    // فقط اولی Node دارد
    //-----------------------------------------

    if(n1 && !n2)
    {
        n1->net()->addPin(p2);
        n1->addPin(p2); // 🔴 همان رفع باگ بالا
        return n1;
    }


    //-----------------------------------------
    // فقط دومی Node دارد
    //-----------------------------------------

    if(!n1 && n2)
    {
        n2->net()->addPin(p1);
        n2->addPin(p1); // 🔴 همان رفع باگ بالا
        return n2;
    }

    //-----------------------------------------
    // هر دو Node دارند
    //-----------------------------------------

    if(n1 != n2)
        mergeNodes(n1,n2);

    return n1;
}
void CircuitScene::mergeNodes(Node *a, Node *b)
{
    if(!a || !b || a == b)
        return;

    Net *netA = a->net();
    Net *netB = b->net();

    //--------------------------------------------------
    // انتقال Pin ها
    //--------------------------------------------------

    QList<Pin*> pins = b->pins();

    for(Pin *pin : pins)
    {
        pin->setNode(a);

        a->addPin(pin); // 🔴 همان رفع باگ - قبلاً فقط به Net اضافه می‌شد نه به Node مقصد

        if(netA)
            netA->addPin(pin);
    }

    //--------------------------------------------------
    // انتقال Wire ها از Net
    //--------------------------------------------------

    if(netA && netB && netA != netB)
    {
        QList<Wire*> wires = netB->wires();

        for(Wire *wire : wires)
        {
            netA->addWire(wire);
            netB->removeWire(wire);
        }
    }

    //--------------------------------------------------
    // حذف Junction
    //--------------------------------------------------

    if(b->junction())
    {
        removeItem(b->junction());
        delete b->junction();
    }

    //--------------------------------------------------
    // حذف Net قدیمی
    //--------------------------------------------------

    if(netA && netB && netA != netB)
    {
        m_nets.removeAll(netB);
        delete netB;
    }

    //--------------------------------------------------
    // حذف Node
    //--------------------------------------------------

    m_nodes.removeAll(b);

    delete b;
}


void CircuitScene::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        cancelPlacement();

        if(m_toolManager)
            m_toolManager->setCurrentTool(Tool::Select);

        return;
    }

    QGraphicsScene::keyPressEvent(event);
}


void CircuitScene::cancelWireDrawing()
{
    m_startPin = nullptr;
    m_drawingWire = false;
    m_currentWire = nullptr;
}


void CircuitScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    // ۱. پیدا کردن شیئی که روی آن دبل‌کلیک شده است
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());

    // ۲. بررسی اینکه آیا این شیء یک قطعه (Component) است یا خیر
    Component *comp = dynamic_cast<Component*>(item);

    if (comp) {
        // ۳. گرفتن ویژگی‌های فعلی قطعه
        auto props = comp->allProperties();

        // ۴. باز کردن پنجره هوشمندی که ساختیم
        DynamicPropertiesDialog dialog(props);
        if (dialog.exec() == QDialog::Accepted) {
            // ۵. دریافت مقادیر جدید از پنجره در صورت فشرده شدن دکمه OK
            auto newValues = dialog.getNewValues();

            // ۶. اعمال مقادیر جدید به قطعه
            for (auto it = newValues.begin(); it != newValues.end(); ++it) {
                comp->setProperty(it.key(), it.value());
            }
        }
    } else {
        // اگر روی قطعه کلیک نشده بود، رفتار پیش‌فرض سیستمی اجرا شود
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

// --- دسترسی به همه قطعات مدار (برای ProjectSerializer / DesignRuleChecker) ---
QList<Component*> CircuitScene::components() const
{
    QList<Component*> result;
    const QList<QGraphicsItem*> allItems = items();
    for (QGraphicsItem *item : allItems) {
        if (Component *c = dynamic_cast<Component*>(item))
            result.append(c);
    }
    return result;
}

// --- ساخت سیم بین دو پایه (منطق مشترک ابزار سیم‌کشی و بارگذاری پروژه) ---
Wire* CircuitScene::connectPins(Pin *a, Pin *b)
{
    if (!a || !b || a == b)
        return nullptr;

    Wire *wire = new Wire(a, b);
    addItem(wire);

    reattachWire(wire);

    return wire;
}

// --- اتصال دوباره یک سیم موجود به توپولوژی مدار (برای ساخت سیم نو و برای Undo حذف سیم) ---
void CircuitScene::reattachWire(Wire *wire)
{
    if (!wire) return;

    Pin *a = wire->startPin();
    Pin *b = wire->endPin();
    if (!a || !b) return;

    if (!wire->scene())
        addItem(wire);

    a->addWire(wire);
    b->addWire(wire);

    Node *node = createOrGetNode(a, b);
    a->setNode(node);
    b->setNode(node);
    node->addWire(wire);
    if (node->net())
        node->net()->addWire(wire);

    if (node->junction())
        node->junction()->updatePosition();

    if (!m_wires.contains(wire))
        m_wires.append(wire);
}

// --- جدا کردن یک سیم از توپولوژی مدار (بدون delete کردن شیء - برای پشتیبانی از Undo) ---
void CircuitScene::disconnectWire(Wire *wire)
{
    if (!wire) return;

    Pin *a = wire->startPin();
    Pin *b = wire->endPin();

    Node *node = nullptr;
    if (a && a->node()) node = a->node();
    else if (b && b->node()) node = b->node();

    if (a) a->removeWire(wire);
    if (b) b->removeWire(wire);

    if (node) {
        node->removeWire(wire);
        if (node->net())
            node->net()->removeWire(wire);
    }

    m_wires.removeOne(wire);

    if (wire->scene())
        removeItem(wire);

    if (node)
        splitNodeIfNeeded(node);
}

// --- بعد از حذف یک سیم، بررسی می‌کند پین‌های باقیمانده گره هنوز (از طریق سیم‌های دیگر)
//     به هم متصل هستند یا باید به چند گره مستقل تقسیم شوند (دقیقاً برعکس mergeNodes) ---
void CircuitScene::splitNodeIfNeeded(Node *node)
{
    if (!node) return;

    const QList<Pin*> allPins = node->pins();
    if (allPins.size() <= 1)
        return; // چیزی برای شکستن نیست

    // BFS روی پین‌های این گره با استفاده از سیم‌های باقیمانده، برای پیدا کردن زیرگروه‌های واقعاً متصل
    QList<QList<Pin*>> groups;
    QSet<Pin*> visited;

    for (Pin *start : allPins) {
        if (visited.contains(start)) continue;

        QList<Pin*> group;
        QList<Pin*> queue;
        queue.append(start);
        visited.insert(start);

        while (!queue.isEmpty()) {
            Pin *current = queue.takeFirst();
            group.append(current);

            for (Wire *w : current->wires()) {
                Pin *other = (w->startPin() == current) ? w->endPin() : w->startPin();
                if (other && allPins.contains(other) && !visited.contains(other)) {
                    visited.insert(other);
                    queue.append(other);
                }
            }
        }

        groups.append(group);
    }

    if (groups.size() <= 1)
        return; // همه پین‌ها هنوز از راه دیگری به هم وصل‌اند - نیازی به شکستن نیست

    bool originalNodeReused = false;

    for (const QList<Pin*> &group : groups) {
        if (group.size() == 1) {
            // این پین دیگر به هیچ‌چیز وصل نیست - رها (Floating) شد
            Pin *p = group.first();
            node->removePin(p);
            p->setNode(nullptr);
            continue;
        }

        if (!originalNodeReused) {
            // گروه اول با ۲+ پین: همان گره قبلی را نگه می‌داریم (پین‌هایش از قبل درستند)
            originalNodeReused = true;
            continue;
        }

        // برای هر زیرگروه اضافی، یک گره/شبکه/Junction مستقل جدید بساز (مثل حالت "گره‌ای وجود ندارد" در createOrGetNode)
        Node *newNode = new Node(this);
        Net *newNet = new Net(this);
        newNode->setNet(newNet);

        Junction *junction = new Junction(newNode);
        addItem(junction);
        newNode->setJunction(junction);

        for (Pin *p : group) {
            node->removePin(p);
            newNode->addPin(p);
            newNet->addPin(p);
            p->setNode(newNode);

            for (Wire *w : p->wires()) {
                if (node->net()) node->net()->removeWire(w);
                newNode->addWire(w);
                newNet->addWire(w);
            }
        }

        m_nodes.append(newNode);
    }

    // اگر گره اصلی دیگر هیچ پینی نداشت (یعنی هیچ زیرگروهی ۲+ پین نبود که آن را نگه دارد)، آزادش کن
    if (node->pins().isEmpty()) {
        if (node->junction()) {
            removeItem(node->junction());
            delete node->junction();
        }
        if (node->net()) {
            m_nets.removeAll(node->net());
            delete node->net();
        }
        m_nodes.removeAll(node);
        delete node;
    }
}

// --- پاک‌سازی کامل مدار: برای «پروژه جدید» و قبل از بارگذاری یک پروژه دیگر ---
void CircuitScene::resetCircuit()
{
    // ۱. آزادسازی اشیاء غیرگرافیکی (Node/Net مالک آیتم گرافیکی نیستند، پس clear() آن‌ها را پاک نمی‌کند)
    qDeleteAll(m_nodes);
    m_nodes.clear();

    qDeleteAll(m_nets);
    m_nets.clear();

    m_wires.clear();

    // ۲. آزادسازی همه آیتم‌های گرافیکی (قطعات، سیم‌ها، Junctionها)
    clear();

    // ۳. بازنشانی وضعیت ابزارهای در حال کار
    m_previewItem = nullptr;
    m_currentWire = nullptr;
    m_startPin = nullptr;
    m_drawingWire = false;
    m_startPositions.clear();
}



// --- ۱. مدیریت کلیک در حالت انتخاب قطعات ---
void CircuitScene::handleSelectToolPress(QGraphicsSceneMouseEvent *event)
{
    m_startPositions.clear();
    QList<QGraphicsItem*> list = selectedItems();
    for(QGraphicsItem *item : list) {
        m_startPositions[item] = item->pos();
    }

    // در حالت انتخاب، حتماً باید رویداد به کلاس پایه برسد تا آیتم‌ها فوکوس بگیرند و منوها باز شوند
    QGraphicsScene::mousePressEvent(event);
}

// --- ۲. مدیریت کلیک در حالت قرار دادن قطعه جدید ---
void CircuitScene::handlePlaceComponentToolPress(QGraphicsSceneMouseEvent *event)
{
    if (!m_previewItem) {
        event->accept();
        return;
    }

    QGraphicsItem *item = ComponentFactory::create(m_previewComponent);
    if (!item) {
        event->accept();
        return;
    }

    item->setPos(m_previewItem->pos());

    // بررسی سبک برخورد برای جلوگیری از لگ
    bool collision = false;
    QList<QGraphicsItem*> collidingList = collidingItems(item);
    for(QGraphicsItem *other : collidingList) {
        if (other == m_previewItem || other->parentItem() == m_previewItem)
            continue;
        // سیم‌ها، پایه‌ها و نقاط Junction هرگز مانع قرار دادن قطعه نمی‌شوند - بخش ۱ و ۲
        // درخواست کاربر: کلیک نزدیک سیم باید باعث اتصال خودکار شود، نه رد شدن قرارگیری.
        if (dynamic_cast<Wire*>(other) || dynamic_cast<Pin*>(other) || dynamic_cast<Junction*>(other)
            || other->type() == QGraphicsTextItem::Type)
            continue;

        collision = true;
        break;
    }

    if (!collision) {
        if (m_undoStack) m_undoStack->beginMacro(tr("Place Component"));

        if (m_undoStack) {
            m_undoStack->push(new AddComponentCommand(this, item));
        } else {
            addItem(item);
        }

        // --- اسنپ خودکار پین به نزدیک‌ترین سیم (بخش ۳ درخواست کاربر) ---
        // اگر یکی از پایه‌های قطعه تازه‌قرارگرفته به فاصله نزدیک یک سیم موجود باشد،
        // به‌جای رها کردن قطعه بدون اتصال، به‌صورت خودکار یک سیم جدید از آن پایه به
        // نزدیک‌ترین سر آن سیم (شروع یا پایان) رسم می‌شود تا در همان گره/شبکه قرار گیرد.
        // هر پایه‌ی قطعه مستقل بررسی می‌شود (نه فقط اولین‌مورد پیداشده) چون کاربر
        // صریحاً خواسته «پین‌ها» (جمع) روی خطوط بنشینند - دقیقاً همان چیزی که برای
        // جاگذاری یک قطعه دوسر (مثل مقاومت) بین دو سیم موجود لازم است.
        if (Component *newComp = dynamic_cast<Component*>(item)) {
            constexpr qreal kWireSnapDistance = 15.0;
            for (Pin *p : newComp->pins()) {
                if (!p) continue;

                Pin *targetPin = nullptr;
                Wire *nearWire = findNearbyWire(p->scenePos(), kWireSnapDistance, &targetPin);
                if (nearWire && targetPin && targetPin != p) {
                    if (Wire *newWire = connectPins(p, targetPin)) {
                        if (m_undoStack)
                            m_undoStack->push(new AddWireCommand(this, newWire));
                    }
                }
            }
        }

        if (m_undoStack) m_undoStack->endMacro();
    } else {
        delete item; // اگر برخورد داشت، قطعه حذف می‌شود
    }

    // رویداد کاملاً مصرف شد؛ تداخلی با ابزارهای دیگر ایجاد نمی‌شود
    event->accept();
}

// --- ۳. مدیریت کلیک در حالت سیم‌کشی ---
void CircuitScene::handleWireToolPress(QGraphicsSceneMouseEvent *event)
{
    Pin *pin = pinAt(event->scenePos());

    // شروع رسم سیم
    if (!m_drawingWire) {
        if (!pin) {
            event->accept();
            return;
        }
        m_startPin = pin;
        m_drawingWire = true;
        event->accept();
        return;
    }

    // لغو رسم سیم (کلیک مجدد روی همان پین)
    if (pin == m_startPin) {
        m_startPin = nullptr;
        m_drawingWire = false;
        event->accept();
        return;
    }

    // اتصال به پین مقصد
    if (pin) {
        m_currentWire = connectPins(m_startPin, pin);

        if (m_currentWire && m_undoStack)
            m_undoStack->push(new AddWireCommand(this, m_currentWire));

        m_currentWire = nullptr;
        m_startPin = nullptr;
        m_drawingWire = false;
    }

    event->accept();
}
#include "circuitscene.h"
#include "junction.h"
#include "componentfactory.h"
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
#include <QDebug>
#include "net.h"
#include <QKeyEvent>
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
}

//////////////////////////////////////////////////////////
// Mouse Move
//////////////////////////////////////////////////////////

void CircuitScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //-----------------------------------------
    // Ghost Component
    //-----------------------------------------

    if(m_previewItem)
    {
        QPointF p = event->scenePos();

        const int grid = 20;

        p.setX(qRound(p.x()/grid)*grid);
        p.setY(qRound(p.y()/grid)*grid);

        m_previewItem->setPos(p);
    }

    //-----------------------------------------
    // Wire
    //-----------------------------------------

    if(m_drawingWire && m_currentWire)
    {
        if(m_tempWirePoints.size() >= 2)
        {
            m_tempWirePoints.last() = event->scenePos();

            m_currentWire->setPoints(m_tempWirePoints);
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

//////////////////////////////////////////////////////////
// Mouse Press
//////////////////////////////////////////////////////////

void CircuitScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!m_toolManager)
    {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    switch(m_toolManager->currentTool())
    {

        //////////////////////////////////////////////////////
        // SELECT
        //////////////////////////////////////////////////////

    case Tool::Select:
    {
        m_startPositions.clear();

        QList<QGraphicsItem*> list = selectedItems();

        for(QGraphicsItem *item : list)
            m_startPositions[item] = item->pos();

        QGraphicsScene::mousePressEvent(event);

        return;
    }

        //////////////////////////////////////////////////////
        // PLACE COMPONENT
        //////////////////////////////////////////////////////

    case Tool::PlaceComponent:
    {
        if(!m_previewItem)
            return;

        QGraphicsItem *item =
            ComponentFactory::create(m_previewComponent);

        if(!item)
            return;

        item->setPos(m_previewItem->pos());

        QRectF rect = item->sceneBoundingRect();

        bool collision = false;

        QList<QGraphicsItem*> list = items(rect);

        for(QGraphicsItem *other : list)
        {
            if(other == m_previewItem)
                continue;

            if(dynamic_cast<Wire*>(other))
                continue;

            if(dynamic_cast<Pin*>(other))
                continue;

            collision = true;
            break;
        }

        if(!collision)
        {
            if(m_undoStack)
            {
                m_undoStack->push(
                    new AddComponentCommand(this,item));
            }
            else
            {
                addItem(item);
            }
        }
        else
        {
            delete item;
        }

        return;
    }

        //////////////////////////////////////////////////////
        // WIRE
        //////////////////////////////////////////////////////

    case Tool::Wire:
    {
        Pin *pin = pinAt(event->scenePos());

        //////////////////////////////////////////////////////
        // شروع رسم سیم
        //////////////////////////////////////////////////////


        if(!m_drawingWire)
        {
            if(!pin)
            {
                QGraphicsScene::mousePressEvent(event);
                return;
            }

            m_startPin = pin;

            m_currentWire = new Wire();

            m_currentWire->setStartPin(pin);

            QPointF p = pin->scenePos();

            m_currentWire->setStartPoint(p);
            m_currentWire->setEndPoint(p);

            addItem(m_currentWire);

            m_tempWirePoints.clear();
            m_tempWirePoints << p << p;

            m_currentWire->setPoints(m_tempWirePoints);

            m_drawingWire = true;

            return;
        }

        //////////////////////////////////////////////////////
        // کلیک روی صفحه = ایجاد Corner
        //////////////////////////////////////////////////////

        if(!pin)
        {
            m_tempWirePoints.last() = event->scenePos();

            m_tempWirePoints.push_back(event->scenePos());

            m_currentWire->setPoints(m_tempWirePoints);

            return;
        }
        //////////////////////////////////////////////////////
        // لغو
        //////////////////////////////////////////////////////

        if(pin == m_startPin)
        {
            removeItem(m_currentWire);

            delete m_currentWire;

            m_currentWire = nullptr;

            m_startPin = nullptr;

            m_tempWirePoints.clear();

            m_drawingWire = false;

            return;
        }

        //////////////////////////////////////////////////////
        // Node
        //////////////////////////////////////////////////////

        Node *node = createOrGetNode(m_startPin,pin);

        m_startPin->setNode(node);

        pin->setNode(node);

        if(node->junction())
            node->junction()->updatePosition();

        //////////////////////////////////////////////////////
        // پایان سیم
        //////////////////////////////////////////////////////
        m_tempWirePoints.last() = pin->scenePos();

        m_currentWire->setPoints(m_tempWirePoints);
        m_currentWire->setEndPin(pin);

        QVector<QPointF> pts;

        pts << m_startPin->scenePos();

        for(const QPointF &p : m_tempWirePoints)
            pts << p;

        pts << pin->scenePos();

        m_currentWire->setPoints(pts);

        m_currentWire->rebuildGeometry();

        if(m_undoStack)
            m_undoStack->push(new AddWireCommand(this,m_currentWire));

        m_currentWire = nullptr;

        m_startPin = nullptr;

        m_tempWirePoints.clear();

        m_drawingWire = false;

        return;
    }
    }

    QGraphicsScene::mousePressEvent(event);
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
        return n1;
    }


    //-----------------------------------------
    // فقط دومی Node دارد
    //-----------------------------------------

    if(!n1 && n2)
    {
        n2->net()->addPin(p1);
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

#include <QKeyEvent>

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
    if(!m_drawingWire)
        return;

    if(m_currentWire)
    {
        removeItem(m_currentWire);
        delete m_currentWire;
        m_currentWire = nullptr;
    }

    m_startPin = nullptr;
    m_drawingWire = false;
}


void CircuitScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    // ۱. پیدا کردن شیئی که روی آن دبل‌کلیک شده است
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());

    // ۲. بررسی اینکه آیا این شیء یک قطعه (Component) است یا خیر
    Component *comp = dynamic_cast<Component*>(item);

    if (comp) {
        // ۳. گرفتن ویژگی‌های فعلی قطعه
        auto props = comp->getProperties();

        // ۴. باز کردن پنجره هوشمندی که ساختیم
        DynamicPropertiesDialog dialog(props);
        if (dialog.exec() == QDialog::Accepted) {
            // ۵. دریافت مقادیر جدید از پنجره در صورت فشرده شدن دکمه OK
            auto newValues = dialog.getNewValues();

            // ۶. اعمال مقادیر جدید به قطعه
            comp->setProperties(newValues);
        }
    } else {
        // اگر روی قطعه کلیک نشده بود، رفتار پیش‌فرض سیستمی اجرا شود
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

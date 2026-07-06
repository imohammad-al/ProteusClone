#include "circuitscene.h"
#include "junction.h"
#include "componentfactory.h"
#include "wire.h"
#include "pin.h"
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
        m_currentWire->setEndPoint(event->scenePos());
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

        if(!pin)
        {
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        //-----------------------------------------
        // FIRST CLICK
        //-----------------------------------------

        if(!m_drawingWire)
        {
            m_startPin = pin;

            m_currentWire = new Wire();

            m_currentWire->setStartPin(pin);

            QPointF p = pin->scenePos();

            m_currentWire->setStartPoint(p);
            m_currentWire->setEndPoint(p);

            m_currentWire->rebuildGeometry();

            addItem(m_currentWire);

            m_drawingWire = true;

            return;
        }

        //-----------------------------------------
        // CANCEL
        //-----------------------------------------

        if(pin == m_startPin)
        {
            delete m_currentWire;

            m_currentWire = nullptr;

            m_startPin = nullptr;

            m_drawingWire = false;

            return;
        }

        //-----------------------------------------
        // NODE SYSTEM
        //-----------------------------------------

        Node *node = createOrGetNode(m_startPin, pin);

        m_startPin->setNode(node);

        pin->setNode(node);

        if(node->junction())
            node->junction()->updatePosition();

        //-----------------------------------------
        // FINISH WIRE
        //-----------------------------------------

        m_currentWire->setEndPin(pin);

        m_currentWire->rebuildGeometry();

        if(m_undoStack)
        {
            m_undoStack->push(
                new AddWireCommand(
                    this,
                    m_currentWire));
        }

        m_currentWire = nullptr;

        m_startPin = nullptr;

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
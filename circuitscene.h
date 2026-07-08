#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include "toolmanager.h"
#include "node.h"
#include <QGraphicsScene>
#include <QString>
#include <QMap>
class MoveCommand;
class Wire;
class Pin;
class Net;


class QUndoStack;
class CircuitScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CircuitScene(QObject *parent = nullptr);

    void setComponentToPlace(const QString &name);

    void setToolManager(ToolManager *manager);

    void beginComponentPlacement(const QString &name);

    void cancelComponentPlacement();

    void cancelPlacement();

    void setUndoStack(QUndoStack *stack);

    void mergeNodes(Node *a, Node *b);

    void cancelWireDrawing();

protected:
    void drawBackground(QPainter *painter,
                        const QRectF &rect) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;


private:
    Pin *pinAt(const QPointF &scenePos) const;

    QString m_componentToPlace;

    QUndoStack *m_undoStack = nullptr;

    Wire *m_currentWire = nullptr;

    Pin *m_startPin = nullptr;

    bool m_drawingWire = false;

    ToolManager *m_toolManager = nullptr;

    QGraphicsItem *m_previewItem = nullptr;

    QString m_previewComponent;

    MoveCommand *m_moveCommand = nullptr;

    QMap<QGraphicsItem*, QPointF> m_startPositions;

    QList<Node*> m_nodes;

    Node* createOrGetNode(Pin *p1, Pin *p2);

    QList<Net*> m_nets;

    QVector<QPointF> m_tempWirePoints;
};

#endif
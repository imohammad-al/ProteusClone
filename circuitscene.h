#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include "toolmanager.h"
#include "node.h"
#include <QGraphicsScene>
#include <QString>
#include <QMap>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
class MoveCommand;
class Wire;
class Pin;
class Net;
class Component;


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

    // --- دسترسی فقط‌خواندنی به محتوای مدار (برای ProjectSerializer و DesignRuleChecker) ---
    QList<Component*> components() const;
    QList<Wire*> wires() const { return m_wires; }
    QList<Node*> nodes() const { return m_nodes; }

    // --- ساخت یک اتصال سیم بین دو پایه بدون وابستگی به رویداد ماوس ---
    // هم توسط ابزار سیم‌کشی و هم توسط ProjectSerializer::load برای بازسازی سیم‌ها استفاده می‌شود
    Wire* connectPins(Pin *a, Pin *b);

    // --- جدا کردن یک سیم موجود از توپولوژی مدار (Pin/Node/Net) و صحنه گرافیکی ---
    // شیء Wire حذف (delete) نمی‌شود تا DeleteWireCommand بتواند آن را Undo کند.
    // در صورت لزوم، گره مشترک را به دو گره مستقل می‌شکند (splitNodeIfNeeded).
    void disconnectWire(Wire *wire);

    // --- اتصال دوباره یک سیم موجود (برای Undo حذف سیم) ---
    void reattachWire(Wire *wire);

    // --- پاک‌سازی کامل مدار (قطعات، سیم‌ها، گره‌ها) برای پروژه جدید یا بارگذاری ---
    void resetCircuit();

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

    void handleSelectToolPress(QGraphicsSceneMouseEvent *event);
    void handlePlaceComponentToolPress(QGraphicsSceneMouseEvent *event);
    void handleWireToolPress(QGraphicsSceneMouseEvent *event);

    // متغیر کمکی برای بهینه‌سازی سرعت ماوس (در گام دوم توضیح داده شده)
    QPointF m_lastGridPos;

    QString m_previewComponent;

    MoveCommand *m_moveCommand = nullptr;

    QMap<QGraphicsItem*, QPointF> m_startPositions;

    QList<Node*> m_nodes;

    Node* createOrGetNode(Pin *p1, Pin *p2);

    // بعد از جدا شدن یک سیم از یک گره، بررسی می‌کند آیا پین‌های باقیمانده آن گره
    // هنوز (از طریق سیم‌های دیگر) به هم متصل هستند یا باید به چند گره مستقل بشکند.
    void splitNodeIfNeeded(Node *node);

    QList<Net*> m_nets;

    QVector<QPointF> m_tempWirePoints;

    // ردیابی مرکزی همه سیم‌های مدار (برای DRC، Serializer و حذف تمیز)
    QList<Wire*> m_wires;
};

#endif
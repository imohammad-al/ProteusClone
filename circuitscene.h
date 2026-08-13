#ifndef CIRCUITSCENE_H
#define CIRCUITSCENE_H

#include "toolmanager.h"
#include "node.h"
#include <QGraphicsScene>
#include <QString>
#include <QMap>
#include <QRectF>
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

    // --- محدوده شماتیک/کادر آبی (بخش ۱ درخواست کاربر) ---
    // اندازه‌اش دقیقاً برابر اندازه‌ای است که کاربر در StartupDialog (یا فایل پروژه
    // بارگذاری‌شده) انتخاب کرده. این متد sceneRect صحنه را هم خودکار بزرگ‌تر از این
    // کادر تنظیم می‌کند تا پس‌زمینه شطرنجی دور کادر هم قابل مشاهده/اسکرول باشد.
    void setSchematicRect(const QRectF &rect);
    QRectF schematicRect() const { return m_schematicRect; }

    // آیا یک قطعه (بر اساس مرکز مستطیل صحنه‌ای‌اش) داخل کادر آبی است؟ ملاک تصمیم
    // «مرکز» است نه کل bounding rect، تا قطعاتی که فقط لبه‌شان کمی از کادر بیرون
    // زده به‌طور غافلگیرکننده‌ای از شبیه‌سازی حذف نشوند.
    bool isWithinSchematicBounds(const Component *component) const;

    // فقط قطعات داخل کادر آبی - SimulationEngine و AnalogSolver برای اجرای واقعی
    // شبیه‌سازی از این متد استفاده می‌کنند (نه components()) تا قطعات بیرون از کادر
    // در شبیه‌سازی شرکت داده نشوند. Save/Load و DRC کماکان از components() کامل
    // استفاده می‌کنند - قطعات بیرون کادر باید ذخیره/بارگذاری و بررسی DRC بشوند،
    // فقط در چرخه شبیه‌سازی شرکت نمی‌کنند.
    QList<Component*> componentsInSchematic() const;

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

    // نزدیک‌ترین سیم به یک نقطه دلخواه (فضای بوم) را در صورتی که فاصله‌اش از
    // maxDistance کمتر باشد برمی‌گرداند و نزدیک‌ترین پایه (شروع یا پایان) آن سیم
    // را در outNearestPin می‌گذارد. برای اسنپ خودکار پین به سیم هنگام قرار دادن
    // قطعه استفاده می‌شود (بخش ۲ درخواست کاربر).
    Wire* findNearbyWire(const QPointF &scenePos, qreal maxDistance, Pin **outNearestPin) const;

    // بعد از جدا شدن یک سیم از یک گره، بررسی می‌کند آیا پین‌های باقیمانده آن گره
    // هنوز (از طریق سیم‌های دیگر) به هم متصل هستند یا باید به چند گره مستقل بشکند.
    void splitNodeIfNeeded(Node *node);

    QList<Net*> m_nets;

    QVector<QPointF> m_tempWirePoints;

    // ردیابی مرکزی همه سیم‌های مدار (برای DRC، Serializer و حذف تمیز)
    QList<Wire*> m_wires;

    // محدوده شماتیک/کادر آبی فعلی (بخش ۱ درخواست کاربر) - پیش‌فرض نامعتبر تا
    // setSchematicRect صدا زده شود؛ MainWindow همیشه در سازنده یک مقدار پیش‌فرض
    // (۳۰۰۰x۳۰۰۰) تنظیم می‌کند، پس در عمل همیشه معتبر است.
    QRectF m_schematicRect;
};

#endif
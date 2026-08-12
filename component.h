#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsObject>
#include <QUuid>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QList>
#include <QJsonObject>

class Pin;
class QMenu;
class QGraphicsSceneContextMenuEvent;
// کلاس فرضی برای شبیه‌سازی که بعداً پیاده‌سازی می‌شود
class SimulationElement { /* ... */ };

struct ComponentProperty {
    QString label;
    QString displayName;  // مورد استفاده در component.cpp
    QVariant value;
    QString type;
    QString unit;
};

class Component : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Component(QGraphicsItem *parent = nullptr);
    virtual ~Component();

    // --- ۱) اطلاعات شناسایی (Identity) ---
    QUuid id() const { return m_id; }
    void setId(const QUuid &id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString componentType() const { return m_type; }
    void setComponentType(const QString &type) { m_type = type; }

    QString category() const { return m_category; }
    void setCategory(const QString &cat) { m_category = cat; }

    // --- ۳) وضعیت (State) ---
    bool isLocked() const { return m_locked; }
    void setLocked(bool locked) { m_locked = locked; }

    // --- ۴) پایه‌ها (Pins) ---
    QList<Pin*> pins() const { return m_pins; }
    int pinCount() const { return m_pins.size(); }
    void addPin(Pin* pin);
    void removePin(Pin* pin);
    Pin* pin(int index) const;

    // --- کلاس Properties (پویا بر اساس QMap) ---
    void setProperty(const QString &key, const QVariant &value, const QString &displayName = "", const QString &unit = "");
    QVariant property(const QString &key) const;
    bool hasProperty(const QString &key) const;
    void removeProperty(const QString &key);
    QMap<QString, ComponentProperty> allProperties() const { return m_properties; }

    // --- Metadata & Library ---
    QString manufacturer;
    QString model;
    QString description;
    QString package;
    QString libraryName;
    QString symbolName;
    QString footprintName;

    // --- ۵) توابع اجباری و مجازی اصلی پروپوزال ---
    virtual QRectF boundingRect() const override = 0;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;
    virtual QPainterPath shape() const override = 0;

    virtual Component* clone() const = 0;
    virtual bool isValid() = 0;
    virtual SimulationElement simulationModel() = 0;

    // --- Serialization (Save/Load) ---
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject &json);

    virtual void updateConnections() {}
    virtual QMap<QString, ComponentProperty> getProperties() const {
        return QMap<QString, ComponentProperty>();
    }
    virtual void setProperties(const QMap<QString, QVariant>& newValues) {
        Q_UNUSED(newValues);
    }

signals:
    // --- Events (سیگنال‌های درخواستی پروپوزال) ---
    void positionChanged(QPointF newPos);
    void rotationChanged(qreal newRotation);
    void propertyChanged(QString key, QVariant newValue);
    void pinConnected(Pin* pin);
    void pinDisconnected(Pin* pin);

protected:
    // مدیریت تغییرات هندسی و اتصالات برای پین‌ها و سیگنال‌ها
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // Undo Hooks
    virtual void beforeChange() {}
    virtual void afterChange() {}
    // توجه: mouseDoubleClickEvent اینجا عمداً override نشده — باز کردن دیالوگ ویژگی‌ها
    // به‌صورت متمرکز توسط CircuitScene::mouseDoubleClickEvent (با DynamicPropertiesDialog)
    // انجام می‌شود. قبلاً اینجا یک override اضافه با کلاس ناموجود ComponentPropertiesDialog
    // بود که هم کامپایل را می‌شکست و هم به دلیل event interception در Scene هیچ‌وقت اجرا نمی‌شد.

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    // قلاب برای زیرکلاس‌ها تا آیتم‌های راست‌کلیک اختصاصی خودشان را اضافه کنند
    // (مثلاً MCU برای افزودن "Load HEX File..." - بخش ۷.۲ مستند). پیاده‌سازی پیش‌فرض
    // خالی است؛ اگر هیچ زیرکلاسی چیزی اضافه نکند، رفتار دقیقاً مثل قبل (بدون منو) می‌ماند.
    virtual void populateContextMenu(QMenu *menu) { Q_UNUSED(menu); }

    // نشانگر بصری انتخاب: هر زیرکلاس این را در انتهای paint() خودش صدا می‌زند (یک خط اضافه).
    // وقتی قطعه انتخاب شده باشد (option->state & QStyle::State_Selected)، یک هایلایت نیمه‌شفاف
    // رنگی + کادر نقطه‌چین روی boundingRect() قطعه رسم می‌کند تا انتخاب با تغییر رنگ محسوس باشد؛
    // در غیر این صورت هیچ کاری نمی‌کند (بدون افت کارایی برای قطعات انتخاب‌نشده).
    void paintSelectionOverlay(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
private:
    // Identity
    QUuid m_id;
    QString m_name;
    QString m_type;
    QString m_category;

    // Status
    bool m_locked = false;

    // Components Links
    QList<Pin*> m_pins;
    QMap<QString, ComponentProperty> m_properties;
};

#endif // COMPONENT_H
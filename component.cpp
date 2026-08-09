#include "component.h"
#include "pin.h" // فرض بر این است که هدر کلاس Pin شما موجود است
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>




Component::Component(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    m_id = QUuid::createUuid(); // تولید شناسه یکتا به محض ساخت قطعه

    // فعال‌سازی فلگ‌های حیاتی برای تعامل، انتخاب و ارسال سیگنال جابجایی در Qt Graphics View
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
}

Component::~Component()
{
    // پاک کردن پین‌ها از حافظه هنگام حذف قطعه
    qDeleteAll(m_pins);
}

// --- مدیریت پایه‌ها ---
void Component::addPin(Pin* pin)
{
    if (pin && !m_pins.contains(pin)) {
        m_pins.append(pin);
    }
}

void Component::removePin(Pin* pin)
{
    if (pin) {
        m_pins.removeOne(pin);
    }
}

Pin* Component::pin(int index) const
{
    if (index >= 0 && index < m_pins.size()) {
        return m_pins.at(index);
    }
    return nullptr;
}

// --- مدیریت ویژگی‌های پویا (Properties) ---
void Component::setProperty(const QString &key, const QVariant &value, const QString &displayName, const QString &unit)
{
    beforeChange();
    if (m_properties.contains(key)) {
        m_properties[key].value = value;
    } else {
        ComponentProperty prop;
        prop.displayName = displayName.isEmpty() ? key : displayName;
        prop.value = value;
        prop.type = value.typeName();
        prop.unit = unit;
        m_properties[key] = prop;
    }
    emit propertyChanged(key, value);
    afterChange();
    update(); // بازرسم قطعه روی بوم
}

QVariant Component::property(const QString &key) const
{
    return m_properties.value(key).value;
}

bool Component::hasProperty(const QString &key) const
{
    return m_properties.contains(key);
}

void Component::removeProperty(const QString &key)
{
    if (m_properties.contains(key)) {
        m_properties.remove(key);
    }
}

// --- Serialization (ذخیره پروژه به JSON) ---
QJsonObject Component::toJson() const
{
    QJsonObject json;
    json["id"] = m_id.toString();
    json["name"] = m_name;
    json["type"] = m_type;
    json["category"] = m_category;
    json["x"] = pos().x();
    json["y"] = pos().y();
    json["rotation"] = rotation();
    json["zValue"] = zValue();

    // ذخیره متاداده ویژگی‌ها
    QJsonObject propsJson;
    for (auto it = m_properties.begin(); it != m_properties.end(); ++it) {
        propsJson[it.key()] = QJsonValue::fromVariant(it.value().value);
    }
    json["properties"] = propsJson;

    return json;
}

// --- Deserialization (بارگذاری پروژه از JSON) ---
void Component::fromJson(const QJsonObject &json)
{
    beforeChange();
    if (json.contains("id")) m_id = QUuid::fromString(json["id"].toString());
    if (json.contains("name")) m_name = json["name"].toString();
    if (json.contains("type")) m_type = json["type"].toString();
    if (json.contains("category")) m_category = json["category"].toString();

    if (json.contains("x") && json.contains("y")) {
        setPos(json["x"].toDouble(), json["y"].toDouble());
    }
    if (json.contains("rotation")) setRotation(json["rotation"].toDouble());
    if (json.contains("zValue")) setZValue(json["zValue"].toDouble());

    if (json.contains("properties")) {
        QJsonObject propsJson = json["properties"].toObject();
        for (const QString &key : propsJson.keys()) {
            if (m_properties.contains(key)) {
                m_properties[key].value = propsJson[key].toVariant();
            } else {
                setProperty(key, propsJson[key].toVariant());
            }
        }
    }
    afterChange();
}

// --- هوک هوشمند کنترل رویدادهای جابجایی و چرخش قطعه ---
QVariant Component::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && scene()) {
        QPointF newPos = value.toPointF();

        emit positionChanged(pos());
        for (Pin* p : m_pins) if (p) p->updateWires();
    }



    if (change == ItemRotationChange) {
        emit rotationChanged(value.toReal());
        for (Pin* p : m_pins) {
            if (p) p->updateWires();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

// mouseDoubleClickEvent عمداً اینجا حذف شد — به component.h مراجعه کن.

void Component::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    populateContextMenu(&menu);

    if (menu.isEmpty()) {
        // بدون این رفتار، کلیک راست روی قطعاتی که هیچ آیتم اختصاصی اضافه نمی‌کنند
        // (اکثر قطعات) دقیقاً مثل قبل از این تغییر نادیده گرفته می‌شود - بدون هیچ تفاوتی.
        event->ignore();
        return;
    }

    menu.exec(event->screenPos());
    event->accept();
}


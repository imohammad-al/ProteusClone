#include "projectserializer.h"
#include "../circuitscene.h"
#include "../component.h"
#include "../componentfactory.h"
#include "../pin.h"
#include "../wire.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QHash>

static const int kProjectFormatVersion = 1;

bool ProjectSerializer::save(CircuitScene *scene, const QString &filePath, QString *errorMessage)
{
    if (!scene) {
        if (errorMessage) *errorMessage = QObject::tr("مداری برای ذخیره‌سازی وجود ندارد.");
        return false;
    }

    QJsonObject root;
    root["formatVersion"] = kProjectFormatVersion;

    // --- ۱. سریالایز کردن قطعات (Component::toJson از قبل id/name/type/category/x/y/rotation/properties را می‌نویسد) ---
    QJsonArray componentsArray;
    const QList<Component*> comps = scene->components();
    for (Component *c : comps) {
        componentsArray.append(c->toJson());
    }
    root["components"] = componentsArray;

    // --- ۲. سریالایز کردن سیم‌ها بر اساس (شناسه قطعه + اندیس پایه) که پایدار و مستقل از UUID پایه است ---
    QJsonArray wiresArray;
    const QList<Wire*> allWires = scene->wires();
    for (Wire *w : allWires) {
        Pin *startPin = w->startPin();
        Pin *endPin = w->endPin();
        if (!startPin || !endPin) continue;

        Component *startComp = dynamic_cast<Component*>(startPin->parentItem());
        Component *endComp = dynamic_cast<Component*>(endPin->parentItem());
        if (!startComp || !endComp) continue;

        const int startIdx = startComp->pins().indexOf(startPin);
        const int endIdx = endComp->pins().indexOf(endPin);
        if (startIdx < 0 || endIdx < 0) continue;

        QJsonObject wj;
        wj["fromComponentId"] = startComp->id().toString();
        wj["fromPinIndex"] = startIdx;
        wj["toComponentId"] = endComp->id().toString();
        wj["toPinIndex"] = endIdx;
        wiresArray.append(wj);
    }
    root["wires"] = wiresArray;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QObject::tr("امکان نوشتن در فایل \"%1\" وجود ندارد.").arg(filePath);
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ProjectSerializer::load(CircuitScene *scene, const QString &filePath, QString *errorMessage)
{
    if (!scene) {
        if (errorMessage) *errorMessage = QObject::tr("صحنه‌ای برای بارگذاری مدار وجود ندارد.");
        return false;
    }

    if (!QFileInfo::exists(filePath)) {
        if (errorMessage) *errorMessage = QObject::tr("فایل \"%1\" یافت نشد.").arg(filePath);
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QObject::tr("امکان باز کردن فایل \"%1\" وجود ندارد.").arg(filePath);
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) *errorMessage = QObject::tr("فایل پروژه معتبر نیست: %1").arg(parseError.errorString());
        return false;
    }

    const QJsonObject root = doc.object();

    // پاک کردن کامل مدار فعلی قبل از بازسازی (رفع نشتی حافظه Node/Net + Component/Wire قدیمی)
    scene->resetCircuit();

    // --- ۱. بازسازی قطعات با استفاده از ComponentFactory (بر اساس همان نامی که در LibraryManager ثبت شده) ---
    QHash<QString, Component*> idToComponent;

    const QJsonArray componentsArray = root["components"].toArray();
    for (const QJsonValue &v : componentsArray) {
        const QJsonObject cj = v.toObject();
        const QString type = cj["type"].toString();

        QGraphicsItem *item = ComponentFactory::create(type);
        Component *comp = dynamic_cast<Component*>(item);
        if (!comp) {
            // نوع قطعه ناشناخته است (مثلاً فایل متعلق به نسخه دیگری از برنامه است) - آن را نادیده می‌گیریم
            delete item;
            continue;
        }

        comp->fromJson(cj);
        scene->addItem(comp);
        idToComponent.insert(comp->id().toString(), comp);
    }

    // --- ۲. بازسازی سیم‌ها با اتصال مجدد بر اساس (شناسه قطعه + اندیس پایه) ---
    const QJsonArray wiresArray = root["wires"].toArray();
    for (const QJsonValue &v : wiresArray) {
        const QJsonObject wj = v.toObject();

        Component *fromComp = idToComponent.value(wj["fromComponentId"].toString(), nullptr);
        Component *toComp = idToComponent.value(wj["toComponentId"].toString(), nullptr);
        if (!fromComp || !toComp) continue;

        Pin *fromPin = fromComp->pin(wj["fromPinIndex"].toInt(-1));
        Pin *toPin = toComp->pin(wj["toPinIndex"].toInt(-1));
        if (!fromPin || !toPin) continue;

        scene->connectPins(fromPin, toPin);
    }

    return true;
}

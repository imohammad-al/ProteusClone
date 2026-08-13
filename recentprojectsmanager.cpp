#include "recentprojectsmanager.h"

#include <QSettings>
#include <QFileInfo>

const char *RecentProjectsManager::kSettingsKey = "recent/projects";
const int RecentProjectsManager::kMaxStored = 20;

QStringList RecentProjectsManager::recentProjects(int maxCount)
{
    QSettings settings;
    const QStringList stored = settings.value(QLatin1String(kSettingsKey)).toStringList();

    QStringList result;
    for (const QString &path : stored) {
        if (!QFileInfo::exists(path))
            continue; // فایل حذف/جابه‌جا شده - در فهرست نمایشی نادیده گرفته می‌شود
        result.append(path);
        if (result.size() >= maxCount)
            break;
    }
    return result;
}

void RecentProjectsManager::addRecentProject(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    QSettings settings;
    QStringList stored = settings.value(QLatin1String(kSettingsKey)).toStringList();

    stored.removeAll(filePath);
    stored.prepend(filePath);

    while (stored.size() > kMaxStored)
        stored.removeLast();

    settings.setValue(QLatin1String(kSettingsKey), stored);
}

void RecentProjectsManager::removeRecentProject(const QString &filePath)
{
    QSettings settings;
    QStringList stored = settings.value(QLatin1String(kSettingsKey)).toStringList();
    stored.removeAll(filePath);
    settings.setValue(QLatin1String(kSettingsKey), stored);
}

#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QList>

#include "componentinfo.h"

class LibraryManager
{
public:

    static void initialize();

    static QList<ComponentInfo> components();

private:

    static QList<ComponentInfo> m_components;
};

#endif
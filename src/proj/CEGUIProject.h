#ifndef CEGUIPROJECT_H
#define CEGUIPROJECT_H

#include "qstring.h"

// Incapsulates a single CEGUI (CEED) project info and methods to work with it

class CEGUIProject
{
public:

    CEGUIProject();

    bool loadFromFile(const QString& fileName);

private:

    QString defaultResolution;
};

#endif // CEGUIPROJECT_H

#ifndef UTILS_H
#define UTILS_H

#include "qbrush.h"

namespace Utils
{

QBrush getCheckerboardBrush(int halfWidth = 5, int halfHeight = 5, QColor firstColour = Qt::darkGray, QColor secondColour = Qt::gray);
void fillTransparencyWithChecker(QImage& image, int halfWidth = 5, int halfHeight = 5, QColor firstColour = Qt::darkGray, QColor secondColour = Qt::gray);

};

#endif // UTILS_H

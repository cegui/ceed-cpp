#include "src/util/Utils.h"
#include <qpainter.h>

namespace Utils
{

// Small helper function that generates a brush usually seen in graphics editing tools. The checkerboard brush
// that draws background seen when edited images are transparent.
QBrush getCheckerboardBrush(int halfWidth, int halfHeight, QColor firstColour, QColor secondColour)
{
    // Disallow too large half sizes to prevent crashes in QPainter and slowness in general
    halfWidth = std::min(halfWidth, 256);
    halfHeight = std::min(halfHeight, 256);

    QBrush ret;
    QPixmap texture(2 * halfWidth, 2 * halfHeight);

    // Render checker
    {
        QPainter painter(&texture);
        painter.fillRect(0, 0, halfWidth, halfHeight, firstColour);
        painter.fillRect(halfWidth, halfHeight, halfWidth, halfHeight, firstColour);
        painter.fillRect(halfWidth, 0, halfWidth, halfHeight, secondColour);
        painter.fillRect(0, halfHeight, halfWidth, halfHeight, secondColour);
    }

    ret.setTexture(texture);

    return ret;
}

void fillTransparencyWithChecker(QImage& image, int halfWidth, int halfHeight, QColor firstColour, QColor secondColour)
{
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setBrush(getCheckerboardBrush(halfWidth, halfHeight, firstColour, secondColour));
    painter.drawRect(image.rect());
    painter.end();
}

};

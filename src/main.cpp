#include "src/Application.h"
#include <qsurfaceformat.h>

int main(int argc, char *argv[])
{
    // Request OpenGL for our global share context
    QSurfaceFormat format;
    format.setSamples(0);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Allow rendering to QOpenGLWidget using our explicit context
    Application::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    Application a(argc, argv);
    return a.exec();
}

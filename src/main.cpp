#include "src/Application.h"

int main(int argc, char *argv[])
{
    // Allow rendering to QOpenGLWidget using our explicit context
    Application::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    Application a(argc, argv);
    return a.exec();
}

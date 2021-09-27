#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/Application.h"
#include <CEGUI/RendererModules/OpenGL/RendererBase.h>
#include <CEGUI/RendererModules/OpenGL/ViewportTarget.h>
#include <CEGUI/System.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/FontManager.h>
#include <qdatetime.h>
#include <qgraphicsitem.h>
#include <qgraphicssceneevent.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qopenglframebufferobject.h>
#include <qmessagebox.h>
#include <qdir.h>

static void validateResolution(float& width, float& height)
{
    if (width < 1.f || height < 1.f)
    {
        auto project = CEGUIManager::Instance().getCurrentProject();
        if (project)
        {
            width = project->getDefaultResolution().width();
            height = project->getDefaultResolution().height();
        }
        else
        {
            width = 1280.f;
            height = 720.f;
        }
    }
}

CEGUIGraphicsScene::CEGUIGraphicsScene(QObject* parent, float width, float height)
    : QGraphicsScene(parent)
    , timeOfLastRender(QDateTime::currentMSecsSinceEpoch())
{
    validateResolution(width, height);
    contextWidth = width;
    contextHeight = height;

    const qreal qWidth = static_cast<qreal>(width);
    const qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    auto renderer = static_cast<CEGUI::OpenGLRendererBase*>(CEGUI::System::getSingleton().getRenderer());
    auto renderTarget = new CEGUI::OpenGLViewportTarget(*renderer, CEGUI::Rectf(0.f, 0.f, contextWidth, contextHeight));
    ceguiContext = &CEGUI::System::getSingleton().createGUIContext(*renderTarget);
}

CEGUIGraphicsScene::~CEGUIGraphicsScene()
{
    if (_fbo)
    {
        CEGUIManager::Instance().makeOpenGLContextCurrent();
        delete _fbo;
        CEGUIManager::Instance().doneOpenGLContextCurrent();
    }

    if (ceguiContext)
    {
        auto renderTarget = &ceguiContext->getRenderTarget();
        CEGUI::System::getSingleton().destroyGUIContext(*ceguiContext);
        delete renderTarget;
    }
}

void CEGUIGraphicsScene::setCEGUIDisplaySize(float width, float height)
{
    validateResolution(width, height);

    if (qFuzzyCompare(contextWidth, width) && qFuzzyCompare(contextHeight, height)) return;

    contextWidth = width;
    contextHeight = height;

    const qreal qWidth = static_cast<qreal>(width);
    const qreal qHeight = static_cast<qreal>(height);
    setSceneRect(QRectF(-padding, -padding, qWidth + 2.0 * padding, qHeight + 2.0 * padding));

    CEGUI::Sizef newSize(contextWidth, contextHeight);
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(newSize);

    auto& renderTarget = ceguiContext->getRenderTarget();
    auto area = renderTarget.getArea();
    area.setSize(newSize);
    renderTarget.setArea(area);

    update();
}

// Renders CEGUI context to texture using FBO. All shared contexts can then access FBO texture.
void CEGUIGraphicsScene::drawCEGUIContextOffscreen()
{
    if (!ceguiContext) return;

    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
    lastDelta = currTime - timeOfLastRender;
    timeOfLastRender = currTime;

    // Inject the time passed since the last render all at once
    CEGUI::System::getSingleton().injectTimePulse(static_cast<float>(lastDelta));
    ceguiContext->injectTimePulse(static_cast<float>(lastDelta));

    drawCEGUIContextInternal();
    CEGUIManager::Instance().doneOpenGLContextCurrent();
}

QImage CEGUIGraphicsScene::getCEGUIScreenshot()
{
    if (!ceguiContext) return QImage();

    drawCEGUIContextInternal();

    QImage result = _fbo->toImage();

    CEGUIManager::Instance().doneOpenGLContextCurrent();

    return result;
}

QList<QGraphicsItem*> CEGUIGraphicsScene::topLevelItems() const
{
    QList<QGraphicsItem*> ret;
    for (QGraphicsItem* item : items())
    {
        if (!item->parentItem())
            ret.push_back(item);
    }

    return ret;
}

bool CEGUIGraphicsScene::ensureDefaultFontExists()
{
    if (ceguiContext && ceguiContext->getDefaultFont()) return true;

    const auto* currentProject = CEGUIManager::Instance().getCurrentProject();
    if (!currentProject) return false;

    auto& fontManager = CEGUI::FontManager::getSingleton();
    const auto& fontRegistry = fontManager.getRegisteredFonts();
    CEGUI::Font* defaultFont = fontRegistry.empty() ? nullptr : fontRegistry.begin()->second;

    // If project has no fonts, offer the user to autocreate a default font
    if (!defaultFont)
    {
        auto ret = QMessageBox::question(qobject_cast<Application*>(qApp)->getMainWindow(),
                                         "Project has no fonts",
                                         "Do you want CEED to create a default font? Note that some widgets may render wrong without a font.\n",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);
        if (ret != QMessageBox::Yes) return false;

        // Copy font source file if not exist
        const auto dstFontPath = currentProject->getResourceFilePath("DejaVuSans.ttf", "fonts");
        if (!QFileInfo(dstFontPath).exists())
        {
            const auto srcFontPath = QDir::current().filePath("data/fonts/DejaVuSans.ttf");
            if (!QFile(srcFontPath).copy(dstFontPath)) return false;
        }

        // Create CEGUI font object
        const QSizeF& resolution = currentProject->getDefaultResolution();
        try
        {
            defaultFont = &fontManager.createFreeTypeFont("Default", 14.f, CEGUI::FontSizeUnit::Pixels,
                true, "DejaVuSans.ttf", "fonts", CEGUI::AutoScaledMode::Disabled,
                CEGUI::Sizef(static_cast<float>(resolution.width()), static_cast<float>(resolution.height())));
        }
        catch (...)
        {
            return false;
        }

        // Save font XML description and add it to all schemes
        // FIXME: createFont() with family, size etc instead of this, add to particular schemes optionally!
        CEGUIManager::Instance().saveFont(*defaultFont, true);
    }

    if (ceguiContext) ceguiContext->setDefaultFont(defaultFont);
    return true;
}

void CEGUIGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Ignore item interaction for rubber band selection in a CEGUIGraphicsView
    if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
    {
        event->ignore();
        return;
    }

    // Accept right button click to preserve selected items for the context menu
    if (event->button() == Qt::RightButton)
    {
        event->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

// NB: it doesn't disable context, callers may need it for further operations
void CEGUIGraphicsScene::drawCEGUIContextInternal()
{
    if (!ceguiContext) return;

    CEGUIManager::Instance().ensureCEGUIInitialized();
    CEGUIManager::Instance().makeOpenGLContextCurrent();

    // FBO is not per-view at least for now because we render only one CEGUI view at a time anyway
    const int w = static_cast<int>(contextWidth);
    const int h = static_cast<int>(contextHeight);
    if (!_fbo || _fbo->size().width() != w || _fbo->size().height() != h)
    {
        if (_fbo) delete _fbo;
        _fbo = new QOpenGLFramebufferObject(w, h);
    }

    if (_fbo->bind())
    {
        auto gl = QOpenGLContext::currentContext()->functions();
        gl->glClearColor(0.f, 0.f, 0.f, 0.f);
        gl->glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        auto renderer = CEGUI::System::getSingleton().getRenderer();
        renderer->beginRendering();
        ceguiContext->draw();
        renderer->endRendering();

        _fbo->release();
    }
}

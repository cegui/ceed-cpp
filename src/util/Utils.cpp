#include "src/util/Utils.h"
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qprocess.h>

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

// Copied from QtCreator sources
// https://github.com/qt-creator/qt-creator/blob/master/src/plugins/coreplugin/fileutils.cpp
// Copyright (C) 2016 The Qt Company Ltd.
// https://www.gnu.org/licenses/gpl-3.0.html
// Modified by CEED C++ team to get rid of dependencies
bool showInGraphicalShell(const QString& path)
{
    const QFileInfo fileInfo(path);
#ifdef Q_OS_WIN
    QStringList param;
    if (!fileInfo.isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    return QProcess::startDetached("explorer.exe", param);
#elif Q_OS_MAC
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(fileInfo.canonicalFilePath());
    if (!QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs)) return false;
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    return QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
    // TODO: Unix/Linux
    // we cannot select a file here, because no file browser really supports it...
    const QString folder = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
    /*
    const QString app = UnixUtils::fileBrowser(ICore::settings());
    QStringList browserArgs = Utils::QtcProcess::splitArgs(
                UnixUtils::substituteFileBrowserParameters(app, folder));
    QString error;
    if (browserArgs.isEmpty()) {
        error = "The command for file browser is not set";
    } else {
        QProcess browserProc;
        browserProc.setProgram(browserArgs.takeFirst());
        browserProc.setArguments(browserArgs);
        const bool success = browserProc.startDetached();
        error = QString::fromLocal8Bit(browserProc.readAllStandardError());
        if (!success && error.isEmpty())
            error = "Error while starting file browser";
    }
    if (!error.isEmpty())
        //showGraphicalShellError(parent, app, error);
    return error.isEmpty();
    */
#endif
    return false;
}

};

#include "src/util/Utils.h"
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qsettings.h>
#include <qapplication.h>
#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>
#ifdef Q_OS_WIN
#include <initguid.h>
#include <netlistmgr.h>
#endif
#undef min
#undef max

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
    return false;
#endif
}

void registerFileAssociation(const QString& extension, const QString& desc, const QString& mimeType, const QString& perceivedType, int iconIndex)
{
    if (extension.isEmpty()) return;

#if defined(Q_OS_WIN)
    // Register file type on Windows
    // NB: association will be created with the last launched instance, beware if you have multiple CEED distributions
    {
        const QString AppPath = qApp->applicationFilePath().replace('/', '\\');
        const QString openCommand(AppPath + " \"%1\"");
        const QString fileIconPath(AppPath + "," + QString::number(iconIndex));
        QSettings classesKey("HKEY_CURRENT_USER\\SOFTWARE\\Classes", QSettings::NativeFormat);
        if (classesKey.contains("CEGUI.CEED." + extension + "/shell/Open/Command/."))
        {
            // Patch application path if changed
            if (iconIndex >= 0 && classesKey.value("CEGUI.CEED." + extension + "/DefaultIcon/.").toString() != fileIconPath)
                classesKey.setValue("CEGUI.CEED." + extension + "/DefaultIcon/.", fileIconPath);
            if (classesKey.value("CEGUI.CEED." + extension + "/shell/Open/Command/.").toString() != openCommand)
                classesKey.setValue("CEGUI.CEED." + extension + "/shell/Open/Command/.", openCommand);
        }
        else
        {
            // Remove key hierarchies being created, just in case
            classesKey.remove("." + extension);
            classesKey.remove("CEGUI.CEED." + extension);

            if (!desc.isEmpty())
                classesKey.setValue("." + extension + "/.", "CEGUI.CEED." + extension);
            if (!mimeType.isEmpty())
                classesKey.setValue("." + extension + "/ContentType", mimeType);
            if (!perceivedType.isEmpty())
                classesKey.setValue("." + extension + "/PerceivedType", perceivedType);

            classesKey.setValue("CEGUI.CEED." + extension + "/.", "CEGUI Project file");
            classesKey.setValue("CEGUI.CEED." + extension + "/shell/Open/Command/.", openCommand);
            if (iconIndex >= 0)
                classesKey.setValue("CEGUI.CEED." + extension + "/DefaultIcon/.", fileIconPath);
        }
    }
#endif
}

bool isInternetConnected()
{
#ifdef Q_OS_WIN
    INetworkListManager* netMgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL, IID_INetworkListManager, reinterpret_cast<LPVOID*>(&netMgr));
    if (SUCCEEDED(hr) && netMgr)
    {
        NLM_CONNECTIVITY connect;
        hr = netMgr->GetConnectivity(&connect);
        netMgr->Release();
        if (hr == S_OK)
            return connect & (NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET);
    }
#endif

    // Fallback to Qt could be here but there is no good way found. Please don't hesitate to implement it.
    // ...

    // Consider an internet is available by default
    return true;
}

bool unzip(const QString& srcFilePath, const QString& dstFolderPath)
{
    struct ZipReaderRAII final
    {
        void* handle = nullptr;

        ZipReaderRAII() { mz_zip_reader_create(&handle); }
        ~ZipReaderRAII() { mz_zip_reader_delete(&handle); } // close happens inside

        operator void*() { return handle; }
    };

    ZipReaderRAII zip;

    //mz_zip_reader_set_entry_cb(zip, userdata, minizip_extract_entry_cb);
    //mz_zip_reader_set_progress_cb(zip, userdata, minizip_extract_progress_cb);
    //mz_zip_reader_set_overwrite_cb(zip, userdata, minizip_extract_overwrite_cb);

    auto err = mz_zip_reader_open_file(zip, srcFilePath.toLocal8Bit().constData());
    if (err != MZ_OK) return false;

    err = mz_zip_reader_save_all(zip, dstFolderPath.toLocal8Bit().constData());
    return (err == MZ_OK || err == MZ_END_OF_LIST);
}

}

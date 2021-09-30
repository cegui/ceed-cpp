#-------------------------------------------------
#
# Project created by QtCreator 2019-03-08T09:49:42
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ceed
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(3rdParty/QtnProperty/QtnProperty/QtnProperty.pri)

CONFIG += c++11

SOURCES += \
    src/cegui/CEGUIUtils.cpp \
    src/cegui/QtnPropertyColourRect.cpp \
    src/editors/anim/AnimationCodeMode.cpp \
    src/editors/anim/AnimationEditor.cpp \
    src/editors/anim/AnimationUndoCommands.cpp \
    src/editors/anim/AnimationVisualMode.cpp \
    src/editors/looknfeel/LookNFeelCodeMode.cpp \
    src/editors/looknfeel/LookNFeelEditor.cpp \
    src/editors/looknfeel/LookNFeelPreviewMode.cpp \
    src/editors/looknfeel/LookNFeelScene.cpp \
    src/editors/looknfeel/LookNFeelUndoCommands.cpp \
    src/editors/looknfeel/LookNFeelVisualMode.cpp \
    src/main.cpp \
    src/ui/CEGUIDebugInfo.cpp \
    src/ui/GuideLine.cpp \
    src/ui/MainWindow.cpp \
    src/ui/NumericValueItem.cpp \
    src/ui/ProjectManager.cpp \
    src/ui/CEGUIWidget.cpp \
    src/ui/CEGUIGraphicsView.cpp \
    src/ui/CEGUIGraphicsScene.cpp \
    src/ui/dialogs/NewProjectDialog.cpp \
    src/ui/dialogs/ProjectSettingsDialog.cpp \
    src/ui/FileSystemBrowser.cpp \
    src/ui/dialogs/UpdateDialog.cpp \
    src/ui/layout/AnchorCornerHandle.cpp \
    src/ui/layout/AnchorEdgeHandle.cpp \
    src/ui/layout/AnchorPopupMenu.cpp \
    src/ui/layout/LayoutContainerHandle.cpp \
    src/ui/widgets/FileLineEdit.cpp \
    src/ui/dialogs/LicenseDialog.cpp \
    src/ui/dialogs/AboutDialog.cpp \
    src/ui/SettingEntryEditors.cpp \
    src/ui/dialogs/MultiplePossibleFactoriesDialog.cpp \
    src/ui/dialogs/SettingsDialog.cpp \
    src/ui/widgets/ColourButton.cpp \
    src/ui/widgets/PenButton.cpp \
    src/ui/dialogs/PenDialog.cpp \
    src/ui/widgets/KeySequenceButton.cpp \
    src/ui/dialogs/KeySequenceDialog.cpp \
    src/ui/UndoViewer.cpp \
    src/ui/widgets/BitmapEditorWidget.cpp \
    src/cegui/CEGUIManager.cpp \
    src/cegui/CEGUIProject.cpp \
    src/cegui/CEGUIProjectItem.cpp \
    src/cegui/CEGUIManipulator.cpp \
    src/cegui/QtnPropertyUDim.cpp \
    src/cegui/QtnPropertyUVector2.cpp \
    src/cegui/QtnPropertyUVector3.cpp \
    src/cegui/QtnPropertyUSize.cpp \
    src/cegui/QtnPropertyURect.cpp \
    src/cegui/QtnPropertyUBox.cpp \
    src/editors/EditorBase.cpp \
    src/editors/TextEditor.cpp \
    src/editors/NoEditor.cpp \
    src/Application.cpp \
    src/util/RecentlyUsed.cpp \
    src/util/Settings.cpp \
    src/util/SettingsCategory.cpp \
    src/util/SettingsSection.cpp \
    src/util/SettingsEntry.cpp \
    src/util/DismissableMessage.cpp \
    src/editors/BitmapEditor.cpp \
    src/editors/MultiModeEditor.cpp \
    src/editors/CodeEditMode.cpp \
    src/editors/layout/LayoutEditor.cpp \
    src/editors/imageset/ImagesetEditor.cpp \
    src/editors/layout/LayoutCodeMode.cpp \
    src/editors/imageset/ImagesetCodeMode.cpp \
    src/editors/layout/LayoutPreviewerMode.cpp \
    src/editors/imageset/ImagesetVisualMode.cpp \
    src/ui/imageset/ImagesetEditorDockWidget.cpp \
    src/ui/ResizableGraphicsView.cpp \
    src/ui/imageset/ImageLabel.cpp \
    src/ui/imageset/ImageOffsetMark.cpp \
    src/ui/imageset/ImageEntry.cpp \
    src/ui/imageset/ImagesetEntry.cpp \
    src/util/Utils.cpp \
    src/ui/ResizableRectItem.cpp \
    src/ui/ResizingHandle.cpp \
    src/editors/imageset/ImagesetUndoCommands.cpp \
    src/ui/widgets/LineEditWithClearButton.cpp \
    src/editors/layout/LayoutUndoCommands.cpp \
    src/editors/layout/LayoutVisualMode.cpp \
    src/ui/layout/WidgetHierarchyTreeView.cpp \
    src/ui/layout/LayoutManipulator.cpp \
    src/ui/layout/LayoutScene.cpp \
    src/ui/layout/WidgetHierarchyTreeModel.cpp \
    src/ui/layout/WidgetHierarchyDockWidget.cpp \
    src/ui/XMLSyntaxHighlighter.cpp \
    src/ui/layout/WidgetTypeTreeWidget.cpp \
    src/ui/layout/CreateWidgetDockWidget.cpp \
    src/ui/layout/WidgetHierarchyItem.cpp

HEADERS += \
    src/QtStdHash.h \
    src/cegui/CEGUIUtils.h \
    src/cegui/QtnProperty2DRotation.h \
    src/cegui/QtnPropertyColour.h \
    src/cegui/QtnPropertyColourRect.h \
    src/cegui/QtnPropertyGlmVec2.h \
    src/cegui/QtnPropertyGlmVec3.h \
    src/cegui/QtnPropertyRectf.h \
    src/cegui/QtnPropertySizef.h \
    src/editors/anim/AnimationCodeMode.h \
    src/editors/anim/AnimationEditor.h \
    src/editors/anim/AnimationUndoCommands.h \
    src/editors/anim/AnimationVisualMode.h \
    src/editors/looknfeel/LookNFeelCodeMode.h \
    src/editors/looknfeel/LookNFeelEditor.h \
    src/editors/looknfeel/LookNFeelPreviewMode.h \
    src/editors/looknfeel/LookNFeelScene.h \
    src/editors/looknfeel/LookNFeelUndoCommands.h \
    src/editors/looknfeel/LookNFeelVisualMode.h \
    src/ui/CEGUIDebugInfo.h \
    src/ui/GuideLine.h \
    src/ui/NumericValueItem.h \
    src/ui/ProjectManager.h \
    src/ui/MainWindow.h \
    src/ui/CEGUIWidget.h \
    src/ui/CEGUIGraphicsView.h \
    src/ui/CEGUIGraphicsScene.h \
    src/cegui/CEGUIManager.h \
    src/cegui/CEGUIProject.h \
    src/cegui/CEGUIProjectItem.h \
    src/cegui/CEGUIManipulator.h \
    src/cegui/QtnPropertyUDim.h \
    src/cegui/QtnPropertyUVector2.h \
    src/cegui/QtnPropertyUVector3.h \
    src/cegui/QtnPropertyUSize.h \
    src/cegui/QtnPropertyURect.h \
    src/cegui/QtnPropertyUBox.h \
    src/ui/dialogs/NewProjectDialog.h \
    src/ui/dialogs/ProjectSettingsDialog.h \
    src/ui/FileSystemBrowser.h \
    src/ui/dialogs/UpdateDialog.h \
    src/ui/layout/AnchorCornerHandle.h \
    src/ui/layout/AnchorEdgeHandle.h \
    src/ui/layout/AnchorPopupMenu.h \
    src/ui/layout/LayoutContainerHandle.h \
    src/ui/widgets/FileLineEdit.h \
    src/ui/dialogs/LicenseDialog.h \
    src/ui/dialogs/AboutDialog.h \
    src/editors/EditorBase.h \
    src/editors/TextEditor.h \
    src/editors/NoEditor.h \
    src/ui/dialogs/MultiplePossibleFactoriesDialog.h \
    src/Application.h \
    src/util/RecentlyUsed.h \
    src/ui/dialogs/SettingsDialog.h \
    src/util/Settings.h \
    src/util/SettingsCategory.h \
    src/util/SettingsSection.h \
    src/util/SettingsEntry.h \
    src/ui/SettingEntryEditors.h \
    src/ui/widgets/ColourButton.h \
    src/ui/widgets/PenButton.h \
    src/ui/dialogs/PenDialog.h \
    src/ui/widgets/KeySequenceButton.h \
    src/ui/dialogs/KeySequenceDialog.h \
    src/ui/UndoViewer.h \
    src/util/DismissableMessage.h \
    src/ui/widgets/BitmapEditorWidget.h \
    src/editors/BitmapEditor.h \
    src/editors/MultiModeEditor.h \
    src/editors/CodeEditMode.h \
    src/editors/layout/LayoutEditor.h \
    src/editors/imageset/ImagesetEditor.h \
    src/editors/layout/LayoutCodeMode.h \
    src/editors/imageset/ImagesetCodeMode.h \
    src/editors/layout/LayoutPreviewerMode.h \
    src/editors/imageset/ImagesetVisualMode.h \
    src/ui/imageset/ImagesetEditorDockWidget.h \
    src/ui/ResizableGraphicsView.h \
    src/ui/imageset/ImageLabel.h \
    src/ui/imageset/ImageOffsetMark.h \
    src/ui/imageset/ImageEntry.h \
    src/ui/imageset/ImagesetEntry.h \
    src/util/Utils.h \
    src/ui/ResizableRectItem.h \
    src/ui/ResizingHandle.h \
    src/editors/imageset/ImagesetUndoCommands.h \
    src/ui/widgets/LineEditWithClearButton.h \
    src/editors/layout/LayoutUndoCommands.h \
    src/editors/layout/LayoutVisualMode.h \
    src/ui/layout/WidgetHierarchyTreeView.h \
    src/ui/layout/LayoutManipulator.h \
    src/ui/layout/LayoutScene.h \
    src/ui/layout/WidgetHierarchyTreeModel.h \
    src/ui/layout/WidgetHierarchyDockWidget.h \
    src/ui/XMLSyntaxHighlighter.h \
    src/ui/layout/WidgetTypeTreeWidget.h \
    src/ui/layout/CreateWidgetDockWidget.h \
    src/ui/layout/WidgetHierarchyItem.h

FORMS += \
    ui/CEGUIDebugInfo.ui \
    ui/MainWindow.ui \
    ui/ProjectManager.ui \
    ui/CEGUIWidget.ui \
    ui/dialogs/NewProjectDialog.ui \
    ui/dialogs/ProjectSettingsDialog.ui \
    ui/FileSystemBrowser.ui \
    ui/dialogs/UpdateDialog.ui \
    ui/layout/AnchorPopupMenu.ui \
    ui/widgets/FileLineEdit.ui \
    ui/dialogs/LicenseDialog.ui \
    ui/dialogs/AboutDialog.ui \
    ui/dialogs/MultiplePossibleFactoriesDialog.ui \
    ui/dialogs/PenDialog.ui \
    ui/dialogs/KeySequenceDialog.ui \
    ui/widgets/BitmapEditorWidget.ui \
    ui/imageset/ImagesetEditorDockWidget.ui \
    ui/layout/WidgetHierarchyDockWidget.ui \
    ui/layout/CreateWidgetDockWidget.ui

RESOURCES += \
    data/Resources.qrc

# QtnProperty integration
# FIXME: not needed, all includes in Qtn must start with <QtnProperty/...>, then including from .pri will work.
INCLUDEPATH += $$PWD/3rdParty/QtnProperty/QtnProperty

# CEGUI integration

INCLUDEPATH += $$PWD/3rdParty/CEGUI/include $$PWD/3rdParty/CEGUI/dependencies/include
CONFIG(debug, debug|release) {
    CEGUI_BIN_DIR = $$PWD/3rdParty/CEGUI/bin/debug
    LIBS += -lCEGUIBase-9999_d -lCEGUIOpenGLRenderer-9999_d
} else {
    CEGUI_BIN_DIR = $$PWD/3rdParty/CEGUI/bin/release
    LIBS += -lCEGUIBase-9999 -lCEGUIOpenGLRenderer-9999
}
LIBS += -L"$$PWD/3rdParty/CEGUI/lib" -L"$$CEGUI_BIN_DIR" # Bin is for DLL searching when debugging

# WinAPI: CoCreateInstance, INetworkListManager

win32 {
    LIBS += -lole32
}

# Deployment

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = data/icons/ceed.ico
#MacOS: ICON = myapp.icns

QMAKE_TARGET_COMPANY = "CEGUI team, Vladimir 'Niello' Orlov"
QMAKE_TARGET_COPYRIGHT = "(c) 2019-2021, CEGUI team, Vladimir 'Niello' Orlov"
QMAKE_TARGET_PRODUCT = "CEED"
QMAKE_TARGET_DESCRIPTION = "CEGUI unified editor (CEED)"
VERSION = 1.0.0

DESTDIR = $$OUT_PWD/bin

CONFIG(debug, debug|release) {
    mac: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,,_d)
}

# --no-plugins + static linking?
# https://doc.qt.io/qt-5/deployment-plugins.html
# https://doc.qt.io/qt-5/plugins-howto.html#static-plugins
win32: DEPLOY_COMMAND = $$shell_quote($$shell_path($$[QT_INSTALL_BINS]/windeployqt)) --no-quick-import --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-angle --no-webkit2 --no-opengl-sw --no-svg
macx: DEPLOY_COMMAND = $$shell_quote($$shell_path($$[QT_INSTALL_BINS]/macdeployqt))

DEPLOY_TARGET = $$shell_quote($$shell_path($$DESTDIR))

!isEmpty(DEPLOY_COMMAND): QMAKE_POST_LINK += $$DEPLOY_COMMAND $$DEPLOY_TARGET$$escape_expand(\n\t)

fonts.path = $$DESTDIR/data/fonts
fonts.files = $$PWD/data/fonts/*
INSTALLS += fonts

doc.path = $$DESTDIR/doc
doc.files = $$PWD/data/doc/*.pdf
INSTALLS += doc

images.path = $$DESTDIR/data/images
images.files = $$PWD/data/icons/ceed.png
INSTALLS += images

win32 {
    cegui_dlls.path = $$DESTDIR
    cegui_dlls.files = $$CEGUI_BIN_DIR/*.dll
    ssl_dlls.path = $$DESTDIR
    ssl_dlls.files = $$PWD/3rdParty/OpenSSL/*.dll
    INSTALLS += cegui_dlls ssl_dlls
    win32-msvc* {
        QMAKE_POST_LINK += $$quote(nmake install$$escape_expand(\n\t))
    } else:win32-g++ {
        QMAKE_POST_LINK += $$quote(mingw32-make install$$escape_expand(\n\t))
    }
}


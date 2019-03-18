#-------------------------------------------------
#
# Project created by QtCreator 2019-03-08T09:49:42
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Editor
TEMPLATE = app

RC_ICONS = data/icons/ceed.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ProjectManager.cpp \
    src/proj/CEGUIProjectManager.cpp \
    src/ui/CEGUIWidget.cpp \
    src/ui/CEGUIGraphicsView.cpp \
    src/ui/CEGUIGraphicsScene.cpp \
    src/proj/CEGUIProject.cpp \
    src/proj/CEGUIProjectItem.cpp \
    src/ui/dialogs/NewProjectDialog.cpp \
    src/ui/dialogs/ProjectSettingsDialog.cpp \
    src/ui/FileSystemBrowser.cpp \
    src/ui/widgets/FileLineEdit.cpp \
    src/ui/dialogs/LicenseDialog.cpp \
    src/ui/dialogs/AboutDialog.cpp \
    src/editors/EditorBase.cpp \
    src/editors/TextEditor.cpp \
    src/editors/NoEditor.cpp \
    src/ui/dialogs/MultiplePossibleFactoriesDialog.cpp \
    src/Application.cpp \
    src/util/RecentlyUsed.cpp \
    src/ui/dialogs/SettingsDialog.cpp \
    src/util/Settings.cpp \
    src/util/SettingsCategory.cpp \
    src/util/SettingsSection.cpp \
    src/util/SettingsEntry.cpp \
    src/ui/SettingEntryEditors.cpp \
    src/ui/widgets/ColourButton.cpp

HEADERS += \
    src/ui/ProjectManager.h \
    src/ui/MainWindow.h \
    src/proj/CEGUIProjectManager.h \
    src/ui/CEGUIWidget.h \
    src/ui/CEGUIGraphicsView.h \
    src/ui/CEGUIGraphicsScene.h \
    src/proj/CEGUIProject.h \
    src/proj/CEGUIProjectItem.h \
    src/ui/dialogs/NewProjectDialog.h \
    src/ui/dialogs/ProjectSettingsDialog.h \
    src/ui/FileSystemBrowser.h \
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
    src/ui/widgets/ColourButton.h

FORMS += \
    ui/MainWindow.ui \
    ui/ProjectManager.ui \
    ui/CEGUIWidget.ui \
    ui/dialogs/NewProjectDialog.ui \
    ui/dialogs/ProjectSettingsDialog.ui \
    ui/FileSystemBrowser.ui \
    ui/widgets/FileLineEdit.ui \
    ui/dialogs/LicenseDialog.ui \
    ui/dialogs/AboutDialog.ui \
    ui/dialogs/MultiplePossibleFactoriesDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    data/Resources.qrc

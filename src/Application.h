#ifndef APPLICATION_H
#define APPLICATION_H

#include <qapplication.h>
#include "ui/MainWindow.h"
#include <map>

// The central application class

class Settings;
class SettingsSection;
class QNetworkAccessManager;
class QCommandLineParser;

class Application : public QApplication
{
    Q_OBJECT

public:

    Application(int& argc, char** argv);
    virtual ~Application() override;

    MainWindow* getMainWindow() { return _mainWindow; }
    Settings* getSettings() const { return _settings; }
    QNetworkAccessManager* getNetworkManager() const { return _network; }

    SettingsSection* getOrCreateShortcutSettingsSection(const QString& groupId, const QString& label);
    QAction* registerAction(const QString& groupId, const QString& id, const QString& label,
                            const QString& help, QIcon icon = QIcon(), QKeySequence defaultShortcut = QKeySequence(), bool checkable = false);
    QAction* getAction(const QString& fullId) const;
    void setActionsEnabled(const QString& groupId, bool enabled);

    QString getDocumentationPath() const;
    QString getUpdatePath() const;

    void checkForUpdates(bool manual, const std::function<void()>& cb = nullptr);

private:

    void createSettingsEntries();
    void onUpdateError(const QUrl& url, const QString& errorString);
    void checkUpdateResults();

    QCommandLineParser* _cmdLine = nullptr;
    MainWindow* _mainWindow = nullptr;
    Settings* _settings = nullptr;
    QNetworkAccessManager* _network = nullptr;
    std::map<QString, QAction*> _globalActions;
};

#endif // APPLICATION_H

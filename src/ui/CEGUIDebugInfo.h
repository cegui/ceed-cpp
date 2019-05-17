#ifndef CEGUIDEBUGINFO_H
#define CEGUIDEBUGINFO_H

#include <QDialog>
#include <CEGUI/Logger.h>

// A debugging/info widget about the embedded CEGUI instance

namespace Ui {
class CEGUIDebugInfo;
}

class QTextBrowser;

class CEGUIDebugInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CEGUIDebugInfo(QWidget *parent = nullptr);
    ~CEGUIDebugInfo();

    void show();
    void logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level);

private:
    Ui::CEGUIDebugInfo *ui;
    QTextBrowser* logView = nullptr;

    QStringList logMessages;

    int errors = 0;
    int warnings = 0;
    int other = 0;
    int messageLimit = 1000;
};

#endif // CEGUIDEBUGINFO_H

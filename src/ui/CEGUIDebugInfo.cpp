#include "src/ui/CEGUIDebugInfo.h"
#include "ui_CEGUIDebugInfo.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qtextbrowser.h"

CEGUIDebugInfo::CEGUIDebugInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CEGUIDebugInfo)
{
    ui->setupUi(this);

    setVisible(false);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    auto logViewAreaLayout = new QVBoxLayout();

    logView = new QTextBrowser();
    logView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    logView->setOpenExternalLinks(true);
    logViewAreaLayout->addWidget(logView);

    ui->logViewArea->setLayout(logViewAreaLayout);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    messageLimit = settings->getEntryValue("global/cegui_debug_info/log_limit").toInt();
}

CEGUIDebugInfo::~CEGUIDebugInfo()
{
    delete ui;
}

void CEGUIDebugInfo::show()
{
    logView->setHtml("\
<html>\
<body>\
<style type=\"text/css\">\
font-size: 10px;\
</style>\
<table>\
<thead>\
<th></th><th>Message</th>\
</thead>\
<tbody>"
            + logMessages.join("\n") +
"</tbody>\
</table>\
</html>");

/*
//???need at all? it is a per-view data and it has no much significance in an editor,
//especially when continuous rendering is disabled
        self.updateFPSTick()
*/

    QDialog::show();
}

/*
    def updateFPSTick(self):
        if not self.isVisible():
            return

        lastRenderDelta = self.containerWidget.ceguiInstance.lastRenderTimeDelta
        if lastRenderDelta <= 0:
            lastRenderDelta = 1

        self.currentFPSBox.setText("%0.6f" % (1.0 / lastRenderDelta))

        QtCore.QTimer.singleShot(500, self.updateFPSTick)
*/

void CEGUIDebugInfo::logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level)
{
    QString levelStr, bgColorStr;
    if (level == CEGUI::LoggingLevel::Error)
    {
        ++errors;
        ui->errorsBox->setText(QString::number(errors));

        levelStr = "E";
        bgColorStr = "#ff5f5f";
    }
    else if (level == CEGUI::LoggingLevel::Warning)
    {
        ++warnings;
        ui->warningsBox->setText(QString::number(warnings));

        levelStr = "W";
        bgColorStr = "#fff76f";
    }
    else
    {
        ++other;
        ui->othersBox->setText(QString::number(other));

        levelStr = " ";
        bgColorStr = "transparent";
    }

    QString qmessage = CEGUIUtils::stringToQString(message);

    // Log info using the logging message, allows debug outputs without GUI
    const QString debugMsg("[CEGUI] %1");
    qDebug(debugMsg.arg(qmessage).toLocal8Bit().data());

    // Remove old messages
    while (logMessages.size() >= messageLimit)
        logMessages.pop_front();

    qmessage = QString("<tr><td style=\"background: %1\">%2</td><td>%3</td></tr>\n").arg(bgColorStr, levelStr, qmessage);

    logMessages.append(qmessage);
}

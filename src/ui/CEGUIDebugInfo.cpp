#include "src/ui/CEGUIDebugInfo.h"
#include "ui_CEGUIDebugInfo.h"

CEGUIDebugInfo::CEGUIDebugInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CEGUIDebugInfo)
{
    ui->setupUi(this);

    setVisible(false);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    /*
        self.containerWidget = containerWidget
        # update FPS and render time very second
        self.boxUpdateInterval = 1

        self.errors = 0
        self.warnings = 0
        self.others = 0

        self.logViewAreaLayout = QtGui.QVBoxLayout()

        self.logView = QtWebKit.QWebView()
        self.logView.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Ignored)
        self.logViewAreaLayout.addWidget(self.logView)

        self.logViewArea.setLayout(self.logViewAreaLayout)

        self.logMessagesLimit = settings.getEntry("global/cegui_debug_info/log_limit").value
        self.logMessages = collections.deque([])

        self.containerWidget.ceguiInstance.logger.registerSubscriber(self.logEvent)
    */
}

CEGUIDebugInfo::~CEGUIDebugInfo()
{
    delete ui;
}

/*

class LogMessageWrapper(object):
    def __init__(self, message, level):
        self.message = message
        self.level = level

    def asTableRow(self):
        bgColour = "transparent"

        if self.level == PyCEGUI.LoggingLevel.Errors:
            stringLevel = "E"
            bgColour = "#ff5f5f"

        elif self.level == PyCEGUI.LoggingLevel.Warnings:
            stringLevel = "W"
            bgColour = "#fff76f"

        else:
            stringLevel = " "
            bgColour = "transparent"

        return "<tr><td style=\"background: %s\">%s</td><td>%s</td></tr>\n" % (bgColour, stringLevel, self.message)

class DebugInfo(QtGui.QDialog):
    """"""

    # This will allow us to view logs in Qt in the future


    def logEvent(self, message, level):
        if level == PyCEGUI.LoggingLevel.Errors:
            self.errors += 1
            self.errorsBox.setText(str(self.errors))

        elif level == PyCEGUI.LoggingLevel.Warnings:
            self.warnings += 1
            self.warningsBox.setText(str(self.warnings))

        else:
            self.others += 1
            self.othersBox.setText(str(self.others))

        # log info using the logging message, allows debug outputs without GUI
        logging.info("CEGUI message: %s", message)

        # remove old messages
        while len(self.logMessages) >= self.logMessagesLimit:
            self.logMessages.popleft()

        self.logMessages.append(LogMessageWrapper(message, level))

    def show(self):
        htmlLog = "\n".join([msg.asTableRow() for msg in self.logMessages])

        self.logView.setHtml("""
<html>
<body>
<style type="text/css">
font-size: 10px;
</style>
<table>
<thead>
<th></th><th>Message</th>
</thead>
<tbody>
""" + htmlLog + """
</tbody>
</table>
</html>""")

        super(DebugInfo, self).show()
        self.updateFPSTick()

    def updateFPSTick(self):
        if not self.isVisible():
            return

        lastRenderDelta = self.containerWidget.ceguiInstance.lastRenderTimeDelta
        if lastRenderDelta <= 0:
            lastRenderDelta = 1

        self.currentFPSBox.setText("%0.6f" % (1.0 / lastRenderDelta))

        QtCore.QTimer.singleShot(500, self.updateFPSTick)
*/

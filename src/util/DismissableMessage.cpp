#include "src/util/DismissableMessage.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "qcryptographichash.h"
#include "qsettings.h"
#include "qmessagebox.h"

/*
    Pops up a modal warning dialog, blocks until user dismisses

    app - ceed.Application, we get QSettings from there
    parentWidget - parent Qt widget of the spawned dialog
    title - window title of the spawned dialog
    message - message text, plain text
    token - this is used to remember whether user dismissed to never show again

    token is generated automatically from title and message if None is passed.
    For messages containing diagnostic info this may not be appropriate. The
    info inside will change, thus changing the token and user will see the
    same warning again. Passing a proper descriptive token is advised.
*/
void DismissableMessage::warning(QWidget* parent, const QString& title, const QString& message, QString token)
{
    if (token.isEmpty())
    {
        QCryptographicHash sha1(QCryptographicHash::Sha1);
        sha1.addData((title + message).toUtf8());
        token = sha1.result();
    }

    QString key = "messages/never_show_warning_" + token;

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    if (settings->value(key, false).toBool())
    {
        // User chose to never show this type of dialog again
        return;
    }

    QMessageBox dialog(parent);
    dialog.setIcon(QMessageBox::Warning);
    dialog.setWindowTitle(title);
    dialog.setText(message);
    dialog.addButton("Ok", QMessageBox::AcceptRole);
    dialog.addButton("Never show again", QMessageBox::RejectRole);

    if (dialog.exec() == QMessageBox::RejectRole)
        settings->setValue(key, true);
}

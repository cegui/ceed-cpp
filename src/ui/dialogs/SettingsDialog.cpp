#include "src/ui/dialogs/SettingsDialog.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qtabwidget.h"
#include "qdialogbuttonbox.h"
#include "qabstractbutton.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
/*
    self.settings = settings

    # sort everything so that it comes at the right spot when iterating
    self.settings.sort()
*/

    setWindowTitle("CEGUI Unified Editor settings");
    setWindowModality(Qt::ApplicationModal);

    // The basic UI
    auto layout = new QVBoxLayout();

    auto label = new QLabel("Provides all persistent settings of CEGUI Unified Editor (CEED),"
                            " everything is divided into categories (see the tab buttons).");
    label->setWordWrap(true);
    layout->addWidget(label);

    auto tabs = new QTabWidget();
    tabs->setTabPosition(QTabWidget::North);
    layout->addWidget(tabs);

    setLayout(layout);
/*
    # for each category, add a tab
    addTab = self.tabs.addTab
    for category in self.settings.categories:
        addTab(interface_types.InterfaceCategory(category, self.tabs), category.label)
*/

    // Apply, cancel etc...
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::onButtonBoxClicked);
    layout->addWidget(buttonBox);
}

/*
    def checkIfRestartRequired(self):
        if self.settings.changesRequireRestart:
            # Restart required
            self.needRestart = QtGui.QMessageBox()
            self.needRestart.setWindowTitle("CEED")
            self.needRestart.setIcon(QtGui.QMessageBox.Warning)
            self.needRestart.setText("Restart is required for the changes to take effect.")
            self.needRestart.exec_()

            # FIXME: Kill the app; then restart it.
            #
            # - This may or may not be the way to get rid of this, but for the
            #   moment we use it as a "the user has been notified they must restart
            #   the application" flag.
            self.settings.changesRequireRestart = False
        return
*/

void SettingsDialog::onButtonBoxClicked(QAbstractButton* button)
{
    switch (buttonBox->buttonRole(button))
    {
        //???enable this button only if changes exist?
        case QDialogButtonBox::ApplyRole:
        {
            /*
            self.settings.applyChanges()

            # Check if restart required
            self.checkIfRestartRequired()
            */
            break;
        }
        case QDialogButtonBox::AcceptRole:
        {
            /*
            self.settings.applyChanges()
            */
            accept();

            // Check if restart required
            /*
            self.checkIfRestartRequired()
            */
            break;
        }
        case QDialogButtonBox::RejectRole:
        {
            /*
            self.settings.discardChanges()
            */
            reject();

            // Reset any entries with changes to their stored value
            /*
            for tabIndex in xrange(self.tabs.count()):
                self.tabs.widget(tabIndex).discardChanges()
            */
            break;
        }
        default: return;
    }

    // - Regardless of the action above, all categories are now unchanged.
    /*
        for (tabIndex in xrange(self.tabs.count()))
            self.tabs.widget(tabIndex).markAsUnchanged()
    */

    // FIXME: That is not entirely true; using the 'X' to close the Settings
    // dialog is not handled here; although, this "bug as a feature" allows
    // Settings to be modified, closed, and it will remember (but not apply)
    // the previous changes.
}

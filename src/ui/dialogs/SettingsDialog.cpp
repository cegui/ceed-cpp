#include "src/ui/dialogs/SettingsDialog.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/ui/SettingEntryEditors.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qtabwidget.h"
#include "qdialogbuttonbox.h"
#include "qabstractbutton.h"
#include "qmessagebox.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("CEGUI Unified Editor settings");
    setWindowModality(Qt::ApplicationModal);

    // The basic UI
    auto layout = new QVBoxLayout();

    auto label = new QLabel("Provides all persistent settings of CEGUI Unified Editor (CEED),"
                            " everything is divided into categories (see the tab buttons).");
    label->setWordWrap(true);
    layout->addWidget(label);

    tabs = new QTabWidget();
    tabs->setTabPosition(QTabWidget::North);
    layout->addWidget(tabs);

    setLayout(layout);

    // Add a tab for each settings category, sorted
    auto settings = qobject_cast<Application*>(qApp)->getSettings();
    settings->sort();
    const auto& categories = settings->getCategories();
    for (auto&& category : categories)
    {
        tabs->addTab(new SettingCategoryWidget(*category, tabs), category->getLabel());
    }

    // Apply, cancel etc...
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::onButtonBoxClicked);
    layout->addWidget(buttonBox);
}

void SettingsDialog::onButtonBoxClicked(QAbstractButton* button)
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

    switch (buttonBox->buttonRole(button))
    {
        //???enable this button only if changes exist?
        case QDialogButtonBox::ApplyRole:
        {
            settings->applyChanges();
            checkIfRestartRequired();
            break;
        }
        case QDialogButtonBox::AcceptRole:
        {
            settings->applyChanges();
            accept();
            checkIfRestartRequired();
            break;
        }
        case QDialogButtonBox::RejectRole:
        {
            settings->discardChanges();
            reject();

            // Reset any entries with changes to their stored value
            for (int i = 0; i < tabs->count(); ++i)
            {
                auto tab = static_cast<SettingCategoryWidget*>(tabs->widget(i));
                tab->updateValuesInUI();
            }

            break;
        }
        default: return; // Unknown button must not be processed
    }

    // Update modified marks (should be all unset now)
    for (int i = 0; i < tabs->count(); ++i)
    {
        auto tab = static_cast<SettingCategoryWidget*>(tabs->widget(i));
        tab->updateUIOnChange(true);
    }

    // FIXME: That is not entirely true; using the 'X' to close the Settings
    // dialog is not handled here; although, this "bug as a feature" allows
    // Settings to be modified, closed, and it will remember (but not apply)
    // the previous changes.
}

void SettingsDialog::checkIfRestartRequired()
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    if (!settings->changesRequireRestart())
        return;

    QMessageBox mbox;
    mbox.setWindowTitle("CEED");
    mbox.setIcon(QMessageBox::Warning);
    mbox.setText("Restart is required for the changes to take effect.");
    mbox.exec();

    // FIXME: Kill the app; then restart it.
    //
    // - This may or may not be the way to get rid of this, but for the
    //   moment we use it as a "the user has been notified they must restart
    //   the application" flag.
    settings->markRequiresRestart(false);
}

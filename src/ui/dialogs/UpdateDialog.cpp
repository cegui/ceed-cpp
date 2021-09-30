#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"
#include "src/Application.h"
#include <qversionnumber.h>
#include <qjsonobject.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qdesktopservices.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>

UpdateDialog::UpdateDialog(const QVersionNumber& currentVersion, const QVersionNumber& newVersion,
                           const QJsonObject& releaseInfo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);

    //setWindowFlags((windowFlags() | Qt::MSWindowsFixedSizeDialogHint) & ~Qt::WindowContextHelpButtonHint);

    ui->lblVersions->setText(currentVersion.toString() + " -> " + newVersion.toString());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    ui->txtReleaseNotes->setMarkdown(releaseInfo.value("body").toString());
#else
    ui->txtReleaseNotes->setText(releaseInfo.value("body").toString());
#endif

    ui->progressBar->setVisible(false);
    ui->progressBar->setMaximum(10000);

    _releaseWebPage = releaseInfo.value("html_url").toString();

    //_releaseAsset

    //if (_releaseAsset.isEmpty()) close dialog, show error! Do that outside here?

    /*
        //!!!show download size in MB! ui->downloadSize->setText(QFormatStr("%1 MB").arg(double(m_Size) / 1048576.0, 0, 'f', 2));
        //!!!check if this version was already downloaded! reload if corrupted?

        _mainWindow->setStatusMessage("Downloading version " + latestVersionStr);
        //!!!show progress!

#if QT_POINTER_SIZE == 4
        const QString arch = "x86";
#else
        const QString arch = "x64";
#endif
*/

    adjustSize();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::keyPressEvent(QKeyEvent* event)
{
    // Prevent closing by Esc
    if (event->key() != Qt::Key_Escape)
        UpdateDialog::keyPressEvent(event);
}

void UpdateDialog::closeEvent(QCloseEvent* event)
{
    if (ui->progressBar->isVisible())
        event->ignore();
    else
        QDialog::closeEvent(event);
}

void UpdateDialog::on_btnUpdate_clicked()
{
    ui->btnUpdate->setEnabled(false);

    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);

    /*
    ui->progressText->setVisible(true);
    ui->progressText->setText(tr("Preparing Download"));

    QElapsedTimer downloadTimer;
    downloadTimer.start();
    */

    QNetworkReply* assetReply = qobject_cast<Application*>(qApp)->getNetworkManager()->get(QNetworkRequest(_releaseAsset));

    /*
    QObject::connect(assetReply, &QNetworkReply::downloadProgress, [this](qint64 recvd, qint64 total)
    {
        UpdateTransferProgress(recvd, total, m_DownloadTimer, ui->progressBar, ui->progressText,
            tr("Downloading update..."));
    });

    QObject::connect(assetReply, &QNetworkReply::errorOccurred, [this, assetReply](QNetworkReply::NetworkError)
    {
        ui->progressBar->setValue(0);
        ui->progressText->setText(tr("Network error:\n%1").arg(assetReply->errorString()));
        ui->update->setEnabled(true);
        ui->close->setEnabled(true);
        ui->update->setText(tr("Retry Update"));
    });
    */
    QObject::connect(assetReply, &QNetworkReply::finished, [this, assetReply]()
    {
        auto response = QMessageBox::question(this, tr("Confirm closing"),
                                              tr("Application will be closed and all unsaved work will be lost.\nContinue?"),
                                              QMessageBox::Yes | QMessageBox::No);
        if (response != QMessageBox::Yes) return;

        // Run external tool and close us
    }
}

void UpdateDialog::on_btnWeb_clicked()
{
    QDesktopServices::openUrl(_releaseWebPage);
}


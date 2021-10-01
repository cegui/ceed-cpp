#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"
#include "src/Application.h"
#include <qversionnumber.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qdesktopservices.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qscreen.h>

UpdateDialog::UpdateDialog(const QVersionNumber& currentVersion, const QVersionNumber& newVersion,
                           const QJsonObject& releaseInfo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    const QString os = "Win";
#else
    const QString os = "";
#endif

#if QT_POINTER_SIZE == 4
    const QString arch = "x86";
#else
    const QString arch = "x64";
#endif

    const QString assetName = QString("CEED-v%1-%2-%3.zip").arg(newVersion.toString(), os, arch);
    const QString assetNameNoCase = assetName.toLower();
    const auto assets = releaseInfo.value("assets").toArray();
    for (const QJsonValue& assetDesc : assets)
    {
        const auto assetObject = assetDesc.toObject();
        if (//assetObject.value("content_type").toString() == "application/zip" &&
            //assetObject.value("state").toString() == "uploaded" &&
            assetObject.value("name").toString().trimmed().toLower() == assetNameNoCase)
        {
            _releaseAsset = assetObject.value("browser_download_url").toString();
            _releaseAssetSize = static_cast<int64_t>(assetObject.value("size").toDouble());
            break;
        }
    }

    QString versionStr = tr("<b>%1 <img width=\"12\" height=\"12\" src=\"://icons/layout_editing/move_forward_in_parent_list.png\"/> %2</b>")
            .arg(currentVersion.toString(), newVersion.toString());

    if (_releaseAsset.isEmpty())
    {
        ui->btnUpdate->setEnabled(false);
        ui->btnUpdate->setText(tr("<No package>"));
        ui->btnUpdate->setToolTip(tr("No downloadable package was detected for your OS,\nplease visit a release web page and download manually"));
    }
    else
    {
        versionStr += tr(" (%3 MB)").arg(_releaseAssetSize / 1048576.0, 0, 'f', 1);
    }

    _releaseWebPage = releaseInfo.value("html_url").toString();
    const QDateTime releaseDate = QDateTime::fromString(releaseInfo.value("published_at").toString(), Qt::ISODate);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (auto myScreen = screen())
    {
        const auto screenRect = myScreen->availableGeometry();
        const auto center = screenRect.center();
        const QSize size(std::min(screenRect.width() / 2, 640), std::min(screenRect.height() / 2, 480));
        setGeometry(center.x() - size.width() / 2, center.y() - size.height() / 2, size.width(), size.height());
    }

    const auto releaseFullName = releaseInfo.value("name").toString();
    if (!releaseFullName.isEmpty())
        setWindowTitle("Update available - " + releaseFullName);

    ui->lblVersions->setText(versionStr);
    ui->lblReleaseDate->setText("Published at " + releaseDate.toString(Qt::SystemLocaleLongDate) + " GMT");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    ui->txtReleaseNotes->setMarkdown(releaseInfo.value("body").toString());
#else
    ui->txtReleaseNotes->setText(releaseInfo.value("body").toString());
#endif

    ui->lblStatus->setVisible(false);
    ui->progressBar->setVisible(false);
    ui->progressBar->setMaximum(10000);
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
    if (_blocked)
        event->ignore();
    else
        QDialog::closeEvent(event);
}

void UpdateDialog::on_btnUpdate_clicked()
{
    // FIXME QTBUG: Qt 5.15.2 freezes in QMessageBox::question below
    //setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    _blocked = true;

    ui->btnUpdate->setEnabled(false);

    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    ui->lblStatus->setVisible(true);
    ui->lblStatus->setText(tr("Preparing download..."));

    //QElapsedTimer downloadTimer;
    //downloadTimer.start();

    QNetworkReply* assetReply = qobject_cast<Application*>(qApp)->getNetworkManager()->get(QNetworkRequest(_releaseAsset));

    QObject::connect(assetReply, &QNetworkReply::downloadProgress, [this](qint64 received, qint64 total)
    {
        //UpdateTransferProgress(recvd, total, m_DownloadTimer, ui->progressBar, ui->progressText, tr("Downloading update..."));
    });

    QObject::connect(assetReply, &QNetworkReply::errorOccurred, [this, assetReply](QNetworkReply::NetworkError)
    {
        ui->lblStatus->setText(tr("Network error: %1").arg(assetReply->errorString()));
    });

    QObject::connect(assetReply, &QNetworkReply::finished, [this, assetReply]()
    {
        //setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

        _blocked = false;

        if (assetReply->error() != QNetworkReply::NoError)
        {
            ui->progressBar->setVisible(false);
            ui->btnUpdate->setEnabled(true);
            ui->btnUpdate->setText(tr("Retry"));
            return;
        }

        auto response = QMessageBox::question(this, tr("Confirm closing"),
                                              tr("Application will be closed and all unsaved work will be lost.\nContinue?"),
                                              QMessageBox::Yes | QMessageBox::No);
        if (response != QMessageBox::Yes) return;

        // Run external tool and close us
    });
}

void UpdateDialog::on_btnWeb_clicked()
{
    QDesktopServices::openUrl(_releaseWebPage);
}


#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <qurl.h>
#include <qelapsedtimer.h>
#include <qversionnumber.h>

class QJsonObject;

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(const QVersionNumber& currentVersion, const QVersionNumber& newVersion,
                          const QJsonObject& releaseInfo, QWidget *parent = nullptr);
    virtual ~UpdateDialog() override;

private slots:
    void on_btnUpdate_clicked();

    void on_btnWeb_clicked();

private:

    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;

    void downloadUpdate();
    void installUpdate();

    Ui::UpdateDialog *ui;

    QElapsedTimer _downloadTimer;

    QVersionNumber _releaseVersion;
    QUrl _releaseWebPage;
    QUrl _releaseAsset;
    int64_t _releaseAssetSize = 0.0;
    bool _blocked = false;
};

#endif // UPDATEDIALOG_H

#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <qurl.h>

class QVersionNumber;
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

    Ui::UpdateDialog *ui;

    QUrl _releaseWebPage;
    QUrl _releaseAsset;
};

#endif // UPDATEDIALOG_H

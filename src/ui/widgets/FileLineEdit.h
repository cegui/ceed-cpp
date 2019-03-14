#ifndef FILELINEEDIT_H
#define FILELINEEDIT_H

#include <QWidget>
#include <functional>

namespace Ui {
class FileLineEdit;
}

class QLineEdit;

class FileLineEdit : public QWidget
{
    Q_OBJECT

public:

    enum class Mode
    {
        NewFile,
        ExistingFile,
        ExistingDirectory
    };

    explicit FileLineEdit(QWidget *parent = nullptr);
    ~FileLineEdit();

    void setText(const QString& text);
    QString text() const;

    void setMode(Mode newMode) { mode = newMode; }
    void setInitialDirectoryDelegate(std::function<QString()> delegate) { getInitialDirectory = delegate; }
    void setFilter(const QString& newFilter) { filter = newFilter; }

private slots:

    void on_browseButton_pressed();

private:

    Ui::FileLineEdit *ui;
    QLineEdit* lineEdit = nullptr;

    Mode mode = Mode::ExistingFile;
    QString filter;
    std::function<QString()> getInitialDirectory;
};

#endif // FILELINEEDIT_H

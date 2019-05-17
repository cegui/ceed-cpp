#ifndef CEGUIDEBUGINFO_H
#define CEGUIDEBUGINFO_H

#include <QDialog>

// A debugging/info widget about the embedded CEGUI instance

namespace Ui {
class CEGUIDebugInfo;
}

class CEGUIDebugInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CEGUIDebugInfo(QWidget *parent = nullptr);
    ~CEGUIDebugInfo();

private:
    Ui::CEGUIDebugInfo *ui;
};

#endif // CEGUIDEBUGINFO_H

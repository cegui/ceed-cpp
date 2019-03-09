#ifndef CEGUIGRAPHICSVIEW_H
#define CEGUIGRAPHICSVIEW_H

#include <QGraphicsView>

class CEGUIGraphicsView final : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CEGUIGraphicsView(QWidget *parent = nullptr);
    ~CEGUIGraphicsView();

    void injectInput(bool inject);

private:

    bool _injectInput = false;
};

#endif // CEGUIGRAPHICSVIEW_H

#include "src/ui/widgets/LineEditWithClearButton.h"
#include "qtoolbutton.h"
#include "qaction.h"
#include "qstyle.h"

LineEditWithClearButton::LineEditWithClearButton(QWidget* parent)
    : QLineEdit(parent)
{
    auto icon = QPixmap(":/icons/widgets/edit-clear.png");

    button = new QToolButton(this);
    button->setIcon(icon);
    button->setIconSize(icon.size());
    button->setCursor(Qt::ArrowCursor);
    button->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    button->hide();

    connect(button, &QToolButton::clicked, this, &LineEditWithClearButton::clear);
    connect(this, &LineEditWithClearButton::textChanged, [this](const QString& text)
    {
        button->setVisible(!text.isEmpty());
    });

    auto clearAction = new QAction(this);
    clearAction->setShortcut(QKeySequence("Esc"));
    clearAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
    connect(clearAction, &QAction::triggered, this, &LineEditWithClearButton::clear);
    addAction(clearAction);

    auto frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; }").arg(button->sizeHint().width() + frameWidth + 1));

    auto minSizeHint = minimumSizeHint();
    setMinimumSize(std::max(minSizeHint.width(), button->sizeHint().width() + frameWidth * 2 + 2),
                   std::max(minSizeHint.height(), button->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEditWithClearButton::resizeEvent(QResizeEvent* /*event*/)
{
    auto sz = button->sizeHint();
    auto frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    button->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height()) / 2);
}

#include "src/ui/widgets/PenButton.h"
#include "src/ui/dialogs/PenDialog.h"

PenButton::PenButton(QWidget* parent)
    : QPushButton(parent)
{
    setAutoFillBackground(true);
    setFlat(true);
    setPen(QPen());

    connect(this, &QPushButton::clicked, this, &PenButton::onClick);
}

void PenButton::setPen(const QPen& newPen)
{
    _pen = newPen;

    QString lineStyleStr;
    if (newPen.style() == Qt::SolidLine)
        lineStyleStr = "solid";
    else if (newPen.style() == Qt::DashLine)
        lineStyleStr = "dash";
    else if (newPen.style() == Qt::DotLine)
        lineStyleStr = "dot";
    else if (newPen.style() == Qt::DashDotLine)
        lineStyleStr = "dash dot";
    else if (newPen.style() == Qt::DashDotDotLine)
        lineStyleStr = "dash dot dot";
    else if (newPen.style() == Qt::CustomDashLine)
        lineStyleStr = "custom dash";
    else
        lineStyleStr = "<unknown line style>";

/* Was commented in an original CEED:

    capStyleStr = ""
    if pen.capStyle() == QtCore.Qt.FlatCap:
        capStyleStr = "flat"
    elif pen.capStyle() == QtCore.Qt.RoundCap:
        capStyleStr = "round"
    elif pen.capStyle() == QtCore.Qt.SquareCap:
        capStyleStr = "square"
    else:
        raise RuntimeError("Unknown pen cap style!")

    joinStyleStr = ""
    if pen.joinStyle() == QtCore.Qt.MiterJoin:
        joinStyleStr = "miter"
    elif pen.joinStyle() == QtCore.Qt.BevelJoin:
        joinStyleStr = "bevel"
    elif pen.joinStyle() == QtCore.Qt.RoundJoin:
        joinStyleStr = "round"
    elif pen.joinStyle() == QtCore.Qt.SvgMiterJoin:
        joinStyleStr = "svg miter"
    else:
        raise RuntimeError("Unknown pen join style!")
*/

    setText(QString("line style: %1, width: %2").arg(lineStyleStr).arg(newPen.widthF(), 0, 'g', 3));

/* Was commented in an original CEED:
    colour = pen.color()
    self.setStyleSheet("background-color: rgba(%i, %i, %i, %i)" % (colour.red(), colour.green(), colour.blue(), colour.alpha()))
*/

    emit penChanged(_pen);
}

void PenButton::onClick()
{
    PenDialog dialog(this);
    dialog.setPen(_pen);
    if (dialog.exec() == QDialog::Accepted)
        setPen(dialog.getPen());
}

#ifndef CODEEDITMODE_H
#define CODEEDITMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qtextedit.h"

// This is the most used alternative editing mode that allows you to edit raw code.
// Raw code is mostly XML in CEGUI formats but can be anything else in a generic sense.

class CodeEditMode : public QTextEdit, public IEditMode
{
public:

    CodeEditMode();

    virtual void activate() override;
    virtual bool deactivate() override;

    void refreshFromVisual();

protected slots:

    void slot_contentsChange(int position, int charsRemoved, int charsAdded);

protected:

    bool ignoreUndoCommands = false;
    QString lastUndoText;
};

#endif // CODEEDITMODE_H

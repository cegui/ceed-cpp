#ifndef LAYOUTCODEMODE_H
#define LAYOUTCODEMODE_H

#include "src/editors/CodeEditMode.h"

class LayoutEditor;

class LayoutCodeMode : public ViewRestoringCodeEditMode
{
public:

    LayoutCodeMode(LayoutEditor& editor);

    virtual QString getNativeCode() override;
    virtual bool propagateNativeCode(const QString& code) override;
};

#endif // LAYOUTCODEMODE_H

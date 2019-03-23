#ifndef LAYOUTCODEMODE_H
#define LAYOUTCODEMODE_H

#include "src/editors/CodeEditMode.h"

class LayoutCodeMode : public ViewRestoringCodeEditMode
{
public:

    LayoutCodeMode(MultiModeEditor& editor);

    virtual QString getNativeCode() override;
    virtual bool propagateNativeCode(const QString& code) override;
};

#endif // LAYOUTCODEMODE_H

#include "src/editors/MultiModeEditor.h"
#include "src/Application.h"

IEditMode::~IEditMode()
{
    // Functor connections aren't disconnected in a destructor, so we do it manually
    disconnectAllConnections();
}

// Connections must be disconnected when editor becomes inactive, since actions are shared
void IEditMode::disconnectActiveStateConnections()
{
    for (const auto& connection : _activeStateConnections)
        QObject::disconnect(connection);
    _activeStateConnections.clear();
}

void IEditMode::disconnectAllConnections()
{
    disconnectActiveStateConnections();

    for (const auto& connection : _anyStateConnections)
        QObject::disconnect(connection);
    _anyStateConnections.clear();
}

//---------------------------------------------------------------------

MultiModeEditor::MultiModeEditor(/*compatibilityManager, */ const QString& filePath)
    : EditorBase(filePath, true)
{
    assert(undoStack);

    tabs.setTabPosition(QTabWidget::South);
    tabs.setTabShape(QTabWidget::Triangular);
}

void MultiModeEditor::initialize()
{
    EditorBase::initialize();

    // Used as a previous index in slot_currentChanged
    currentTabIndex = tabs.currentIndex();
    assert(currentTabIndex >= 0);
}

void MultiModeEditor::activate(MainWindow& mainWindow)
{
    EditorBase::activate(mainWindow);

    if (auto mode = dynamic_cast<IEditMode*>(tabs.currentWidget()))
        mode->activate(mainWindow);

    connect(&tabs, &QTabWidget::currentChanged, this, &MultiModeEditor::slot_currentChanged);
}

void MultiModeEditor::deactivate(MainWindow& mainWindow)
{
    disconnect(&tabs, &QTabWidget::currentChanged, this, &MultiModeEditor::slot_currentChanged);

    if (auto mode = dynamic_cast<IEditMode*>(tabs.currentWidget()))
        mode->deactivate(mainWindow, true);

    EditorBase::deactivate(mainWindow);
}

void MultiModeEditor::setTabWithoutUndoHistory(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= tabs.count() || tabIndex == tabs.currentIndex()) return;

    ignoreCurrentChangedForUndo = true;
    tabs.setCurrentIndex(tabIndex);
    ignoreCurrentChangedForUndo = false;
}

void MultiModeEditor::slot_currentChanged(int newTabIndex)
{
    if (ignoreCurrentChanged) return;

    auto prevTabWidget = tabs.widget(currentTabIndex);
    auto prevTabMode = dynamic_cast<IEditMode*>(prevTabWidget);
    auto currTabMode = dynamic_cast<IEditMode*>(tabs.widget(newTabIndex));

    auto& mainWindow = *qobject_cast<Application*>(qApp)->getMainWindow();

    if (prevTabMode && !prevTabMode->deactivate(mainWindow, false))
    {
        ignoreCurrentChanged = true;
        tabs.setCurrentWidget(prevTabWidget);
        ignoreCurrentChanged = false;
        return;
    }

    if (currTabMode) currTabMode->activate(mainWindow);

    if (!ignoreCurrentChangedForUndo)
        undoStack->push(new ModeSwitchCommand(*this, currentTabIndex, newTabIndex));

    currentTabIndex = newTabIndex;
}

//---------------------------------------------------------------------

ModeSwitchCommand::ModeSwitchCommand(MultiModeEditor& editor, int oldTabIndex, int newTabIndex)
    : _editor(editor)
    , _oldTabIndex(oldTabIndex)
    , _newTabIndex(newTabIndex)
{
    // We never every merge edit mode changes, no need to define this as refreshText
    setText(QString("Change edit mode to '%1'").arg(editor.getTabText(newTabIndex)));
}

void ModeSwitchCommand::undo()
{
    QUndoCommand::undo();
    _editor.setTabWithoutUndoHistory(_oldTabIndex);
}

void ModeSwitchCommand::redo()
{
    _editor.setTabWithoutUndoHistory(_newTabIndex);
    QUndoCommand::redo();
}

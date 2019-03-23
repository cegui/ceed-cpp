#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

// Usually a singleton that manages all action categories and therefore actions within them

class ActionManager
{
public:

    static ActionManager& Instance()
    {
        static ActionManager mgr;
        return mgr;
    }

    ActionManager();
};

#endif // ACTIONMANAGER_H

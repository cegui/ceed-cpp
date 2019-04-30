CEED C++ (`CE`gui`ED`itor)
===========


CEED C++ is a GPL3-licensed, cross-platform, C++ port of the (now unmaintained) [python CEED](https://bitbucket.org/cegui/ceed/).  
It provides a multi-tab CEGUI layout designer and imageset editor.

Collaborators are highly welcome.


Dependencies
-------------
1. [QtnProperty](https://github.com/lexxmark/QtnProperty) - a Qt property framework. Sources are included;
2. CEGUI [(default branch)](https://bitbucket.org/cegui/cegui/src/default/) with the OpenGLRenderer or OpenGL3Renderer;
3. Qt (Only tested with QT 5.12 yet, older versions down to 5.8 inclusive should also work).

Preparing CEGUI
-------------
Windows: build CEGUI and either place it into /3rdParty/CEGUI/\[bin, lib, include, dependencies\] or fix your local .pro file to point to the build. Prebuilt CEGUI to be included in the near future.


Building with Qt Creator 4.x
-------------
Open the Editor.pro with Qt Creator


Building with MSVC 2015 or later
-------------
Use your favorite shell to go into the ceed-cpp directory, and run:

```
qmake -tp vc
```

You can then build it using the generated Visual Studio project files.  
QT 5.12 seems to only support x64 with MSVC 2015. With MSVC 2017 the x86 and x64 target is supported

  

Acknowledgements
----------------

- The python CEED Team and contributors for the original source code this is based off of.

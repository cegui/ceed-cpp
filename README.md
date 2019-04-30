CEED C++ (`CE`gui`ED`itor)
===========


CEED C++ is a GPL3-licensed, cross-platform, C++ port of the (now unmaintained) [python CEED](https://bitbucket.org/cegui/ceed/).  
It provides a multi-tab CEGUI layout designer and imageset editor.

Collaborators are highly welcome.


Dependencies
-------------
2. CEGUI [(default branch)](https://bitbucket.org/cegui/cegui/src/default/) with the OpenGLRenderer
3. Qt (Only tested with QT 5.12 yet, older versions should also work)

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

CEED C++ (`CE`gui`ED`itor)
===========


CEED C++ is a GPL3-licensed, cross-platform, C++ port of the (now unmaintained) [python CEED](https://bitbucket.org/cegui/ceed/).
It provides a multi-tab CEGUI layout designer and imageset editor.

Collaborators are highly welcome.


Dependencies
-------------
1. [QtnProperty v2.0.3 (c7ff097+)](https://github.com/qtinuum/QtnProperty) - a Qt property framework. Sources are included;
2. CEGUI [(master branch)](https://github.com/cegui/cegui) with the OpenGL3Renderer (old OpenGLRenderer is working too but disabled by default);
3. Qt (tested with 5.12, 5.13 and 5.15.2, older versions down to 5.8 inclusive should also work).
4. [OpenSSL](https://www.openssl.org/) 1.1.0 - update checking and downloading
5. [zlib](https://github.com/madler/zlib) - update zip unpacking. Sources are included;
6. [minizip-ng](https://github.com/zlib-ng/minizip-ng) - update zip unpacking. Sources are included;

Preparing CEGUI
-------------
Windows: build CEGUI and either place it into /3rdParty/CEGUI/\[bin, lib, include, dependencies\] or fix your local .pro file to point to the build. If you have your cegui and ceed-cpp repo folders side by side, use /3rdParty/CEGUI/update_cegui_win_sdk.cmd. Prebuilt 32-bit CEGUI for Windows is included into the [GitHub release](https://github.com/cegui/ceed-cpp/releases).


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
- [OpenSSL](https://www.openssl.org/). Distributed under the [double license of the OpenSSL and SSLeay licenses](https://www.openssl.org/source/license-openssl-ssleay.txt). Copyright (c) 1998-2021 The OpenSSL Project. Copyright (C) 1995-1998 Eric Young.

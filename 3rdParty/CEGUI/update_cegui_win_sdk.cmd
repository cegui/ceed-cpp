@echo off
set src_bin_folder=..\..\..\cegui\build\bin
set src_lib_folder=..\..\..\cegui\build\lib
set src_dep_folder=..\..\..\cegui\dependencies\bin

rem CEGUI Includes
rmdir /S /Q "include"
xcopy /S /Y /Q /I "..\..\..\cegui\cegui\include" "include"
xcopy /S /Y /Q /I "..\..\..\cegui\build\cegui\include" "include"

rem CEGUI DLLs
copy /Y "%src_bin_folder%\CEGUIBase-9999.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUICommonDialogs-9999.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUICoreWindowRendererSet.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUIExpatParser.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUIOpenGLRenderer-9999.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUISILLYImageCodec.dll" ".\bin\release"
copy /Y "%src_bin_folder%\CEGUIBase-9999_d.dll" ".\bin\debug"
copy /Y "%src_bin_folder%\CEGUICommonDialogs-9999_d.dll" ".\bin\debug"
copy /Y "%src_bin_folder%\CEGUICoreWindowRendererSet_d.dll" ".\bin\debug"
copy /Y "%src_bin_folder%\CEGUIExpatParser_d.dll" ".\bin\debug"
copy /Y "%src_bin_folder%\CEGUIOpenGLRenderer-9999_d.dll" ".\bin\debug"
copy /Y "%src_bin_folder%\CEGUISILLYImageCodec_d.dll" ".\bin\debug"

rem CEGUI Libs
copy /Y "%src_lib_folder%\CEGUIBase-9999.lib" ".\lib"
copy /Y "%src_lib_folder%\CEGUIOpenGLRenderer-9999.lib" ".\lib"
copy /Y "%src_lib_folder%\CEGUIBase-9999_d.lib" ".\lib"
copy /Y "%src_lib_folder%\CEGUIOpenGLRenderer-9999_d.lib" ".\lib"

rem CEGUI Dependency Includes
rmdir /S /Q "dependencies"
xcopy /S /Y /Q /I "..\..\..\cegui\dependencies\include" "dependencies\include"
xcopy /S /Y /Q /I "..\..\..\cegui\dependencies\*.txt" "dependencies"

rem CEGUI Dependency DLLs
copy /Y "%src_dep_folder%\freetype.dll" ".\bin\release"
copy /Y "%src_dep_folder%\fribidi.dll" ".\bin\release"
copy /Y "%src_dep_folder%\glew.dll" ".\bin\release"
copy /Y "%src_dep_folder%\harfbuzz.dll" ".\bin\release"
copy /Y "%src_dep_folder%\jpeg.dll" ".\bin\release"
copy /Y "%src_dep_folder%\libexpat.dll" ".\bin\release"
copy /Y "%src_dep_folder%\libpng.dll" ".\bin\release"
copy /Y "%src_dep_folder%\raqm.dll" ".\bin\release"
copy /Y "%src_dep_folder%\SILLY.dll" ".\bin\release"
copy /Y "%src_dep_folder%\zlib.dll" ".\bin\release"
copy /Y "%src_dep_folder%\freetype_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\fribidi_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\glew_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\harfbuzz_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\jpeg_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\libexpat_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\libpng_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\raqm_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\SILLY_d.dll" ".\bin\debug"
copy /Y "%src_dep_folder%\zlib_d.dll" ".\bin\debug"

pause

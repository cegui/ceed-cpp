
HEADERS += \
    $$PWD/mz.h \
    $$PWD/mz_crypt.h \
    $$PWD/mz_os.h \
    $$PWD/mz_strm.h \
    $$PWD/mz_strm_buf.h \
    $$PWD/mz_strm_mem.h \
    $$PWD/mz_strm_os.h \
    $$PWD/mz_strm_split.h \
    $$PWD/mz_strm_zlib.h \
    $$PWD/mz_zip.h \
    $$PWD/mz_zip_rw.h

SOURCES += \
    $$PWD/mz_crypt.c \
    $$PWD/mz_os.c \
    $$PWD/mz_strm.c \
    $$PWD/mz_strm_buf.c \
    $$PWD/mz_strm_mem.c \
    $$PWD/mz_strm_split.c \
    $$PWD/mz_strm_zlib.c \
    $$PWD/mz_zip.c \
    $$PWD/mz_zip_rw.c

win32 {
	SOURCES += $$PWD/mz_os_win32.c \
		$$PWD/mz_strm_os_win32.c
} else {
	SOURCES += $$PWD/mz_os_posix.c \
		$$PWD/mz_strm_os_posix.c
}

INCLUDEPATH += $$PWD

DEFINES += _CRT_SECURE_NO_DEPRECATE HAVE_ZLIB NO_FSEEKO HAVE_STDINT_H HAVE_INTTYPES_H MZ_ZIP_NO_CRYPTO MZ_ZIP_NO_ENCRYPTION


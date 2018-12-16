#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#
#    .--------------------------------------------------.
#    |  This file is part of NTGraphics                 |
#    |  Created 2018 by NT (https://ttnghia.github.io)  |
#    '--------------------------------------------------'
#                            \o/
#                             |
#                            / |
#
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

include($$PWD/../LibCommon/LibCommon.pri)

TARGET = LibSimulation

TEMPLATE = lib
CONFIG += staticlib

HEADERS = $$files($$PWD/LibSimulation/*.h, true)
SOURCES += $$files($$PWD/LibSimulation/*.cpp, true)

#-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
CONFIG(debug, debug|release) {
    CONFIG_NAME = Debug
} else {
    static {
        CONFIG_NAME = ReleaseStaticBuild
    } else {
       CONFIG_NAME = Release
    }
}

DESTDIR = $$PWD/../../Build/$${CONFIG_NAME}

win32: QMAKE_POST_LINK += $$quote(if exist \"$$shell_path($$OUT_PWD/$${CONFIG_NAME}/$${TARGET}.pdb)\" \
                           xcopy /C /r /y \"$$shell_path($$OUT_PWD/$${CONFIG_NAME}/$${TARGET}.pdb)\" \"$$shell_path($$PWD/../../Build/$${CONFIG_NAME}/)\")

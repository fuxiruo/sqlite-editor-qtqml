TEMPLATE = app

QT += qml quick sql core widgets

HEADERS += \
dbthread.h \
sqlitemodel.h

SOURCES += main.cpp \
dbthread.cpp \
sqlitemodel.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
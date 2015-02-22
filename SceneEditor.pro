#-------------------------------------------------
#
# Project created by QtCreator 2014-11-19T10:11:46
#
#-------------------------------------------------

QT       += core gui opengl multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SceneEditor
TEMPLATE = app

INCLUDEPATH += .

SOURCES += main.cpp\
        SeMainWindow.cpp \
    SeTreeScenes.cpp \
    SeMdiArea.cpp \
    SeSceneItem.cpp \
    SeSceneLed.cpp \
    SeScene.cpp \
    SeSceneView.cpp \
    SeSceneLayer.cpp \
    SeScenePlayer.cpp \
    SeMosaicWindow.cpp

HEADERS  += SeMainWindow.h \
    SeTreeScenes.h \
    SeMdiArea.h \
    SeSceneItem.h \
    SeSceneLed.h \
    SeScene.h \
    SeSceneView.h \
    SeSceneLayer.h \
    SeScenePlayer.h \
    SeMosaicWindow.h \
    SeGeneral.h

FORMS    += SeMainWindow.ui \
    SeMosaicWindow.ui

RESOURCES += \
    images.qrc

DISTFILES += \
    README \
    LICENSE

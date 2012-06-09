#-------------------------------------------------
#
# Project created by QtCreator 2012-03-13T23:28:22
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = qpdf
TEMPLATE = app


SOURCES += main.cpp \
    qpdf.cpp \
    pdfwidget.cpp

HEADERS  += \
    qpdf.h \
    pdfwidget.h

FORMS    += qpdf.ui

LIBS += -lpoppler-qt4 -lGLU

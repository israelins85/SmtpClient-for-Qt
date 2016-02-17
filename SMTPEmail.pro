#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T20:59:25
#
#-------------------------------------------------

QT       += core network

TARGET = SMTPEmail

# Build as an application
#TEMPLATE = app

# Build as a library
TEMPLATE = lib
DEFINES += SMTP_BUILD
win32:CONFIG += dll

include(SMTPEmail.pri)

OTHER_FILES += \
    LICENSE \
    README.md

FORMS +=

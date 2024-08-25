QMAKE_MACOSX_DEPLOYMENT_TARGET=10.15
QT       += core gui

#---opengl----------------------------
QT       +=opengl
QT	 += multimedia
QT += multimediawidgets
QT += openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG+=sdk_no_version_check


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FitParse/fitparse.cpp \
    FitSDK/fit.cpp \
    FitSDK/fit_accumulated_field.cpp \
    FitSDK/fit_accumulator.cpp \
    FitSDK/fit_buffer_encode.cpp \
    FitSDK/fit_buffered_mesg_broadcaster.cpp \
    FitSDK/fit_buffered_record_mesg_broadcaster.cpp \
    FitSDK/fit_crc.cpp \
    FitSDK/fit_date_time.cpp \
    FitSDK/fit_decode.cpp \
    FitSDK/fit_developer_field.cpp \
    FitSDK/fit_developer_field_definition.cpp \
    FitSDK/fit_developer_field_description.cpp \
    FitSDK/fit_encode.cpp \
    FitSDK/fit_factory.cpp \
    FitSDK/fit_field.cpp \
    FitSDK/fit_field_base.cpp \
    FitSDK/fit_field_definition.cpp \
    FitSDK/fit_mesg.cpp \
    FitSDK/fit_mesg_broadcaster.cpp \
    FitSDK/fit_mesg_definition.cpp \
    FitSDK/fit_mesg_with_event_broadcaster.cpp \
    FitSDK/fit_profile.cpp \
    FitSDK/fit_protocol_validator.cpp \
    FitSDK/fit_unicode.cpp \
    VideoRenderer/videorenderer.cpp \
    core/utils.cpp \
    dashboard/dashboard.cpp \
    decoder/decoder.cpp \
    main.cpp \
    mainwindow.cpp \
    myslide/myslide.cpp

HEADERS += \
    FitParse/fitparse.h \
    FitParse/listener.h \
    FitSDK/*.hpp \
    Logger/logger.h \
    VideoPlayer/VideoPlayer.h \
    VideoRenderer/videorenderer.h \
    core/defines.h \
    core/ring_buffer.hpp \
    core/types.h \
    core/utils.h \
    dashboard/dashboard.h \
    decoder/decoder.h \
    mainwindow.h \
    myslide/myslide.h

FORMS += \
    dashboard/dashboard.ui \
    mainwindow.ui \
    myslide/myslide.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    nootbook.txt \
    shader/fragmentshader.frag \
    shader/vertexshader.vert



#arm64 的编译宏
contains(QMAKE_HOST.arch, aarch64){
    #aarch64 是编译器类型，如果不是这个编译器， 可以把.arch 替换成对应的类型
    #arm 64平台
    message("编译目标平台: arm-64")
    message("编译器平台类型: $$QMAKE_HOST.arch")
}else{
    message("编译目标平台: x86")
    message("编译器平台类型: $$QMAKE_HOST.arch")

    win32{
        # 仅x86-windows下的
        message("win32")
        QMAKE_CXXFLAGS += "-Wa,-mbig-obj"

        win32: LIBS += -LE:/install/ffmpeg-6.1.1/lib/ -lavcodec    \
                                                      -lavformat   \
                                                      -lavdevice   \
                                                      -lavfilter   \
                                                      -lavutil     \
                                                      -lpostproc   \
                                                      -lswresample \
                                                      -lswscale

        LIBS += -lopengl32  -lGLU32

        INCLUDEPATH += E:/install/ffmpeg-6.1.1/include
        DEPENDPATH += E:/install/ffmpeg-6.1.1/include

    }

    #仅在linux -x86 平台下的内容
    unix{
        message("unix")
        INCLUDEPATH += \
            /usr/local/ffmpeg/include/
        LIBS += \
            /usr/local/ffmpeg/lib/libavformat.61.dylib \
            /usr/local/ffmpeg/lib/libavcodec.61.dylib \
            /usr/local/ffmpeg/lib/libavdevice.61.dylib \
            /usr/local/ffmpeg/lib/libavfilter.10.dylib \
            /usr/local/ffmpeg/lib/libavutil.59.dylib \
            /usr/local/ffmpeg/lib/libpostproc.58.dylib \
            /usr/local/ffmpeg/lib/libswresample.5.dylib \
            /usr/local/ffmpeg/lib/libswscale.8.dylib
    }
}

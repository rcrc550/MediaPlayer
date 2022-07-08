QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QT += multimedia
INCLUDEPATH+=C:/Qt/opencv-release/include/opencv2\
C:/Qt/opencv-release/include\
$$PWD/ffmpeg-4.2.2/include





LIBS+=C:\Qt\opencv-release\lib\libopencv_calib3d420.dll.a\
C:\Qt\opencv-release\lib\libopencv_core420.dll.a\
C:\Qt\opencv-release\lib\libopencv_features2d420.dll.a\
C:\Qt\opencv-release\lib\libopencv_flann420.dll.a\
C:\Qt\opencv-release\lib\libopencv_highgui420.dll.a\
C:\Qt\opencv-release\lib\libopencv_imgproc420.dll.a\
C:\Qt\opencv-release\lib\libopencv_ml420.dll.a\
C:\Qt\opencv-release\lib\libopencv_objdetect420.dll.a\
C:\Qt\opencv-release\lib\libopencv_video420.dll.a\
C:\Qt\opencv-release\lib\libopencv_videoio420.dll.a\
$$PWD/ffmpeg-4.2.2/lib/avcodec.lib\
$$PWD/ffmpeg-4.2.2/lib/avdevice.lib\
$$PWD/ffmpeg-4.2.2/lib/avfilter.lib\
$$PWD/ffmpeg-4.2.2/lib/avformat.lib\
$$PWD/ffmpeg-4.2.2/lib/avutil.lib\
$$PWD/ffmpeg-4.2.2/lib/postproc.lib\
$$PWD/ffmpeg-4.2.2/lib/swresample.lib\
$$PWD/ffmpeg-4.2.2/lib/swscale.lib

SOURCES += \
    audio_read.cpp \
    main.cpp \
    picinpic_read.cpp \
    recorddialog.cpp \
    savevideofile.cpp \
    screenrecorder.cpp \
    videoshow.cpp

HEADERS += \
    audio_read.h \
    common.h \
    picinpic_read.h \
    recorddialog.h \
    savevideofile.h \
    screenrecorder.h \
    videoshow.h

FORMS += \
    recorddialog.ui \
    videoshow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#ifndef COMMON_H
#define COMMON_H
extern "C"
{
 #include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include"libswresample/swresample.h"
#include"libavutil/time.h"

}
#include<QAudioDeviceInfo>
#include<QDebug>
#include<QMessageBox>
#include<QImage>
#include<QImageReader>
#include<QBuffer>
#include<QPainter>
#include<QTimer>
#include<QTime>
#include"highgui/highgui.hpp"
#include"imgproc/imgproc.hpp"
#include"core/core.hpp"
#include"opencv2\imgproc\types_c.h"
using namespace cv;
#define FRAME_RATE (15)
#endif // COMMON_H

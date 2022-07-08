#include "videodialog.h"

#include <QApplication>
#include<iostream>

using namespace std;
#undef main
int main(int argc, char *argv[])
{
    cout << "Hello FFmpeg!" << endl;
    av_register_all();
    //unsigned version = avcodec_version();
   // cout << "version is:" << version<<endl;
    QApplication a(argc, argv);
    VideoDialog w;
    w.show();
    return a.exec();
}

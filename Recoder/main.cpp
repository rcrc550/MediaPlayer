#include "recorddialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RecordDialog w;
    w.show();
    return a.exec();
}

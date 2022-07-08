#ifndef ONLINEDIALOG_H
#define ONLINEDIALOG_H

#include <QDialog>
#include<QString>
namespace Ui {
class OnlineDialog;
}

class OnlineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineDialog(QWidget *parent = nullptr);
    ~OnlineDialog();
signals:
    void SIG_rtmp(QString str);
private slots:
    void on_pb_send_clicked();

private:
    Ui::OnlineDialog *ui;
};

#endif // ONLINEDIALOG_H

#include "onlinedialog.h"
#include "ui_onlinedialog.h"

OnlineDialog::OnlineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineDialog)
{
    ui->setupUi(this);
}

OnlineDialog::~OnlineDialog()
{
    delete ui;
}

void OnlineDialog::on_pb_send_clicked()
{
    QString str=ui->le_rtmp->text();
    if(!str.isEmpty())
    {
        Q_EMIT SIG_rtmp(str);
    }
    this->close();
}


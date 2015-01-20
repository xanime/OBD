#include "obdsur.h"
#include "ui_obdsur.h"

OBDSUR::OBDSUR(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OBDSUR)
{
    ui->setupUi(this);
}

OBDSUR::~OBDSUR()
{
    delete ui;
}

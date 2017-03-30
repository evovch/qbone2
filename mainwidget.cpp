#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
}

MainWidget::setIp(QString ip)
{
    ui->label_ip->setText("IP: " + ip);
}


MainWidget::~MainWidget()
{
    delete ui;
}

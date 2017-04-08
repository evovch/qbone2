#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
}

void MainWidget::setIp(QString ip)
{
    ui->label_ip->setText("IP: " + ip);
}


MainWidget::~MainWidget(void)
{
    delete ui;
}

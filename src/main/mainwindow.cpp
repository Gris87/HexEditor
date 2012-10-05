#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mHexEditor=new HexEditor(this);
    ui->hexLayout->addWidget(mHexEditor);
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mHexEditor=new HexEditor(this);
    ui->hexLayout->addWidget(mHexEditor);

    QString aData="Hello, World! It is test data for HexEditor...\n\tYou can modify any binary data with it. Please try.\nGarbage:";

    for (int i=0; i<1024; ++i)
    {
        aData.append("X");
    }

    mHexEditor->setData(aData.toUtf8());
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mHexEditor=new HexEditor(this);
    ui->hexLayout->addWidget(mHexEditor);

    QString aData="Hello, World!\n    It is test data for HexEditor...\n\tYou can modify any binary data with it. Please try.\nGarbage:";

    for (int i=0; i<1024; ++i)
    {
        aData.append("X");
    }

    QByteArray aArray=aData.toUtf8();

    for (int i=0; i<256; ++i)
    {
        aArray.append(i);
    }

    aArray.append(46);
    aArray.append(78);
    aArray.append(56);
    aArray.append(67);

    mHexEditor->setData(aArray);

    QPalette aPalette=mHexEditor->palette();

    aPalette.setColor(QPalette::Base, QColor(245, 245, 255));
    aPalette.setColor(QPalette::AlternateBase, QColor(10, 200, 90));

    mHexEditor->setPalette(aPalette);
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingPage.cpp"
#include "quitAppPage.cpp"
#include "newCardPage.cpp"
#include "reportLossPage.cpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 工具栏设置
    addToolBar(Qt::LeftToolBarArea, ui->toolBar);
    ui->toolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    ui->toolBar->setFloatable(false);
    ui->toolBar->setMovable(false);

    // 状态栏设置
    ui->statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));

    readerConnectStatusCheckBox = new QCheckBox();
    readerConnectStatusCheckBox->setChecked(false);
    ui->statusBar->addWidget(readerConnectStatusCheckBox);

    comNumberLabel = new QLabel("当前无连接");
    ui->statusBar->addWidget(comNumberLabel);

    databaseConnectStatusCheckBox = new QCheckBox();
    databaseConnectStatusCheckBox->setChecked(false);
    ui->statusBar->addWidget(databaseConnectStatusCheckBox);

    databaseLabel = new QLabel("数据库无连接");
    ui->statusBar->addWidget(databaseLabel);

    deviceLabel = new QLabel(device.getNameAndDepositAllowed());
    ui->statusBar->addWidget(deviceLabel);    


    // 清空部分输入框
    ui->userIdBox->clear();
    newCardUserIdFilled = false;
    connect(ui->userIdBox, &QSpinBox::valueChanged, [this]{ newCardUserIdFilled = true; });
    ui->reportLossUserIdBox->clear();
    reportLossUserIdFilled = false;
    connect(ui->reportLossUserIdBox, &QSpinBox::valueChanged, [this]{ reportLossUserIdFilled = true; });


    // 设置启动页面
    ui->stackedWidget->setCurrentWidget(ui->settingPage);
}


MainWindow::~MainWindow()
{
    delete ui;
}


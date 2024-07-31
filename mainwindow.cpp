#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingPage.cpp"
#include "quitAppPage.cpp"
#include "newCardPage.cpp"
#include "reportLossPage.cpp"
#include "depositPage.cpp"
#include "consumePage.cpp"
#include "queryPage.cpp"


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

    connect(ui->depositUserIdBox, &QSpinBox::valueChanged, [this]{ depositUserIdFilled = true; });

    connect(ui->queryUserIdBox, &QSpinBox::valueChanged, [this]{ queryUserIdFilled = true; });


    // 查询表格设置
    ui->queryResultTable->setShowGrid(true);                                    //设置显示格子线
    ui->queryResultTable->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中
    ui->queryResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止编辑
    ui->queryResultTable->horizontalHeader()->setStretchLastSection(true);      //行头自适应表格
    ui->queryResultTable->horizontalHeader()->setHighlightSections(false);      //点击表头时不对表头光亮
    ui->queryResultTable->setSortingEnabled(true);                              //启动排序
    // 设置表头字体加粗
    QFont queryResultTableFont =  ui->queryResultTable->horizontalHeader()->font();
    queryResultTableFont.setBold(true);
    ui->queryResultTable->horizontalHeader()->setFont(queryResultTableFont);
    // 设置表头内容
    QStringList queryResultTableHeader;
    queryResultTableHeader << "时间" << "类型" << "金额" << "余额" << "设备" << "交易号";
    ui->queryResultTable->setColumnCount(queryResultTableHeader.size());            //设置列数
    ui->queryResultTable->setHorizontalHeaderLabels(queryResultTableHeader);
    ui->queryResultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter); // 每列居中对齐

    ui->queryResultTable->setColumnWidth(0, 130);
    ui->queryResultTable->setColumnWidth(1, 40);
    ui->queryResultTable->setColumnWidth(2, 60);
    ui->queryResultTable->setColumnWidth(3, 80);
    ui->queryResultTable->setColumnWidth(4, 100);
    ui->queryResultTable->setColumnWidth(5, 240);



    // 设置启动页面
    ui->stackedWidget->setCurrentWidget(ui->settingPage);
}


MainWindow::~MainWindow()
{
    delete ui;
}


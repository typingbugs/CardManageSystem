#include "mainwindow.h"
#include "ui_mainwindow.h"


/*
 * 功能：切换到退出页面
 * 触发：点击工具栏的“退出”
*/
void MainWindow::on_quitAppAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->quitAppPage);
}


/*
 * 功能：退出程序
 * 触发：在退出页面点击“确认”按钮
*/
void MainWindow::on_confirmQuitButton_clicked()
{
    this->close();
}

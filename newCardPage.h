#ifndef NEWCARDPAGE_H
#define NEWCARDPAGE_H

#include "mainwindow.h"
#include "ui_mainwindow.h"


/*
 * 功能：切换到开卡页面
 * 触发：点击工具栏的“开卡”
*/
void MainWindow::on_NewCardAction_triggered()
{
    if (!ready())
    {
        QMessageBox::warning(this, QString("提示"), QString("读卡器或数据库未连接，请设置。"));
        ui->stackedWidget->setCurrentWidget(ui->settingPage);
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->newCardPage);
}


/*
 * 功能：读卡器扫描卡片
 * 触发：点击开卡页面的“查询”
*/
void MainWindow::on_inventoryButton_clicked()
{

}


#endif // NEWCARDPAGE_H

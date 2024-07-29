#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到开卡页面
 *  点击工具栏的“开卡”触发：
 *  - 如果读卡器或数据库未连接，显示警告信息并跳转到设置页面。
 *  - 如果连接已准备好，则切换到开卡页面。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void MainWindow::on_NewCardAction_triggered()
{
    if (!ready())
    {
        QMessageBox::warning(this, QString("提示"), QString("读卡器或数据库未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    ui->stackedWidget->setCurrentWidget(ui->newCardPage);
}


/**
 * @brief   读卡器扫描卡片
 *  点击开卡页面的“查询”触发。显示Inventory的查询结果，最多显示10张卡。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-29
 */
void MainWindow::on_inventoryButton_clicked()
{
    QStringList cardIdList = reader.inventory(10);  // 最多显示10张卡
    ui->cardIdBox->clear();
    ui->cardIdBox->addItems(cardIdList);
}

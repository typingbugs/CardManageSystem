#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到消费页面
 *  点击工具栏的“消费”触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_ConsumptionAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->consumePage);
}


/**
 * @brief   读卡器扫描卡片
 * 点击消费页面的“查询”触发。
 * 如果读卡器未连接，显示警告信息并跳转到设置页面。
 * 显示Inventory的查询结果，最多显示10张卡。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_consumeInventoryButton_clicked()
{
    if (!reader.is_connected())
    {
        QMessageBox::warning(this, QString("提示"), QString("读卡器未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    QStringList cardIdList = reader.inventory(10);  // 最多显示10张卡
    ui->consumeCardIdBox->clear();
    if (cardIdList.empty())
    {
        QMessageBox::warning(this, "提示", "未发现卡片，请将卡片放置于读卡器上方。");
    }
    else
    {
        ui->consumeCardIdBox->addItems(cardIdList);
    }
}

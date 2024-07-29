#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到设置页面
 *  点击工具栏的“设置”触发。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::on_settingAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->settingPage);
}


/**
 * @brief   更新状态栏的COM口号相关内容
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::updateStatusBarComNumber()
{
    if (reader.is_connected())
    {
        readerConnectStatusCheckBox->setChecked(true);
        QString s = "已连接读卡器COM：" + QString::number(reader.getComNumber());
        comNumberLabel->setText(s);
    }
    else
    {
        readerConnectStatusCheckBox->setChecked(false);
        QString s = "当前无连接";
        comNumberLabel->setText(s);
    }
}


/**
 * @brief   连接读卡器
 *  在连接读卡器部分点击“连接”按钮触发。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::on_connectReaderButton_clicked()
{
    int comNumber = ui->comNumberBox->value();
    reader.setComNumber(comNumber);
    bool connectSuccess = reader.connect();

    if (!connectSuccess) {
        QString warningMessage = QString("COM") + QString::number(comNumber);
        warningMessage += QString("上未识别到读卡器。\n请输入正确的COM口号。");
        QMessageBox::warning(this, "读卡器状态提示", warningMessage);
    }

    updateStatusBarComNumber();
}


/**
 * @brief   连接数据库并检查设备名
 *  在连接数据库部分点击“连接”按钮触发。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::on_connectDatabaseButton_clicked()
{
    if (db == nullptr)
    {
        db = new Database(QSqlDatabase::addDatabase("QMYSQL"), ui->ipAddrEdit->text(), ui->portBox->value(), ui->passwordEdit->text());
    }
    else
    {
        db->setHostName(ui->ipAddrEdit->text());
        db->setPort(ui->portBox->value());
        db->setPassword(ui->passwordEdit->text());
    }

    if (!db->open())
    {
        QMessageBox::warning(this, QString("数据库状态提示"), QString("数据库连接失败，请重试。"));
        databaseConnectStatusCheckBox->setChecked(false);
        databaseLabel->setText(QString("数据库未连接"));
        return;
    }
    databaseConnectStatusCheckBox->setChecked(true);
    databaseLabel->setText(QString("数据库已连接：") + db->getHostName() + QString(":") + QString::number(db->getPort()));

    device.setDevice(ui->deviceEdit->text(), db);
    if (!device.is_verified())
    {
        QMessageBox::warning(this, QString("设备名提示"), QString("该设备名无效，请重试。"));
    }
    deviceLabel->setText(device.getNameAndDepositAllowed());
}


/**
 * @brief   检查读卡器和数据库是否准备好
 * @param   void
 * @return  读卡器和数据库状态
 *  - true  已准备好
 *  - false 未准备好
 * @author  柯劲帆
 * @date    2024-07-27
 */
bool MainWindow::ready()
{
    if (!reader.is_connected()) return false;
    if (db == nullptr || !db->is_connected()) return false;
    if (!device.is_verified()) return false;
    return true;
}

#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include "mainwindow.h"
#include "ui_mainwindow.h"


/*
 * 功能：切换到设置页面
 * 触发：点击工具栏的“设置”
*/
void MainWindow::on_settingAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->settingPage);
}


/*
 * 功能：设置连接的读卡器的COM口号并更新状态栏的相应内容
 * 输入：要设置的COM口号
 * 返回：无
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


/*
 * 功能：连接读卡器
 * 触发：在设置页面的连接读卡器部分点击“连接”按钮
*/
void MainWindow::on_connectReaderButton_clicked()
{
    int comNumber = ui->comNumberBox->value();
    reader.setComNumber(comNumber);
    bool connectSuccess = reader.connectReader();

    if (!connectSuccess) {
        QString warningMessage = QString("COM") + QString::number(comNumber);
        warningMessage += QString("上未识别到读卡器。\n请输入正确的COM口号。");
        QMessageBox::warning(this, "读卡器状态提示", warningMessage);
    }

    updateStatusBarComNumber();
}


/*
 * 功能：连接数据库并检查设备名
 * 触发：在设置页面的连接数据库部分点击“连接”按钮
*/
void MainWindow::on_connectDatabaseButton_clicked()
{
    databaseIpAddr = ui->ipAddrEdit->text();
    databasePort = ui->portBox->value();
    databasePassword = ui->passwordEdit->text();

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(databaseIpAddr);
    db.setPort(databasePort);
    db.setPassword(databasePassword);
    db.setDatabaseName("cardManageSystem");
    db.setUserName("cardManageSystem");

    if (!db.open())
    {
        QMessageBox::warning(this, QString("数据库状态提示"), QString("数据库连接失败，请重试。"));
        databaseConnectStatusCheckBox->setChecked(false);
        databaseLabel->setText(QString("数据库未连接"));
        databaseIpAddr = "";
        return;
    }
    databaseConnectStatusCheckBox->setChecked(true);
    databaseLabel->setText(QString("数据库已连接：") + databaseIpAddr + QString(":") + QString::number(databasePort));

    device = ui->deviceEdit->text();
    QSqlQuery query;
    QString sql = QString("select * from device where id = '%1';").arg(device);
    query.exec(sql);
    if (query.next())
    {
        if (query.value(2).toBool()) depositAllowed = true;
        else depositAllowed = false;
    }
    else
    {
        device = QString("未指定设备名");
        QMessageBox::warning(this, QString("设备名提示"), QString("该设备名无效，请重试。"));
    }
    deviceLabel->setText(device);
}


/*
 * 功能：检查读卡器和数据库是否准备好
 * 输入：无
 * 返回：true - 已准备好 | false - 未准备好
*/
bool MainWindow::ready()
{
    if (!reader.is_connected() || databaseIpAddr.isEmpty()) return false;
    return true;
}


#endif // SETTINGPAGE_H

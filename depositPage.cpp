#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到充值页面
 *  点击工具栏的“充值”触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_depositAction_triggered()
{
    ui->depositUserIdBox->clear();
    depositUserIdFilled = false;
    ui->stackedWidget->setCurrentWidget(ui->depositPage);
}


/**
 * @brief   读卡器扫描卡片
 * 点击充值页面的“查询”触发。
 * 如果读卡器未连接，显示警告信息并跳转到设置页面。
 * 显示Inventory的查询结果，最多显示10张卡。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_depositInventoryButton_clicked()
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
    ui->depositCardIdBox->clear();
    if (cardIdList.empty())
    {
        QMessageBox::warning(this, "提示", "未发现卡片，请将卡片放置于读卡器上方。");
    }
    else
    {
        ui->depositCardIdBox->addItems(cardIdList);
    }
}


/**
 * @brief   按卡号充值
 * 该函数在用户点击“按卡号充值”按钮时触发，用于对指定卡号进行充值操作。
 * @param   void
 * @return  void
 * @details
 * 函数首先检查软件和设备的准备状态。如果数据库未连接或设备不支持充值，函数将显示警告信息并返回。
 * 接着，函数检查是否选择了卡号和充值金额。如果未选择卡号或充值金额无效，函数将显示警告信息并返回。
 * 然后，函数调用 `topUpCard` 函数进行充值操作，并在操作成功后显示充值结果。
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_depositByCardIdButton_clicked()
{
    if (!softwareReady())
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    if (!reader.is_connected())
    {
        QMessageBox::warning(this, QString("提示"), QString("读卡器未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    if (!device.is_depositAllowed())
    {
        QMessageBox::warning(this, QString("提示"), QString("本设备不支持充值。"));
        return;
    }

    if (ui->depositCardIdBox->currentIndex() == -1)
    {
        QMessageBox::warning(this, "提示", "请放置卡片并点击查询按钮。");
        return;
    }
    QString cardId = ui->depositCardIdBox->currentText();

    double topUpValue = ui->depositValueBox->value();
    if (!(topUpValue > 0.00))
    {
        QMessageBox::warning(this, "提示", "请输入大于0.00的充值金额。");
        return;
    }

    bool success = false;
    QString info;
    double originalBalance, finalBalance;
    QString recordId;
    success = topUpCard(cardId, topUpValue, originalBalance, finalBalance, recordId, info);
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n充值失败。"));
        return;
    }

    QString depositResultMessage = QString("充值成功：") + QString::number(topUpValue) + QString("元\n");
    depositResultMessage += QString("原余额：") + QString::number(originalBalance) + QString("元\n");
    depositResultMessage += QString("充值后余额：") + QString::number(finalBalance) + QString("元\n");
    depositResultMessage += QString("充值设备：") + device.getName() + QString("\n");
    depositResultMessage += QString("交易记录号：") + recordId + QString("\n");
    QMessageBox::information(this, "充值成功", depositResultMessage);
    return;
}


/**
 * @brief   按学/工号充值
 * 该函数在用户点击“按学/工号充值”按钮时触发，用于对指定学/工号的用户进行充值操作。
 * @param   void
 * @return  void
 * @details
 * 函数首先检查软件和设备的准备状态。如果数据库未连接或设备不支持充值，函数将显示警告信息并返回。
 * 接着，函数检查是否填写了学/工号和充值金额。如果未填写或充值金额无效，函数将显示警告信息并返回。
 * 然后，函数在数据库中查询指定学/工号绑定的卡片。如果卡片不存在，函数将显示警告信息并返回。
 * 如果卡片存在，函数调用 `topUpCard` 函数进行充值操作，并在操作成功后显示充值结果。
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_depositByUserIdButton_clicked()
{
    if (!softwareReady())
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    if (!device.is_depositAllowed())
    {
        QMessageBox::warning(this, QString("提示"), QString("本设备不支持充值。"));
        return;
    }

    if (!depositUserIdFilled)
    {
        QMessageBox::warning(this, "提示", "请填写学/工号。");
        return;
    }
    int userId = ui->depositUserIdBox->value();

    double topUpValue = ui->depositValueBox->value();
    if (!(topUpValue > 0.00))
    {
        QMessageBox::warning(this, "提示", "请输入大于0.00的充值金额。");
        return;
    }

    QSqlQuery query(db->getDatabase());
    bool success = false;
    query.prepare(QString("select id from card "
                          "where userId = :userId;"));
    query.bindValue(":userId", userId);
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, "提示", "数据库异常。充值失败。");
        return;
    }
    if (!query.next())
    {
        QMessageBox::warning(this, "提示", "学/工号未绑定卡。充值失败。");
        return;
    }
    QString cardId = query.value("id").toString();

    QString info;
    double originalBalance = 0, finalBalance = 0;
    QString recordId;
    success = topUpCard(cardId, topUpValue, originalBalance, finalBalance, recordId, info);
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n充值失败。"));
        return;
    }

    QString depositResultMessage = QString("充值成功：") + QString::number(topUpValue) + QString("元\n");
    depositResultMessage += QString("原余额：") + QString::number(originalBalance) + QString("元\n");
    depositResultMessage += QString("充值后余额：") + QString::number(finalBalance) + QString("元\n");
    depositResultMessage += QString("充值设备：") + device.getName() + QString("\n");
    depositResultMessage += QString("交易记录号：") + recordId + QString("\n");
    QMessageBox::information(this, "充值成功", depositResultMessage);
    ui->depositValueBox->setValue(0.00);
    return;
}


/**
 * @brief   充值卡
 * 该函数用于给指定的卡片充值。充值前会进行一系列检查，包括设备是否支持充值、卡片状态、充值金额是否超过限额等。
 * 如果所有检查通过，将调用存储过程 `sp_depositCard` 进行充值操作，并返回充值后的余额。
 * @param   cardId          要充值的卡号
 * @param   topUpValue      充值金额
 * @param   originalBalance 充值前的余额，通过引用返回
 * @param   finalBalance    充值后的余额，通过引用返回
 * @param   recordId        交易编号，通过引用返回
 * @param   info            如果出现异常，填入异常信息，通过引用返回
 * @return  bool            是否充值成功
 * - true   成功
 * - false  失败
 * @details
 * 函数首先检查设备是否支持充值。如果设备不支持，函数返回失败并设置错误信息。
 * 接着，函数查询数据库获取卡片的当前状态和余额。如果卡片不存在、已挂失或未启用，函数返回失败并设置相应的错误信息。
 * 然后，函数检查充值后余额是否超过限额（9999.99元）。如果超过限额，函数返回失败并设置错误信息。
 * 如果所有检查通过，函数调用存储过程 `sp_depositCard` 进行充值操作，并通过存储过程返回充值后的余额。
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::topUpCard(QString cardId, double topUpValue, double &originalBalance, double &finalBalance, QString &recordId, QString &info)
{
    if (!device.is_depositAllowed())
    {
        info = "设备不支持充值。";
        return false;
    }

    QSqlQuery query(db->getDatabase());
    bool success;
    query.prepare(QString("select userId, `status`, balance from card "
                          "where id = :cardId;"));
    query.bindValue(":cardId", cardId);
    success = query.exec();
    if (!success)
    {
        info = "数据库异常。";
        return false;
    }
    if (!query.next())
    {
        info = "卡号不存在。";
        return false;
    }
    int cardStatus = query.value("status").toInt();
    if (cardStatus == -1)
    {
        info = "此卡已被挂失。";
        return false;
    }
    if (cardStatus == 0)
    {
        info = "此卡未被启用。";
        return false;
    }
    originalBalance = query.value("balance").toDouble();
    int userId = query.value("userId").toInt();

    if (originalBalance + topUpValue > (double)9999.99)
    {
        info = "充值金额超过限额。卡余额不得超过9999.99元。";
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    recordId = getRecordId(currentTime, userId, 1);

    query.finish();
    query.prepare(QString("call sp_depositCard (:cardId, :value, :recordId, :device, :time, :type, @newBalance);"));
    query.bindValue(":cardId", cardId);
    query.bindValue(":value", topUpValue);
    query.bindValue(":recordId", recordId);
    query.bindValue(":device", device.getId());
    query.bindValue(":time", currentTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":type", 1);
    success = query.exec();
    if (!success)
    {
        info = "数据库异常。";
        return false;
    }
    query.exec("select @newBalance;");
    query.next();
    finalBalance = query.value("@newBalance").toDouble();

    /// @todo 写卡

    return true;
}


/**
 * @brief   生成唯一的记录号
 * 该函数用于生成一个唯一的记录号，记录号由当前时间、用户ID、记录类型和一个随机数拼接而成。
 * @param   currentTime 当前时间
 * @param   userId      用户的学/工号
 * @param   recordType  记录类型
 * - 0  消费
 * - 1  充值
 * @return  QString     生成的唯一记录号（长度为30个字符）
 * @details
 * 函数首先将当前时间转换为字符串格式，然后将用户ID转换为11位长度的字符串（右对齐，不足部分用0填充）。
 * 接着生成一个随机的4位十六进制数，最后将这些部分拼接起来形成一个唯一的记录号。
 * @author  柯劲帆
 * @date    2024-07-30
 */
QString MainWindow::getRecordId(QDateTime currentTime, int userId, int recordType)
{
    QString timeStr = currentTime.toString("yyyyMMddhhmmss");   // 第0-13位：时间
    QString userIdStr = QString::number(userId).rightJustified(11, '0');    // 第14-24位：用户学/工号
    QString typeStr = QString::number(recordType);  // 第25位：记录类型
    QString randomHex = QString::number(QRandomGenerator::global()->bounded(0x10000), 16).rightJustified(4, '0');   // 第26-29位：随机十六进制数
    QString recordId = timeStr + userIdStr + typeStr + randomHex;   // 共30位
    return recordId;
}

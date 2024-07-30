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


/**
 * @brief   消费
 * 该函数在用户点击“消费”按钮时触发，用于对指定卡片进行消费操作。
 * @param   void
 * @return  void
 * @details
 * 函数首先检查软件是否已准备好并且数据库已连接。如果数据库未连接，函数将显示警告信息并返回。
 * 接着，函数检查是否选择了卡片。如果未选择卡片，函数将显示警告信息并返回。
 * 然后，函数检查并获取绑定的用户信息。如果卡片不存在或已挂失或未启用，函数将显示相应的警告信息并返回。
 * 如果消费金额大于50元，函数会要求进行密码验证。如果验证失败，函数将显示警告信息并返回。
 * 在所有检查通过后，函数调用 `deductCard` 函数进行消费操作，并在操作成功后显示消费结果。
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_consumeButton_clicked()
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

    if (ui->consumeCardIdBox->currentIndex() == -1)
    {
        QMessageBox::warning(this, "提示", "请放置卡片并点击查询按钮。");
        return;
    }
    QString cardId = ui->consumeCardIdBox->currentText();
    int userId;
    int cardStatus;
    double originalBalance;

    // 检查和获取绑定用户
    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select userId, `status`, balance from card "
                          "where id = :cardId;"));
    query.bindValue(":cardId", cardId);
    bool success = false;
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库异常。消费失败。"));
        return;
    }
    if (query.next())  // 卡已被注册，获取用户ID
    {
        cardStatus = query.value("status").toInt();
        if (cardStatus == -1)
        {
            QMessageBox::warning(this, QString("提示"), QString("此卡已被挂失。消费失败。"));
            return;
        }
        if (cardStatus == 0)
        {
            QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。消费失败。"));
            return;
        }
        userId = query.value("userId").toInt();
        originalBalance = query.value("balance").toDouble();
    }
    else    // 卡没有注册
    {
        query.finish();
        query.prepare(QString("insert into card "
                              "values (:cardId, 0, 0.0, null);"));
        query.bindValue(":cardId", cardId);
        success = query.exec();
        if (!success)
        {
            QMessageBox::warning(this, QString("提示"), QString("数据库异常。消费失败。"));
            return;
        }

        QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。消费失败。"));
        return;
    }


    double deductValue = ui->consumeValueBox->value();
    if (!(deductValue > 0.00))
    {
        QMessageBox::warning(this, "提示", "请输入大于0.00的消费金额。");
        return;
    }
    if (originalBalance - deductValue < 0.00)
    {
        QMessageBox::warning(this, "提示", "余额不足。");
        return;
    }
    if (deductValue > 300.00)
    {
        QMessageBox::warning(this, "提示", "消费金额不得超过300.00元。");
        return;
    }

    if (deductValue > 50.00)
    {
        QString prompt = "消费超过50.00元，需要密码验证。";
        QString info;
        success = verifyUser(userId, prompt, info);
        if (!success)
        {
            QMessageBox::warning(this, "提示", info + QString("\n用户验证不通过。消费失败。"));
            return;
        }
    }

    QString info;
    double finalBalance;
    QString recordId;
    success = deductCard(cardId, deductValue, originalBalance, finalBalance, recordId, info);
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n消费失败。"));
        return;
    }

    QString consumeResultMessage = QString("消费成功：") + QString::number(deductValue) + QString("元\n");
    consumeResultMessage += QString("原余额：") + QString::number(originalBalance) + QString("元\n");
    consumeResultMessage += QString("消费后余额：") + QString::number(finalBalance) + QString("元\n");
    consumeResultMessage += QString("消费设备：") + device.getName() + QString("\n");
    consumeResultMessage += QString("交易记录号：") + recordId + QString("\n");
    QMessageBox::information(this, "消费成功", consumeResultMessage);
    ui->consumeValueBox->setValue(0.00);
    return;
}


/**
 * @brief   卡消费
 * 该函数用于对指定卡片进行消费操作。消费前会进行一系列检查，包括卡片状态、余额是否足够以及单次消费额是否超过上限等。
 * 如果所有检查通过，将调用存储过程 `sp_consumeCard` 进行消费操作，并返回消费后的余额。
 * @param   cardId          要消费的卡号
 * @param   deductValue     消费金额
 * @param   originalBalance 消费前的余额，通过引用返回
 * @param   finalBalance    消费后的余额，通过引用返回
 * @param   recordId        交易记录号，通过引用返回
 * @param   info            如果出现异常，填入异常信息，通过引用返回
 * @return  bool            是否消费成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::deductCard(QString cardId, double deductValue, double &originalBalance, double &finalBalance, QString &recordId, QString &info)
{
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

    if (originalBalance - deductValue < 0.00)
    {
        info = "余额不足。";
        return false;
    }
    if (deductValue > 300.00)
    {
        info = "单次消费额上限为300.00元。";
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    recordId = getRecordId(currentTime, userId, 0);

    query.finish();
    query.prepare(QString("call sp_consumeCard (:cardId, :value, :recordId, :device, :time, :type, @newBalance);"));
    query.bindValue(":cardId", cardId);
    query.bindValue(":value", deductValue);
    query.bindValue(":recordId", recordId);
    query.bindValue(":device", device.getId());
    query.bindValue(":time", currentTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":type", 0);
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




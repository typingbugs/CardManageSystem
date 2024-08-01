#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到查询页面
 *  点击工具栏的“查询”触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::on_QueryAction_triggered()
{
    queryPageInitContent();
    ui->stackedWidget->setCurrentWidget(ui->queryPage);
}


/**
 * @brief   读卡器扫描卡片
 * 点击查询页面的“查询”触发。
 * 如果读卡器未连接，显示警告信息并跳转到设置页面。
 * 显示Inventory的查询结果，最多显示10张卡。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::on_queryInventoryButton_clicked()
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
    ui->queryCardIdBox->clear();
    if (cardIdList.empty())
    {
        QMessageBox::warning(this, "提示", "未发现卡片，请将卡片放置于读卡器上方。");
    }
    else
    {
        ui->queryCardIdBox->addItems(cardIdList);
    }
}


/**
 * @brief   用户选择卡片查询记录
 * 该函数在用户点击“查询”按钮时触发，用于根据选择的卡片ID查询该卡片的所有交易记录。
 * @param   void
 * @return  void
 * @details
 * 函数首先检查数据库是否已连接。如果数据库未连接，函数将显示警告信息并返回。
 * 接着，函数检查是否选择了卡片。如果未选择卡片，函数将显示警告信息并返回。
 * 然后，函数会查询卡片的绑定用户信息，包括用户ID、卡片状态和余额。如果卡片未被启用或已挂失，函数将显示相应的警告信息并返回。
 * 在查询卡片信息成功后，函数会要求进行用户身份验证。如果验证失败，函数将显示警告信息并返回，并重置查询页面。
 * 如果验证成功，函数会查询该卡片的所有交易记录，并将这些记录转换为 `QStringList` 后显示在界面的表格中。
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::on_userRecordQueryButton_clicked()
{
    if (!databaseReady())
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    if (ui->queryCardIdBox->currentIndex() == -1)
    {
        QMessageBox::warning(this, "提示", "请放置卡片并点击查询按钮。");
        return;
    }
    QString cardId = ui->queryCardIdBox->currentText();
    int userId;
    int cardStatus;
    double balance;

    // 检查和获取绑定用户
    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select userId, `status`, balance from card "
                          "where id = :cardId;"));
    query.bindValue(":cardId", cardId);
    bool success = false;
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
        queryPageInitContent();
        return;
    }
    if (query.next())  // 卡已被注册，获取用户ID
    {
        cardStatus = query.value("status").toInt();
        if (cardStatus == 0)
        {
            QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。"));
            queryPageInitContent();
            return;
        }
        if (cardStatus == -1)
        {
            ui->queryCardStatusLabel->setText(QString("已被挂失"));
        }
        if (cardStatus == 1)
        {
            ui->queryCardStatusLabel->setText(QString("启用中"));
        }

        userId = query.value("userId").toInt();
        balance = query.value("balance").toDouble();
        ui->queryBalanceShowEdit->setText(QString::number(balance, 'f', 2));
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
            QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
            queryPageInitContent();
            return;
        }

        QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。"));
        queryPageInitContent();
        return;
    }

    QString prompt = "查询用户所有交易记录需要验证用户身份。";
    QString info;
    success = verifyUser(userId, prompt, info);
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n用户验证不通过。查询失败。"));
        queryPageInitContent();
        return;
    }

    query.finish();
    query.prepare(QString("select time, type, value, balance, device, id from record_view "
                          "where cardId = :cardId;"));
    query.bindValue(":cardId", cardId);
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
        queryPageInitContent();
        return;
    }

    std::vector<QStringList> transactionRecordList;
    while (query.next())
    {
        QStringList transactionRecord = transactionRecord2QStringList
        (
            query.value("time").toDateTime(),
            query.value("type").toInt(),
            query.value("value").toDouble(),
            query.value("balance").toDouble(),
            query.value("device").toString(),
            query.value("id").toString()
        );
        transactionRecordList.push_back(transactionRecord);
    }

    displayInTableWidget(transactionRecordList);
}


/**
 * @brief   用户输入学/工号查询记录
 * 该函数在用户点击“查询”按钮时触发，用于根据输入的学/工号查询该用户的所有交易记录。
 * @param   void
 * @return  void
 * @details
 * 函数首先检查软件是否已准备好并且数据库已连接。如果数据库未连接，函数将显示警告信息并返回。
 * 接着，函数检查是否输入了学/工号。如果未输入学/工号，函数将显示警告信息并返回。
 * 然后，函数会验证用户身份，如果验证失败，函数将显示警告信息并返回，并重置查询页面。
 * 如果验证成功，函数会查询该学/工号绑定的卡片信息，包括卡片ID、状态和余额，并将余额和状态显示在界面上。
 * 接着，函数会查询该卡片的所有交易记录，并将这些记录转换为 `QStringList` 后显示在界面的表格中。
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::on_userIdRecordQueryButton_clicked()
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

    if (!queryUserIdFilled)
    {
        QMessageBox::warning(this, "提示", "请填写学/工号。");
        return;
    }
    int userId = ui->queryUserIdBox->value();

    bool success = false;
    QString prompt = "查询用户所有交易记录需要验证用户身份。";
    QString info;
    success = verifyUser(userId, prompt, info); // 内部会检查用户是否存在
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n查询失败。"));
        queryPageInitContent();
        return;
    }

    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select id, `status`, balance from card "
                          "where userId = :userId;"));
    query.bindValue(":userId", userId);
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, "提示", "数据库异常。查询失败。");
        queryPageInitContent();
        return;
    }
    if (!query.next())
    {
        QMessageBox::warning(this, "提示", "学/工号未绑定卡。查询失败。");
        queryPageInitContent();
        return;
    }
    QString cardId = query.value("id").toString();
    int cardStatus = query.value("status").toInt();
    double balance = query.value("balance").toDouble();
    ui->queryBalanceShowEdit->setText(QString::number(balance, 'f', 2));
    if (cardStatus == -1)
    {
        ui->queryCardStatusLabel->setText(QString("已被挂失"));
    }
    else if (cardStatus == 1)
    {
        ui->queryCardStatusLabel->setText(QString("启用中"));
    }

    query.finish();
    query.prepare(QString("select time, type, value, balance, device, id from record_view "
                          "where cardId = :cardId;"));
    query.bindValue(":cardId", cardId);
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
        queryPageInitContent();
        return;
    }

    std::vector<QStringList> transactionRecordList;
    while (query.next())
    {
        QStringList transactionRecord = transactionRecord2QStringList
            (
                query.value("time").toDateTime(),
                query.value("type").toInt(),
                query.value("value").toDouble(),
                query.value("balance").toDouble(),
                query.value("device").toString(),
                query.value("id").toString()
                );
        transactionRecordList.push_back(transactionRecord);
    }

    displayInTableWidget(transactionRecordList);
}


/**
 * @brief   查询卡内记录
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::on_cardRecordQueryButton_clicked()
{
    if (!databaseReady())
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库未连接，请设置。"));
        if (ui->stackedWidget->currentWidget() != ui->settingPage)
        {
            ui->stackedWidget->setCurrentWidget(ui->settingPage);
        }
        return;
    }

    if (ui->queryCardIdBox->currentIndex() == -1)
    {
        QMessageBox::warning(this, "提示", "请放置卡片并点击查询按钮。");
        return;
    }
    QString cardId = ui->queryCardIdBox->currentText();
    int cardStatus;
    double balance;

    // 检查和获取绑定用户
    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select userId, `status`, balance from card "
                          "where id = :cardId;"));
    query.bindValue(":cardId", cardId);
    bool success = false;
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
        queryPageInitContent();
        return;
    }
    if (query.next())  // 卡已被注册，获取用户ID
    {
        cardStatus = query.value("status").toInt();
        if (cardStatus == 0)
        {
            QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。"));
            queryPageInitContent();
            return;
        }
        if (cardStatus == -1)
        {
            ui->queryCardStatusLabel->setText(QString("已被挂失"));
        }
        if (cardStatus == 1)
        {
            ui->queryCardStatusLabel->setText(QString("启用中"));
        }
        balance = query.value("balance").toDouble();
        ui->queryBalanceShowEdit->setText(QString::number(balance, 'f', 2));
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
            QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
            queryPageInitContent();
            return;
        }

        QMessageBox::warning(this, QString("提示"), QString("此卡未被启用。"));
        queryPageInitContent();
        return;
    }

    QStringList cardRecordIdList = reader.readAllRecords(cardId, success);
    if (!success)
    {
        QMessageBox::warning(this, QString("提示"), QString("读卡器异常。查询失败。"));
        queryPageInitContent();
        return;
    }

    query.finish();
    query.prepare(QString("select time, type, value, balance, device, id from record_view "
                          "where cardId = :cardId and id = :recordId;"));
    query.bindValue(":cardId", cardId);


    std::vector<QStringList> transactionRecordList;
    for (int i = 0; i < cardRecordIdList.size(); i++)
    {
        QString recordId = cardRecordIdList[i];

        query.bindValue(":recordId", recordId);
        success = query.exec();
        if (!success)
        {
            QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
            queryPageInitContent();
            return;
        }
        if (!query.next()) {
            // 原来的代码
            // QMessageBox::warning(this, QString("提示"), QString("数据库异常。查询失败。"));
            // queryPageInitContent();
            // return;

            // 这里本来应该报错退出，但是读卡器经常发疯，见Reader::readAllRecords(QString cardId, bool &ok)
            // 所以发现有不在数据库中的记录号，直接跳过
            continue;
        }

        QStringList transactionRecord = transactionRecord2QStringList
        (
            query.value("time").toDateTime(),
            query.value("type").toInt(),
            query.value("value").toDouble(),
            query.value("balance").toDouble(),
            query.value("device").toString(),
            query.value("id").toString()
        );
        transactionRecordList.push_back(transactionRecord);
    }

    displayInTableWidget(transactionRecordList);
}


/**
 * @brief   将数据库查询返回的数据转化为QStringList
 * 该函数用于将数据库查询返回的交易记录数据转换为一个 `QStringList`，以便在界面上显示。
 * @param   time    交易时间，类型为 QDateTime
 * @param   type    交易类型，非零表示充值，零表示消费
 * @param   value   交易金额，类型为 double
 * @param   balance 交易后的余额，类型为 double
 * @param   device  交易设备名称，类型为 QString
 * @param   id      交易记录号，类型为 QString
 * @return  QStringList  转换后的交易记录列表
 * @details
 * 函数将输入的交易时间、交易类型、交易金额、余额、设备名称和记录号转换为一个QStringList。
 * 交易类型用QString表示，充值为“充值”，消费为“消费”。交易金额根据交易类型显示为正值或负值，保留两位小数。
 * @author  柯劲帆
 * @date    2024-07-31
 */
QStringList MainWindow::transactionRecord2QStringList(QDateTime time, int type, double value, double balance, QString device, QString id)
{
    QStringList stringList = {
        time.toString("yyyy-MM-dd hh:mm:ss"),
        type != 0 ? QString("充值") : QString("消费"),
        QString::number((type != 0 ? value : -value), 'f', 2),
        QString::number(balance, 'f', 2),
        device,
        id
    };

    return stringList;
}


/**
 * @brief   初始化查询页面的各个组件内容
 * 该函数用于初始化查询页面上的各个组件，包括清空用户ID输入框、清空查询结果表、清空余额显示框和重置卡片状态标签。
 * @param   void
 * @return  void
 * @details
 * 函数首先将 queryUserIdFilled 标志设置为false，表示尚未填入用户ID。
 * 然后，函数依次清空用户ID输入框、查询结果表和余额显示框，并将卡片状态标签设置为"-"。
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::queryPageInitContent()
{
    queryUserIdFilled = false;
    ui->queryUserIdBox->clear();
    ui->queryResultTable->clearContents();
    ui->queryBalanceShowEdit->clear();
    ui->queryCardStatusLabel->setText(QString("-"));
}


/**
 * @brief   在列表组件中显示内容
 * 该函数用于在 `QTableWidget` 中显示交易记录列表。交易记录包括充值和消费信息，并且根据交易类型进行颜色标注。
 * @param   transactionRecordList  包含交易记录的 `QStringList` 向量，每个 `QStringList` 表示一条记录
 * @return  void
 * @details
 * 函数首先清除 `QTableWidget` 中的所有内容，然后设置行数为 `transactionRecordList` 的大小。
 * 接着，函数遍历每条交易记录，并将其内容逐行逐列插入 `QTableWidget` 中。对于充值记录，文本颜色设置为绿色；
 * 对于消费记录，文本颜色设置为红色。所有单元格内容都右对齐。
 * @author  柯劲帆
 * @date    2024-07-31
 */
void MainWindow::displayInTableWidget(std::vector<QStringList> transactionRecordList)
{
    ui->queryResultTable->clearContents();

    int numRows = transactionRecordList.size();

    ui->queryResultTable->setRowCount(numRows);

    for (int i = 0; i < numRows; i++)
    {
        QStringList transactionRecord = transactionRecordList[i];
        bool is_deposit = transactionRecord[1] == QString("充值");
        for (int j = 0; j < 6; j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(transactionRecord[j]);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);  // 设置右对齐

            // 渲染：充值为绿色，消费为红色（不知为何不工作）
            if (is_deposit)
                item->setForeground(QBrush(QColor(Qt::green)));
            else
                item->setForeground(QBrush(QColor(Qt::red)));

            ui->queryResultTable->setItem(i, j, item);  // 设置内容
        }
    }
}

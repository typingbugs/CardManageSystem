#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到开卡页面
 *  点击工具栏的“开卡”触发，切换到开卡页面。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void MainWindow::on_NewCardAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->newCardPage);
}


/**
 * @brief   读卡器扫描卡片
 *  点击开卡页面的“查询”触发。
 *  如果读卡器未连接，显示警告信息并跳转到设置页面。
 *  显示Inventory的查询结果，最多显示10张卡。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-29
 */
void MainWindow::on_inventoryButton_clicked()
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
    ui->cardIdBox->clear();
    if (cardIdList.empty())
    {
        QMessageBox::warning(this, "提示", "未发现卡片，请将卡片放置于读卡器上方。");
    }
    else
    {
        ui->cardIdBox->addItems(cardIdList);
    }

}


/**
 * @brief   开卡
 * 如果数据库未连接，显示警告信息并跳转到设置页面。
 * 该函数在用户点击“确认”按钮时触发，用于处理新卡片的注册和绑定操作。
 * 函数首先检查是否选择了卡片和填写了用户ID，然后进行各种验证和数据库操作，确保卡片和用户的状态正确。
 * 如果所有操作成功，最终将新卡片和用户绑定。
 * @details
 * 如果数据库未连接，显示警告信息并跳转到设置页面。
 * 函数首先从用户界面中获取选择的卡片ID和填写的用户ID，并进行非空检查。
 * 如果未选择卡片或未填写用户ID，则弹出警告对话框并退出函数。
 * 然后，函数会查询数据库，验证卡片和用户的状态，并根据不同情况进行相应处理：
 * - 如果卡片已被启用，提示用户开卡失败。
 * - 如果卡片已挂失，需要用户输入密码进行验证和重开卡。
 * - 如果卡片未在数据库中，则将其初始化为未启用状态。
 * 函数还会检查用户是否为新用户，如果是新用户，则插入新用户记录并绑定卡片。
 * 如果用户已有卡片，则需要根据卡片状态决定是否允许开新卡或进行移资操作。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-29
 */
void MainWindow::on_newCardButton_clicked()
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

    if (ui->cardIdBox->currentIndex() == -1)
    {
        QMessageBox::warning(this, "提示", "请放置卡片并点击查询按钮。");
        return;
    }

    if (!newCardUserIdFilled)
    {
        QMessageBox::warning(this, "提示", "请填写学/工号。");
        return;
    }

    QString cardIdSelected = ui->cardIdBox->currentText();
    int userId = ui->userIdBox->value();

    // 查询卡片能否使用
    QSqlQuery query(db->getDatabase());
    query.finish();
    query.prepare(QString("select `status`, userId from card "
                  "where id = :cardId;"));
    query.bindValue(":cardId", cardIdSelected);
    bool success = query.exec();
    if (!success)
    {
        qDebug() << "select `status`, userId from card where id = :cardId;1";
        QMessageBox::warning(this, "提示", QString("数据库异常。开卡失败，请重试。"));
        return;
    }
    int cardStatus; ///< 0未启用；1已启用；-1已挂失
    if (query.next())
    {
        cardStatus = query.value("status").toInt();
        if (cardStatus == 1)    // 已被启用，不能开卡
        {
            QMessageBox::warning(this, "卡状态提示", "本卡已被启用，开卡失败。");
            return;
        }
        if (cardStatus == -1)   // 已被挂失，需要密码重开
        {
            // 弹出验证用户界面，要求用户输入密码（注意这里需要查询数据库得到挂失卡的用户）
            int cardUserId = query.value("userId").toInt();
            query.finish();
            query.prepare("select `name` from `user` "
                          "where id = :userId");
            query.bindValue(":userId", cardUserId);
            bool success = query.exec();
            if (!success || !query.next())
            {
                QMessageBox::warning(this, "提示", QString("数据库异常。\n重开卡失败，请重试。"));
                return;
            }
            QString cardUserName = query.value("name").toString();
            QString prompt = QString("本卡关联的学/工号为") + QString::number(cardUserId);
            prompt += QString("，关联的姓名为") + cardUserName + QString("。\n");
            prompt += QString("如需重开本卡，请输入密码。");
            QString info;
            success = verifyUser(cardUserId, prompt, info);
            if (!success)
            {
                QMessageBox::warning(this, "提示", info + QString("\n验证用户失败，请重试。"));
                return;
            }

            success = reopenCard(cardIdSelected, info);
            if (!success)
            {
                QMessageBox::warning(this, "提示", info + QString("\n重开卡失败，请重试。"));
                return;
            }

            return;
        }
    }
    else    // 该卡不在数据库中，将该卡在数据库中初始化，设置为未启用
    {
        query.finish();
        query.prepare("insert into card "
                      "values (:cardId, 0, 0.00, null);");
        query.bindValue(":cardId", cardIdSelected);
        query.exec();
    }

    // 检查是否是新用户
    query.finish();
    query.prepare("select 1 from `user` "
                  "where id = :userId;");
    query.bindValue(":userId", userId);
    success = query.exec();
    if (!success)
    {
        qDebug() << "select 1 from `user` where id = :userId;";
        QMessageBox::warning(this, "提示", QString("数据库异常。开卡失败，请重试。"));
        return;
    }
    if (!query.next())  // 库中无用户记录，将新用户插入数据库并开卡
    {
        QString info;
        bool success = createUser(userId, info);    // 将新用户插入数据库
        if (!success)
        {
            QMessageBox::warning(this, "提示", info + QString("\n注册失败，请重新开卡。"));
            return;
        }
        success = bindUserWithCard(userId, cardIdSelected, info);   // 将新用户和卡绑定（卡在读取时已写入数据库）
        if (!success)
        {
            QMessageBox::warning(this, "提示", info + QString("\n注册失败，请重新开卡。"));
            return;
        }
        QMessageBox::information(this, "提示", "新用户开卡成功。");
        return;
    }
    else    // 库中有用户记录
    {
        // 检查用户是否有卡，不可开卡/有挂失卡，是否需要移资
        query.finish();
        query.prepare("select `status`, id from card "
                      "where userId = :userId;");
        query.bindValue(":userId", userId);
        bool success = query.exec();
        if (!success)
        {
            qDebug() << "select `status`, id from card where userId = :userId;2";
            QMessageBox::warning(this, "提示", QString("数据库异常。开卡失败，请重试。"));
            return;
        }
        if (query.next())  // 用户有卡
        {
            int userCardStatus = query.value("status").toInt();
            if (userCardStatus == 1)    // 用户已启用卡
            {
                QMessageBox::warning(this, "提示", "该用户已有正在使用的卡，开卡失败。\n如需开新卡，请挂失正在使用的卡。");
                return;
            }
            else if (userCardStatus == -1)  // 用户有挂失卡，需要移资
            {
                /// @todo 弹出验证用户界面，要求用户输入密码；将挂失卡的信息和消费记录移到新卡
                QString info, prompt = QString("如需将挂失卡移资到本卡，请输入密码。");
                bool success = verifyUser(userId, prompt, info);
                if (!success)
                {
                    QMessageBox::warning(this, "提示", info + QString("\n验证用户失败。移资失败，请重试。"));
                    return;
                }

                QString userCardId = query.value("id").toString();
                success = transferCard(userId, cardIdSelected, userCardId, info);
                if (!success)
                {
                    QMessageBox::warning(this, "提示", info + QString("\n移资失败，请重试。"));
                    return;
                }

                QMessageBox::information(this, "提示", "移资成功。");
                return;
            }
        }
        else    // 用户无卡
        {
            QString info;
            bool success = bindUserWithCard(userId, cardIdSelected, info);   // 将新用户和卡绑定（卡在读取时已写入数据库）
            if (!success)
            {
                QMessageBox::warning(this, "提示", info + QString("\n注册失败，请重新开卡。"));
                return;
            }
            QMessageBox::information(this, "提示", "新用户开卡成功。");
            return;
        }
    }
}


/**
 * @brief   绑定用户和卡号
 * 该函数用于在数据库中绑定用户和卡号。函数将更新卡片的状态、余额和用户ID。
 * 如果操作失败，函数将返回失败并设置错误信息。
 * @details
 * 函数首先创建一个QSqlQuery对象，并准备一个更新卡片信息的SQL语句，将卡片的状态设置为启用（1），余额设置为0，
 * 并将用户ID绑定到卡片ID。如果SQL执行失败，函数会将错误信息写入info并返回false。
 * @param   userId  要绑定的用户的学/工号
 * @param   cardId  要绑定的卡号
 * @param   info    如果出现异常，填入异常信息
 * @return  是否绑定成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::bindUserWithCard(int userId, QString cardId, QString &info)
{
    QSqlQuery query(db->getDatabase());
    query.finish();
    query.prepare(QString("update card "
                          "set `status` = :status, balance = :balance, userId = :userId "
                          "where id = :cardId;"));
    query.bindValue(":status", 1);
    query.bindValue(":balance", (double)0.00);
    query.bindValue(":userId", userId);
    query.bindValue(":cardId", cardId);
    bool updateCardIsExecuted = query.exec();
    if (!updateCardIsExecuted)
    {
        info = QString("数据库异常。");
        return false;
    }

    /// @todo 写卡

    return true;
}


/**
 * @brief   在数据库中创建新用户
 * 该函数用于在数据库中创建新用户。首先，它调用 `getNewUserInfo` 函数获取新用户的姓名和密码。
 * 然后，将这些信息与用户ID一起插入到数据库中。如果任何步骤失败，函数将返回失败并设置错误信息。
 * @details
 * 函数首先调用 `getNewUserInfo` 获取新用户的姓名和密码。如果获取信息失败，函数立即返回失败并设置错误信息。
 * 然后，函数准备并执行一个插入新用户信息的SQL语句，将用户ID、姓名和密码插入到数据库中的 `user` 表。
 * 如果SQL执行失败，函数返回失败并设置错误信息。
 * @param   userId  要创建的用户的学/工号
 * @param   info    如果出现异常，填入异常信息
 * @return  bool    是否创建成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::createUser(int userId, QString &info)
{
    QString username, password;
    if (!getNewUserInfo(username, password, info)) return false;

    QSqlQuery query(db->getDatabase());
    query.finish();
    query.prepare(QString("insert into `user` "
                          "values (:id, :name, :password);"));
    query.bindValue(":id", userId);
    query.bindValue(":name", username);
    query.bindValue(":password", password);
    bool insertUserIsExecuted = query.exec();
    if (!insertUserIsExecuted)
    {
        info = QString("数据库异常。");
        return false;
    }

    return true;
}


/**
 * @brief   弹出交互窗口获取新用户的信息
 * 该函数弹出多个输入对话框，依次获取新用户的姓名和密码，并进行简单验证。
 * 如果用户在任一步骤取消输入或输入无效，函数将返回失败并设置错误信息。
 * @details
 * 函数依次弹出三个输入对话框，获取新用户的姓名、密码和确认密码。
 * - 如果用户取消输入或输入为空，函数返回失败并设置错误信息。
 * - 如果两次输入的密码不一致，函数返回失败并设置错误信息。
 * - 如果所有输入均有效，函数返回成功。
 * @param   username    获取的新用户的姓名
 * @param   password    获取的新用户的密码
 * @param   info    如果出现异常，填入异常信息
 * @return  bool    是否获取成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::getNewUserInfo(QString &username, QString &password, QString &info)
{
    bool ok = false;
    username = QInputDialog::getText(this, tr("新用户注册"), tr("请输入姓名"), QLineEdit::Normal, 0, &ok);
    if (!ok || username.isEmpty())
    {
        info = QString("输入信息异常。");
        return false;
    }
    ok = false;
    password = QInputDialog::getText(this, tr("新用户注册"), tr("请输入密码"), QLineEdit::Password, 0, &ok);
    if (!ok || password.isEmpty())
    {
        info = QString("输入信息异常。");
        return false;
    }
    ok = false;
    QString confirmPassword = QInputDialog::getText(this, tr("新用户注册"), tr("请再次输入密码"), QLineEdit::Password, 0, &ok);
    if (!ok || confirmPassword.isEmpty())
    {
        info = QString("输入信息异常。");
        return false;
    }
    if (confirmPassword != password)
    {
        info = QString("两次输入的密码不一致。");
        return false;
    }

    return true;
}


/**
 * @brief   弹出交互窗口认证用户，在数据库中比对
 * 该函数用于弹出一个窗口要求用户输入密码，并在数据库中比对用户信息以验证身份。
 * 如果用户输入正确的密码，则认证成功，否则认证失败。
 * @details
 * 函数首先弹出一个输入对话框，要求用户输入密码。如果用户取消输入或未输入密码，函数返回认证失败并设置错误信息。
 * 然后，函数在数据库中查询对应的用户ID和密码，并与用户输入的密码进行比对。如果密码正确，则返回认证成功。
 * 否则，函数返回认证失败并设置相应的错误信息。
 * @param   userId  要认证的用户学/工号
 * @param   prompt  在交互窗口的提示文字
 * @param   info    如果出现异常，填入异常信息
 * @return  bool    是否认证成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::verifyUser(int userId, QString prompt, QString &info)
{
    bool ok = false;
    QString text;
    if (prompt.isEmpty()) text = QString("请输入密码");
    else text = prompt + QString("\n请输入密码");
    QString password = QInputDialog::getText(this, tr("验证用户"), text, QLineEdit::Password, 0, &ok);
    if (!ok || password.isEmpty())
    {
        info = QString("输入信息异常。");
        return false;
    }

    QSqlQuery query(db->getDatabase());
    query.finish();
    query.prepare(QString("select `password` from `user` "
                          "where id = :id;"));
    query.bindValue(":id", userId);
    if (!query.exec())
    {
        info = QString("数据库异常。");
        return false;
    }
    if (query.next())
    {
        QString validPassword = query.value(0).toString();
        if (validPassword != password)
        {
            info = QString("密码不正确。");
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        info = QString("学/工号不存在。");
        return false;
    }
}


/**
 * @brief   移资，将旧卡信息和记录移到新卡，删除旧卡信息
 * 该函数用于将旧卡的余额和相关记录移到新卡，并删除旧卡的信息。函数执行以下步骤：
 * - 查询旧卡的余额，并将其转移到新卡。
 * - 更新新卡的状态、余额和用户ID。
 * - 更新所有记录，将旧卡ID替换为新卡ID。
 * - 删除旧卡的信息
 * @details
 * 函数首先查询旧卡的余额，并在执行过程中进行多次数据库操作，以确保所有相关信息和记录都正确地从旧卡移到新卡。
 * 如果任何操作失败，函数会将错误信息写入info并返回false。
 * @param   userId      新旧卡所属的用户的学/工号
 * @param   newCardId   新卡的ID
 * @param   oldCardId   旧卡的ID
 * @param   info    如果出现异常，填入异常信息
 * @return  bool    是否移资成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::transferCard(int userId, QString newCardId, QString oldCardId, QString &info)
{
    QSqlQuery query(db->getDatabase());
    bool isExecuted = false;

    // 查询旧卡余额
    query.finish();
    query.prepare("select balance from card "
                  "where userId = :userId;");
    query.bindValue(":userId", oldCardId);
    isExecuted = query.exec();
    if (!isExecuted || query.next())
    {
        info = QString("数据库异常。");
        return false;
    }
    double balance = query.value("balance").toDouble();

    // 更新新卡信息
    query.finish();
    query.prepare(QString("update card "
                          "set `status` = :status, balance = :balance, userId = :userId "
                          "where id = :cardId;"));
    query.bindValue(":status", 1);
    query.bindValue(":balance", balance);
    query.bindValue(":userId", userId);
    query.bindValue(":cardId", newCardId);
    isExecuted = query.exec();
    if (!isExecuted)
    {
        info = QString("数据库异常。");
        return false;
    }

    // 更新所有记录，将旧卡ID替换为新卡ID
    query.finish();
    query.prepare(QString("update record "
                          "set cardId = :newCardId "
                          "where cardId = :oldCardId;"));
    query.bindValue(":newCardId", newCardId);
    query.bindValue(":oldCardId", oldCardId);
    isExecuted = query.exec();
    if (!isExecuted)
    {
        info = QString("数据库异常。");
        return false;
    }

    // 删除旧卡信息
    query.finish();
    query.prepare(QString("delete from card "
                          "where id = :cardId;"));
    query.bindValue(":cardId", oldCardId);
    isExecuted = query.exec();
    if (!isExecuted)
    {
        info = QString("数据库异常。");
        return false;
    }

    /// @todo 将数据库上的记录写到新卡上

    return true;
}


/**
 * @brief   重开卡
 * 该函数用于重新启用一张已挂失的卡片。卡片信息和记录都保留在数据库中，只需将卡片的状态从-1（挂失）更改为1（启用）。
 * @details
 * 函数首先创建一个QSqlQuery对象，并准备一个更新卡片状态的SQL语句，将指定卡片的状态更改为启用状态（1）。
 * 如果SQL执行失败，函数会将错误信息写入info并返回false。
 * @param   userId  卡所属的用户的学/工号
 * @param   info    如果出现异常，填入异常信息
 * @return  bool    是否重开卡成功
 * - true   成功
 * - false  失败
 * @author  柯劲帆
 * @date    2024-07-30
 */
bool MainWindow::reopenCard(QString cardId, QString &info)
{
    // 卡信息和记录都还在数据库中，只是status为-1，更改即可
    QSqlQuery query(db->getDatabase());
    query.finish();
    query.prepare("update card "
                  "set `status` = 1 "
                  "where id = :cardId");
    query.bindValue(":cardId", cardId);
    bool success = query.exec();
    if (!success)
    {
        info = QString("数据库异常。");
        return false;
    }

    /// @todo 看看是否有写卡需求

    return true;
}

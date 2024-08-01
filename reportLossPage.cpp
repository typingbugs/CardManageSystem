#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到挂失页面
 *  点击工具栏的“挂失”触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void MainWindow::on_reportLossAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->reportLossPage);
}


/**
 * @brief   挂失卡
 * 该函数用于将指定用户的卡片设置为挂失状态。当用户点击“挂失”按钮时触发。
 * @details
 * 函数首先检查数据库连接状态，如果数据库未连接，显示警告信息并跳转到设置页面。
 * 然后检查是否填写了用户的学/工号。如果未填写，显示警告信息并返回。
 * 然后，函数在数据库中查询该学/工号是否存在。如果不存在，显示警告信息并返回。
 * 如果学/工号存在，函数将该用户的卡片状态设置为挂失（`status` = -1），并在操作成功后显示挂失成功的信息。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-30
 */
void MainWindow::on_reportLossButton_clicked()
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

    if (!reportLossUserIdFilled)
    {
        QMessageBox::warning(this, "提示", "请填写学/工号。");
        return;
    }
    int userId = ui->reportLossUserIdBox->value();

    // 查询学/工号是否存在
    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select id from card "
                          "where userId = :userId;"));
    query.bindValue(":userId", userId);
    bool success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, "提示", "数据库异常，挂失失败。");
        return;
    }
    if (!query.next())
    {
        QMessageBox::warning(this, "提示", "学/工号不存在，挂失失败。");
        return;
    }

    QString info, prompt = QString("如需挂失该学/工号绑定的卡，请输入密码。");
    success = verifyUser(userId, prompt, info);
    if (!success)
    {
        QMessageBox::warning(this, "提示", info + QString("\n验证用户失败。挂失失败，请重试。"));
        return;
    }

    // 将该学/工号的卡设置为挂失状态
    query.prepare(QString("update card "
                          "set `status` = -1 "
                          "where userId = :userId;"));
    query.bindValue(":userId", userId);
    success = query.exec();
    if (!success)
    {
        QMessageBox::warning(this, "提示", "数据库异常，挂失失败。");
        return;
    }

    QMessageBox::information(this, "提示", "挂失成功。");
    return;
}

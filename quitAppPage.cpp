#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief   切换到退出页面
 *  点击工具栏的“退出”触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::on_quitAppAction_triggered()
{
    ui->stackedWidget->setCurrentWidget(ui->quitAppPage);
}


/**
 * @brief   退出程序
 *  在退出页面点击“确认”按钮触发
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void MainWindow::on_confirmQuitButton_clicked()
{
    this->close();
}

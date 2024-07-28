#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <readerAPI.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateStatusBarComNumber();
    bool ready();

private slots:
    void on_settingAction_triggered();
    void on_quitAppAction_triggered();
    void on_NewCardAction_triggered();

    void on_connectReaderButton_clicked();
    void on_confirmQuitButton_clicked();
    void on_connectDatabaseButton_clicked();
    void on_inventoryButton_clicked();



private:
    Ui::MainWindow *ui;
    Reader reader;
    QSqlDatabase db;
    QString databaseIpAddr = "";
    int databasePort = 3306;
    QString databasePassword = "";
    QString device = QString("未指定设备名");
    bool depositAllowed = false;

    QStatusBar *statusBar;
    QStackedWidget *stackedWidget;

    QCheckBox *readerConnectStatusCheckBox;
    QLabel *comNumberLabel;
    QCheckBox *databaseConnectStatusCheckBox;
    QLabel *databaseLabel;
    QLabel *deviceLabel;
};
#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSqlQuery>

#include <readerAPI.h>
#include <databaseAPI.h>
#include <deviceAPI.h>

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
    Database *db = nullptr;
    Device device;

    QStatusBar *statusBar;
    QStackedWidget *stackedWidget;

    QCheckBox *readerConnectStatusCheckBox;
    QLabel *comNumberLabel;
    QCheckBox *databaseConnectStatusCheckBox;
    QLabel *databaseLabel;
    QLabel *deviceLabel;
};
#endif // MAINWINDOW_H

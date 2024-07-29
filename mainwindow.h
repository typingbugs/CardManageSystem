#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSqlQuery>
#include <QInputDialog>

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
    bool bindUserWithCard(int userId, QString cardId, QString &info);
    bool getNewUserInfo(QString &username, QString &password, QString &info);
    bool createUser(int userId, QString &info);
    bool verifyUser(int userId, QString prompt, QString &info);
    bool transferCard(int userId, QString newCardId, QString oldCardId, QString &info);
    bool reopenCard(QString cardId, QString &info);

private slots:
    void on_settingAction_triggered();
    void on_quitAppAction_triggered();
    void on_NewCardAction_triggered();

    void on_connectReaderButton_clicked();
    void on_confirmQuitButton_clicked();
    void on_connectDatabaseButton_clicked();
    void on_inventoryButton_clicked();
    void on_newCardButton_clicked();

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

    bool userIdFilled;  ///< 初始时学/工号填写框被清空，该变量为false；当用户填写后该变量为true
};
#endif // MAINWINDOW_H

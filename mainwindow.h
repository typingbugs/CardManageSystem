#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSqlQuery>
#include <QInputDialog>
#include <QDateTime>
#include <QRandomGenerator>

#include <vector>

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
    bool allReady();
    bool softwareReady();
    bool databaseReady();
    bool bindUserWithCard(int userId, QString cardId, QString &info);
    bool getNewUserInfo(QString &username, QString &password, QString &info);
    bool createUser(int userId, QString &info);
    bool verifyUser(int userId, QString prompt, QString &info);
    bool transferCard(int userId, QString newCardId, QString oldCardId, QString &info);
    bool reopenCard(QString cardId, QString &info);
    bool topUpCard(QString cardId, double topUpValue, double &originalBalance, double &finalBalance, QString &recordId, QString &info);
    QString getRecordId(QDateTime currentTime, int userId, int recordType);
    bool deductCard(QString cardId, double deductValue, double &originalBalance, double &finalBalance, QString &recordId, QString &info);
    void displayInTableWidget(std::vector<QStringList> transactionRecordList);
    QStringList transactionRecord2QStringList(QDateTime time, int type, double value, double balance, QString device, QString id);
    void queryPageInitContent();

private slots:
    void on_settingAction_triggered();
    void on_quitAppAction_triggered();
    void on_NewCardAction_triggered();
    void on_reportLossAction_triggered();
    void on_depositAction_triggered();
    void on_ConsumptionAction_triggered();
    void on_QueryAction_triggered();

    void on_connectReaderButton_clicked();
    void on_confirmQuitButton_clicked();
    void on_connectDatabaseButton_clicked();
    void on_inventoryButton_clicked();
    void on_newCardButton_clicked();
    void on_reportLossButton_clicked();
    void on_depositInventoryButton_clicked();
    void on_depositByCardIdButton_clicked();
    void on_depositByUserIdButton_clicked();
    void on_consumeInventoryButton_clicked();
    void on_consumeButton_clicked();
    void on_queryInventoryButton_clicked();
    void on_userRecordQueryButton_clicked();
    void on_userIdRecordQueryButton_clicked();

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

    bool newCardUserIdFilled;  ///< 开卡时：初始时学/工号填写框被清空，该变量为false；当用户填写后该变量为true
    bool reportLossUserIdFilled;  ///< 挂失时：初始时学/工号填写框被清空，该变量为false；当用户填写后该变量为true
    bool depositUserIdFilled;  ///< 充值时：初始时学/工号填写框被清空，该变量为false；当用户填写后该变量为true
    bool queryUserIdFilled;  ///< 查询时：初始时学/工号填写框被清空，该变量为false；当用户填写后该变量为true
};

#endif // MAINWINDOW_H

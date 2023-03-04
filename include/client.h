#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QtSql/QSqlDatabase> //Бд
//#include <QtSql/QSqlError> //Обработка ошибок
#include <QtSql/QSqlQuery> //Запросы
#include <QMessageBox>
#include <QSqlTableModel>
#include <QTableWidget>
#include <QAbstractItemView>
#include <QButtonGroup>
#include <QSpinBox>

namespace Ui {
class client;
}



class client : public QWidget
{
    Q_OBJECT

public:
    ///Конструктор
    explicit client(QWidget *parent = nullptr,int cliId=-1);
    void setClientLabel(const int& client_reg_id);

    ///Деструктор
    ~client();

private slots:
    ///Обновление таблиц
    void updateTables();
    ///Сохранение изменений в профиле
    void on_pushButton_save_clicked();
    ///Создание заказа
    void on_createOrder_clicked();
    ///Переход на вторую страницу заказа
    void on_pushButton_next_clicked();
    ///Переход на первую страницу заказа
    void on_pushButton_nazad_clicked();
    ///Выход из аккаунта
    void on_pushButton_exit_clicked();
    ///Очистка таблиц
    void clear_tables();

private:
    Ui::client *ui;
    QSqlDatabase * db;
    QSqlTableModel*model;
    QSqlTableModel*model1;

    QList<QCheckBox*> checkBoxList;
    QList<QCheckBox*> checkBoxList2;

    QList<int> dish_checked;
    QList<int> rest_checked;

    bool checkBoxChecked  = false;
    bool checkBoxChecked2  = false;

    int clientId=-1;
    int reg_clientId=-1;
    int currentAutorization=-1;

    int rest_id;
    int dish_id;
    int dish;
    int spinBoxValue;
};

#endif // CLIENT_H

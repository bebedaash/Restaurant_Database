#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QWidget>
#include <QMessageBox>
#include <iostream>
#include <QtSql/QSqlDatabase> //Бд
//#include <QtSql/QSqlError> //Обработка ошибок
#include <QtSql/QSqlQuery> //Запросы

namespace Ui {
class registration;
}

class registration : public QWidget
{
    Q_OBJECT

public:
    ///Конструктор
    explicit registration(QWidget *parent = nullptr);
    ///Деструктор
    ~registration();

private slots:
    void openWindow();
    void on_login_currentIndexChanged(int index);
    void on_pushButton_back_clicked();
    void on_pushButton_reg_clicked();
    void connection();

private:
    Ui::registration *ui;
    QSqlDatabase * db;
    int reg_id;
    int client_reg_id;
};

#endif // REGISTRATION_H

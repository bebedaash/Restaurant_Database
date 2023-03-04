#ifndef ADMIN_H
#define ADMIN_H

#include <QWidget>
#include <QtSql/QSqlDatabase> //БД
//#include <QtSql/QSqlError> //Обработка ошибок
#include <QtSql/QSqlQuery> //Запросы
#include <QMessageBox>
#include <QSqlTableModel> //Модель
#include <QTableWidget>
#include <QAbstractItemView>

namespace Ui {
class admin;
}
///Окно администратора
class admin : public QWidget
{
    Q_OBJECT

public:
    ///Конструктор
    explicit admin(QWidget *parent = nullptr,int id=-1,QString login_r="");
    ///Функция с id зарегистрированного клиента
    void setLabel(const int& reg_id);
    ///Деструктор
    ~admin();

private slots:
    ///Добавление нового блюда
    void on_insertDish_clicked();

    ///Сохранение изменений в меню
    void on_pushButton_save_clicked();

    ///Обновление таблиц
    void updateTables();

    ///Подключение в БД
    void connection();

    ///Сохранение изменений в инфо
    void on_pushButton_save_info_clicked();

    ///Удаление строки
    void on_pushButton_delete_clicked();

    ///Выбор индекса
    void on_menuView_clicked(const QModelIndex &index);

    ///Выход из аккаунта
    void on_pushButton_exit_clicked();

private:
    Ui::admin *ui;

    QSqlDatabase * db; //БД
    QSqlTableModel*model; //Модель с инфо о ресторане
    QSqlTableModel* model1; //Модель с инфо о блюдах

    QString loginr;
    int Id;
    int registr_id=-1;
    int currentRow=-1;
    int row_count;
    int currentAutorization=-1; //Переменная с инфо по способу авторизации: регистрация или вход
};
#endif // ADMIN_H

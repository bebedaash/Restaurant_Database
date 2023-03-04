#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase> //Бд
//#include <QtSql/QSqlError> //Обработка ошибок
#include <QtSql/QSqlQuery> //Запросы
#include <QMessageBox>
#include "checkid.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ///Конструктор
    explicit MainWindow(QWidget *parent = nullptr);

    ///Деструктор
    ~MainWindow();


private slots:
//    void on_type_currentIndexChanged(int index);
    void on_connect_clicked();
    void connectDB();
    void on_connect_2_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase * db;
    QSqlDatabase * db2;
    void openWindow();
    int Id;
    QString namer,loginr;

    int clientId;
};

#endif // MAINWINDOW_H

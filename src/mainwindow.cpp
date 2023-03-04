#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "admin.h"
#include "client.h"
#include "order.h"
#include "registration.h"
#include <QCryptographicHash>

///Конструктор
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    db = nullptr;
}
///Деструктор
MainWindow::~MainWindow()
{
    delete ui;
    db->close();
    delete db;
}
///Открытие окна в зависимости от аккаунта
void MainWindow::openWindow()
{
    switch (ui->type->currentIndex()){
    case 0://0-ресторан,1-юзер
    {
        admin* ad = new admin(nullptr,Id,loginr);
        ad->show();
        MainWindow::close();

        return;
    }
    case 1:
    {
        client* cl = new client(nullptr, clientId);
        cl->show();
        MainWindow::close();

        return;
    }
    }
}
///Подключение к БД
void MainWindow::connectDB()
{
    if (db!=nullptr) //all comm
    {
        db->close();
        delete db;
        db = nullptr;
        return;
    }
    if(db == nullptr) {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL"));
    } else {
        return;
    }
    if(db->isOpen())
    {
        throw "Already opened";
    }
    QString dbName("postgres"); //db name
    QString host("127.0.0.1"); //server address
    QString usr("postgres"); //login
    QString dbPasw("123"); //[assword
    db->setDatabaseName(dbName);
    db->setHostName(host);
    db->setUserName(usr);
    db->setPassword(dbPasw);
    db->setPort(5432);
    if (!db->open())
    {
        QMessageBox::warning(this,"error",db->lastError().text());
        delete  db;
        db=nullptr;
    }
}

///Обработка нажатия на кнопку "Войти"
void MainWindow::on_connect_clicked()
{
    connectDB();
    QString psw= ui->password->text().trimmed();
    QString login = ui->login->text().trimmed();
    QString label_login=ui->label_login->text();
    QString label_passw=ui->label_passw->text();
    if (login.isEmpty()){
        QMessageBox::information(this, "Инфо", "Поле "+label_login+" не должно быть пустым ");
        return;
    }
    if (psw.isEmpty()){
        QMessageBox::information(this, "Инфо", "Поле "+label_passw+" не должно быть пустым ");
        return;
    }
    /// Хэширование пароля с помощью QCryptographicHash
    psw = QString(QCryptographicHash::hash(psw.toUtf8(),QCryptographicHash::Md5).toHex());

    if (db != nullptr && db->isOpen())
    {
        switch (ui->type->currentIndex()){
        case 0: //Авторизация администратора
        {
            QSqlQuery QueryOnline;
            QueryOnline.exec("select a.id, a.login from rtdm.rest_login a "
                             "where a.login='"+login+"' and a.pass='"+psw+"'");
            if (QueryOnline.next())
            {
                Id = QueryOnline.value(0).toInt();
                QMessageBox::information(this,"Информация","Добро пожаловать");// авторизация прошла
                openWindow();
            }
            else {
                QMessageBox::warning(this,"Ошибка","Авторизация не прошла");
                ui->password->clear();
            }
            return;
        }
        case 1: //Авторизация пользователя
        {
            QSqlQuery QueryOn;
            QueryOn.exec("select id from rtdm.clients "
                         "where phone='"+login+"' and password='"+psw+"'");
            if (QueryOn.next()) //Запрос выполняется, авторизация прошла
            {
                clientId = QueryOn.value(0).toInt();
                QMessageBox::information(this,"Информация","Добро пожаловать");
                openWindow();
            }
            else {
                QMessageBox::warning(this,"Ошибка","Авторизация не прошла");
                ui->password->clear();
            }
            return;
        }
        }
    }
    else  //При ошибке авторизации выводим сообщение
    {
        QMessageBox::warning(this,"Ошибка","Авторизация не прошла");
        return;
    }
}

///Обработка нажатия на кнопку "Регистрация"
void MainWindow::on_connect_2_clicked()
{
    registration* reg = new registration();
    reg->show();
    MainWindow::close();
}

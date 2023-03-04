#include "registration.h"
#include "ui_registration.h"
#include "admin.h"
#include "client.h"
#include <QCryptographicHash>
#include "mainwindow.h"
///Конструктор
registration::registration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::registration)
{
    ui->setupUi(this);
    db = nullptr;
}
///Деструктор
registration::~registration()
{
    delete ui;
    db->close();
    delete db;
}

void registration::on_login_currentIndexChanged(int index)
{
    if (index == 0) {// Регистрация пользователя
        ui->label_name->setText("Название ресторана");
        ui->label_login->setText("Имя пользователя");
    }
    else {
        ui->label_name->setText("ФИО");
    }
}
void registration::openWindow()
{
    switch (ui->login->currentIndex()){
    case 0:
    {
        admin* ad = new admin();
        ad->setLabel(reg_id);
        ad->show();
        registration::close();
        return;
    }
    case 1:
    {
        client* cl = new client();
        cl->setClientLabel(client_reg_id);
        cl->show();
        registration::close();
        return;
    }
    }
}

void registration::on_pushButton_back_clicked()
{
    MainWindow* mw = new MainWindow();
    mw->show();
    registration::close();
    return;
}
void registration::connection(){
    if (db != nullptr)
    {
        db->close();
        delete db;
        db = nullptr;
        return;
    }
    else {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL"));
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
//        QMessageBox::warning(this,"error",db->lastError().text());
        delete  db;
        db=nullptr;
    }
}
void registration::on_pushButton_reg_clicked()
{
    connection();
    if (db != nullptr && db->isOpen())
    {
        QString name= ui->lineEdit_name->text().trimmed();
        QString login= ui->lineEdit_phone_login->text().trimmed();
        QString label_name=ui->label_name->text();
        QString password = ui->lineEdit_password->text().trimmed();
        QString password_check = ui->lineEdit_password_2->text().trimmed();
        QString label_login=ui->label_login->text();

        QRegExp rool("[^0-9а-яА-ЯёЁa-zA-Z]");
        QValidator *validator = new QRegExpValidator(rool, this);
        ui->login->setValidator( validator );
        if (login.contains(rool)) {
            QMessageBox::information(this, "Инфо", "Поле "+label_login+" не должно содержать спецсимволов");
            return;            }
        if (name.size()>100){
            QMessageBox::information(this, "Инфо", "Поле "+label_name+" должно быть не более 100 символов");
            return;
        }
        if (name<5){
            QMessageBox::information(this, "Инфо", "Длина поля "+label_name+" должна быть более 5 символов ");
            return;
        }
        if (login.isEmpty()){
            QMessageBox::information(this, "Инфо", "Поле "+label_login+" не должно быть пустым ");
            return;
        }

        if (password.length()<5) {
            QMessageBox::information(this, "Инфо", "Длина пароля должна быть более 5 символов");
            return;
        }
        if (password_check != password) {
            QMessageBox::information(this, "Инфо", "Пароль не совпадают");
            return;
        }
        if (QMessageBox::question(this, "Подтверждение", "Вы подтверждаете регистрацию?",
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
        /// Хэширование пароля с помощью QCryptographicHash
        password = QString(QCryptographicHash::hash(password.toUtf8(),QCryptographicHash::Md5).toHex());
        switch (ui->login->currentIndex()){
        case 0: //Выбран администратор
        {
            //Сообщения об ошибках
            if (login.size()>100){
                QMessageBox::information(this, "Инфо", "Поле "+label_name+" должно быть не более 100 символов");
                return;
            }
            if (login<5){
                QMessageBox::information(this, "Инфо", "Длина поля "+label_name+"должна быть более 5 символов ");
                return;
            }
            QSqlQuery QueryCheck;
            QueryCheck.exec("SELECT login FROM rtdm.rest_login WHERE login='"+login+"'");
            if (QueryCheck.next()) //Если запрос выполняется
            {
                QString loginDB = QueryCheck.value(0).toString().trimmed();
                if(loginDB==login){
                    QString label_login=ui->label_login->text();
                    QMessageBox::warning(this,"Ошибка","Поле "+label_login+" должно быть уникальным");
                    return;
                }
            }
            else
            {
                QSqlQuery queryInsert("INSERT INTO rtdm.rest(namer,login) "
                                      "VALUES ('"+name+"', '"+login+"')RETURNING id");
                queryInsert.first();
                reg_id = queryInsert.value(0).toInt();
                QSqlQuery queryIns;
                queryIns.exec("INSERT INTO rtdm.rest_login(id,login,pass) "
                              "VALUES ("+QString::number(reg_id)+",'"+login+"', '"+password+"');");
                QMessageBox::information(this, "Регистрация", "Вы зарегистрированы");
                openWindow();
                return;
            }
            return;
        }
        case 1://Выбран клиент
        {
            QSqlQuery QueryCheck;
            QueryCheck.exec("SELECT phone FROM rtdm.clients WHERE phone='"+login+"'"); //Получение инфо о логине
            if (QueryCheck.next()) //Если запрос выполняется
            {
                QString phoneDB = QueryCheck.value(0).toString().trimmed();
                if(phoneDB==login)
                {
                    QMessageBox::warning(this,"Ошибка","Поле "+label_login+" должно быть уникальным");
                    return;
                }
            }
            else
            {
                QSqlQuery queryInsert("INSERT INTO rtdm.clients_inf(name) " //Запрос на добавление инфо о клиенте
                                      "VALUES ('"+name+"')RETURNING id");
                queryInsert.first();
                client_reg_id = queryInsert.value(0).toInt();
                QSqlQuery queryIns; //Запрос на добавление данных авторизации клиента
               bool success =  queryIns.exec("INSERT INTO rtdm.clients(id,phone,password) "
                              "VALUES ("+QString::number(client_reg_id)+",'"+login+"', '"+password+"');");
               if(!success) {
                QMessageBox::warning(this, "Ошибка при регистрации","Произошла неизвестная ошибка");
                return;
               }
                   QMessageBox::information(this, "Регистрация", "Вы зарегистрированы");
                openWindow();
                return;
            }
            return;
        }
        }
        ui->lineEdit_name->clear(); //Очистка полей
        ui->lineEdit_phone_login->clear();
        ui->lineEdit_password->clear();
        ui->lineEdit_password_2->clear();
    }
    else //Вывод инфо об ошибке
    {
        QMessageBox::warning(this,"Ошибка","База данных устала");
    }
}


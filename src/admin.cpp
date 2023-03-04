#include "admin.h"
#include "ui_admin.h"
#include <mainwindow.h>

///Конструктор
admin::admin(QWidget *parent,int id,QString login_r) :
    QWidget(parent),
    ui(new Ui::admin)
{
    ui->setupUi(this);
    Id=id; //Получаем значения id из других окон
    loginr=login_r;
    ui->label_no_order->hide(); //Скрываем label с инфо по отсутствию заказов
    updateTables(); //Обновляем таблицы
}

///Деструктор
admin::~admin()
{
    db->close();
    delete db;
    delete ui;
}
///Функция с id зарегистрированного клиента
void admin::setLabel(const int& reg_id)
{
    registr_id=reg_id; //Записываем id зарегистрированного ресторана
    updateTables(); //Обновляем таблицы
}
///Подключение в БД
void admin::connection(){
    if (db != nullptr) //если подключение уже есть
    {
        db->close(); //закрываем экземпляр бд
        delete db; //удаляем экземпляр
        db = nullptr; //присваиваем значение nullptr
        return;
    }
    else  //Иначе создаем новое подключение
    {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL"));
    }
    QString dbName("postgres"); //db name
    QString host("127.0.0.1"); //server address
    QString usr("postgres"); //login
    QString dbPasw("123"); //password
    ///Передаем значения бд
    db->setDatabaseName(dbName);
    db->setHostName(host);
    db->setUserName(usr);
    db->setPassword(dbPasw);
    db->setPort(5432);
    if (!db->open()) //Если другая бд уже открыта
    {
        delete  db;
        db=nullptr;
    }
}
///Обновление таблиц
void admin::updateTables()
{
//    if (Id!=-1) //Если админ авторизовался
//    {
//        currentAutorization=Id;
//    }
    if (registr_id!=-1) //Если админ зарегистрировался
    {
        Id=registr_id;
    }
//    model1->clear();
    QSqlQuery QueryDish; //Запрос для выборки блюд
    QSqlDatabase db = QSqlDatabase::database("QPSQL");
    QueryDish.exec("SELECT id FROM rtdm.dishess WHERE rest_id="+QString::number(Id)+"");
    if (QueryDish.next()) //Если запрос выполняется
    {
        ui->label_dish->hide(); //Скрываем подпись, показываем кнопки
        ui->pushButton_save->setEnabled(true);
        ui->pushButton_delete->setEnabled(true);
        model1 = new QSqlTableModel(this, db); //модель с блюдами
        model1->setTable("rtdm.dishess"); //Задаем таблицу для select
        model1->setFilter("dishess.rest_id="+QString::number(Id)+""); //Условие для текущего ресторана
        model1->select();
        model1->setHeaderData(1, Qt::Horizontal, tr("Название блюда")); //Подпись колонок
        model1->setHeaderData(2, Qt::Horizontal, tr("Цена (руб)"));
        ui->menuView->setModel(model1);
        ui->menuView->setColumnHidden(0, true); //скрываем лишние колонки
        ui->menuView->setColumnHidden(3, true);
        ui->menuView->show();
        ui->menuView->resizeRowsToContents(); //редактирование размера menuView
        ui->menuView->resizeRowsToContents();
    }
    else //Запрос не выполняется
    {
     ui->label_dish->setText("Меню пустое"); //показываем инфо об отсутствие блюд
     ui->pushButton_save->setEnabled(false);
     ui->pushButton_delete->setEnabled(false);
    }

    model = new QSqlTableModel(this, db); //модель с информацией о ресторане
    model->setTable("rtdm.rest");
    model->setFilter("rest.id="+QString::number(Id)+""); //условие для текущего ресторана
    model->select();
    model->setHeaderData(1, Qt::Horizontal, tr("Название ресторана")); //подпись колонок
    model->setHeaderData(3, Qt::Horizontal, tr("Описание"));
    model->setHeaderData(4, Qt::Horizontal, tr("Адрес"));
    ui->rest_info->setModel(model);
    ui->rest_info->setColumnHidden(0, true); //скрываем лишние колонки
    ui->rest_info->setColumnHidden(2, true);
    ui->rest_info->show();
    ui->rest_info->resizeColumnsToContents(); //редактирование размера rest_info
    ui->rest_info->resizeRowsToContents();

    QSqlQuery QueryAllOrder; //Запрос для отображения заказов этого ресторана
    QueryAllOrder.exec("SELECT a.id, a.order_date, a.order_address, b.name, b.phone, d.dishes_name, d.price "
                    "FROM rtdm.client_order a "
                    "JOIN rtdm.clients_inf b ON a.client_id=b.id "
                    "JOIN rtdm.cook_tasks c ON a.id=c.order_id "
                    "JOIN rtdm.dishess d ON c.dishes_id=d.id and c.rest_id =d.rest_id "
                    "WHERE c.rest_id="+QString::number(Id)+"");
    if (QueryAllOrder.next()) //Если заказы были найдены, то отображаем их в OrderCurrent
    {
        ui->OrderCurrent->setColumnCount(7);
        for(int i = 0; QueryAllOrder.next(); i++) //Заполняем таблицу OrderCurrent
        {
            ui->OrderCurrent->insertRow(i);
            ui->OrderCurrent->setItem(i,0, new QTableWidgetItem(QueryAllOrder.value(0).toString()));
            ui->OrderCurrent->setItem(i,1, new QTableWidgetItem(QueryAllOrder.value(1).toString()));
            ui->OrderCurrent->setItem(i,2, new QTableWidgetItem(QueryAllOrder.value(2).toString()));
            ui->OrderCurrent->setItem(i,3, new QTableWidgetItem(QueryAllOrder.value(3).toString()));
            ui->OrderCurrent->setItem(i,4, new QTableWidgetItem(QueryAllOrder.value(4).toString()));
            ui->OrderCurrent->setItem(i,5, new QTableWidgetItem(QueryAllOrder.value(5).toString()));
            ui->OrderCurrent->setItem(i,6, new QTableWidgetItem(QueryAllOrder.value(6).toString()));
        }
        //Задаем подпись к колонкам
            ui->OrderCurrent->setHorizontalHeaderLabels(QString("Номер заказа;Дата;Адрес заказа;Имя клиента;Телефон клиента;Название блюда;Цена (руб)").split(";"));
            ui->OrderCurrent->resizeColumnsToContents(); //Устанавливаем размеры колоном и строк
            ui->OrderCurrent->resizeRowsToContents();
    }
    else  //Если у ресторана заказов еще нет
    {
        ui->label_no_order->show();
    }
}
///Добавление нового блюда
void admin::on_insertDish_clicked()
{
    QString name = ui->DishName->text().trimmed(); //Получение данных из интерфейса
    QString priceText = ui->DishPrice->text();
    //Если есть ошибки, то показываем QMessageBox

    QRegExp rool("[^0-9а-яА-ЯёЁa-zA-Z]");
    QValidator *validator = new QRegExpValidator(rool, this);
    ui->DishName->setValidator(validator);
    if (name.contains(rool)) {
        QMessageBox::information(this, "Инфо", "Поле Название не должно содержать спецсимволов");
        ui->DishName->clear();
        ui->DishPrice->clear();
        return;
    }
    if (name.isEmpty()) //Название блюда пустое
    {
        QMessageBox::critical(this, "Ошибка", "Проверьте поле Название блюда");
        return;
    }
    if (priceText.isEmpty()) //Если поле "цена" пустое
    {
        QMessageBox::critical(this, "Ошибка", "Необходимо ввести цену блюда");
        return;
    }
    if (priceText.length()>9) //Если цена слишком длинна
    {
        QMessageBox::critical(this, "Ошибка", "Цена за блюдо не может быть длиннее 9 символов");
        return;
    }

    bool ok;
    int price =priceText.toInt(&ok);
    int price2=ui->DishPrice->text().toInt();
    if (!ok) //Цена за блюдо не является числом
    {
        QMessageBox::critical(this, "Ошибка", "Цена за блюда должна быть целым числом");
        return;
    }

    if (price<0) //Если уена за блюдо отрицательная
    {
        QMessageBox::critical(this, "Ошибка", "Цена за блюдо должна быть больше 0");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Добавить блюдо?",
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No){
        return;
    }
    if (Id!=-1)
    {
        QSqlQuery queryInsert("INSERT INTO rtdm.dishess(rest_id,dishes_name,price) VALUES("+QString::number(Id)+",'"+name+"',"+QString::number(price2)+");");
        queryInsert.first();
    }
    //Очистка полей
    ui->DishName->clear();
    ui->DishPrice->clear();
    updateTables(); //Обновляем таблицы
}
///Сохранение изменений в меню
void admin::on_pushButton_save_clicked()
{
    if(model1->submitAll())
    {
        model1->database().transaction();

        model1->database().commit();
        QMessageBox::information(this,"Сохранено","Изменения сохранены"); //Выводим QMessageBox с информацией об успешном сохранении
    }
    model1->database().rollback();
    updateTables(); //Обновляем таблицы
}
///Сохранение изменений в инфо
void admin::on_pushButton_save_info_clicked()
{
    if(model->submitAll())
    {
        model->database().transaction();
        model->database().commit();
        QMessageBox::information(this,"Сохранено","Изменения сохранены"); //Выводим QMessageBox с информацией об успешном сохранении
    }
    else
//        QMessageBox::warning(this,"error",db->lastError().text());
    model->database().rollback();
    updateTables(); //Обновляем таблицы
}
///Удаление строки
void admin::on_pushButton_delete_clicked()
{
    ui->menuView->setSelectionBehavior(QAbstractItemView::SelectRows);
    model1->setEditStrategy(QSqlTableModel::OnRowChange);
    int selectedRowCount = ui->menuView->selectionModel()->selectedRows().count();
    if(selectedRowCount==0) //Строка для удаления не была выбрана
    {
        QMessageBox::critical(this, "Ошибка", "Выберите строку для удаления");
        return;
    }
    else //Если была выбрана строка для удаления
    {
        QModelIndexList selection = ui->menuView->selectionModel()->selectedRows();
        for (int i = 0; i < selection.count(); i++) //Проходимся по выбранным колонкам
        {
            model1->removeRow(selection.at(i).row());
            model1->select();
        }
        model1->setEditStrategy(QSqlTableModel::OnManualSubmit); //Меняем стратегию редактирования для model1
        updateTables(); //Обновляем таблицы
    }
}
///Выбор индекса блюда для удаления
void admin::on_menuView_clicked(const QModelIndex &index)
{
    currentRow=index.row(); //Получаем индекс выбранного блюда
}
///Выход из аккаунта
void admin::on_pushButton_exit_clicked()
{
    QMessageBox::StandardButton reply; //Спрашиваем пользователя хочет ли он выйти
      reply = QMessageBox::question(this, "Выход", "Вы уверены, что хотите выйти?",
                                    QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) //Если пользователь хочет выйти
      {
          MainWindow* mainw = new MainWindow();
          mainw->show(); //Выводим окно авторизации
          admin::close(); //Закрываем окно администратора
          delete  db;
          db=nullptr;
          return;
      }
      else //Пользователь не хочет выйти
      {
          return;
      }

}

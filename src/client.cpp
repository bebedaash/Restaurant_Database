#include "client.h"
#include "ui_client.h"
#include "order.h"
#include "mainwindow.h"


///Конструктор
client::client(QWidget *parent,int cliId) :
    QWidget(parent),
    ui(new Ui::client)
{
    ui->setupUi(this);
    clientId=cliId;
    //Скрываем лишнюю информацию
    ui->pushButton_nazad->hide();
    ui->createOrder->hide();
    ui->dishes_table->hide();
    ui->label_dish->hide();
    ui->label_address->hide();
    ui->lineEdit_address->hide();
    ui->name_no_order->hide();
    updateTables(); //Обновляем таблицы
}
///Деструктор
client::~client()
{
    clear_tables();
    db->close();
    delete db;
    delete ui;
}
void client::setClientLabel(const int& client_reg_id)
{
    reg_clientId=client_reg_id;
    updateTables(); //Обновляем таблицы
}

///Обновление таблиц
void client::updateTables(){
//    clearOrderHistory();
    clear_tables();
    if (reg_clientId!=-1) //если админ зарегистрировался
    {
        clientId=reg_clientId;
    }
    QSqlDatabase db = QSqlDatabase::database("QPSQL");
    model = new QSqlTableModel(this, db); //Загрузка информации о клиенте
    model->setTable("rtdm.clients_inf");
    model->setFilter("clients_inf.id="+QString::number(clientId)+"");
    model->select();
    model->setHeaderData(1, Qt::Horizontal, tr("Имя"));
    ui->client_info->setModel(model);
    ui->client_info->setColumnHidden(0, true);
    ui->client_info->show();
    ui->client_info->resizeColumnsToContents();
    ui->client_info->resizeRowsToContents();

    ui->order_history->clear();
    ui->order_history->setRowCount(0);

    QSqlQuery QueryClientOrder; //Запрос для вывода инфо о заказах клиента
    QueryClientOrder.exec("SELECT a.id, a.order_date, a.order_address,b.quantity, c.dishes_name, c.price "
                          "FROM rtdm.client_order a "
                          "JOIN rtdm.cook_tasks b ON a.id=b.order_id "
                          "JOIN rtdm.dishess c ON b.dishes_id=c.id and b.rest_id =c.rest_id "
                          "WHERE a.client_id="+QString::number(clientId)+"");
    if (QueryClientOrder.next()) //Если запрос выполняется
    {
        ui->order_history->setColumnCount(6);
        ui->order_history->insertRow(0);
        ui->order_history->setItem(0,0, new QTableWidgetItem(QueryClientOrder.value(0).toString()));
        ui->order_history->setItem(0,1, new QTableWidgetItem(QueryClientOrder.value(1).toString()));
        ui->order_history->setItem(0,2, new QTableWidgetItem(QueryClientOrder.value(2).toString()));
        ui->order_history->setItem(0,3, new QTableWidgetItem(QueryClientOrder.value(3).toString()));
        ui->order_history->setItem(0,4, new QTableWidgetItem(QueryClientOrder.value(4).toString()));
        ui->order_history->setItem(0,5, new QTableWidgetItem(QueryClientOrder.value(5).toString()));
        for(int i = 1; QueryClientOrder.next(); i++)
        {
            ui->order_history->insertRow(i);
            ui->order_history->setItem(i,0, new QTableWidgetItem(QueryClientOrder.value(0).toString()));
            ui->order_history->setItem(i,1, new QTableWidgetItem(QueryClientOrder.value(1).toString()));
            ui->order_history->setItem(i,2, new QTableWidgetItem(QueryClientOrder.value(2).toString()));
            ui->order_history->setItem(i,3, new QTableWidgetItem(QueryClientOrder.value(3).toString()));
            ui->order_history->setItem(i,4, new QTableWidgetItem(QueryClientOrder.value(4).toString()));
            ui->order_history->setItem(i,5, new QTableWidgetItem(QueryClientOrder.value(5).toString()));

        }
        ui->order_history->setHorizontalHeaderLabels(QString("Номер заказа;Дата;Адрес заказа;Количество;Название блюда;Цена (руб)").split(";"));
        ui->order_history->resizeColumnsToContents();
        ui->order_history->resizeRowsToContents();
        ui->name_no_order->hide();
    }
    else {
        ui->name_no_order->show();
    }

    QSqlQuery QueryRestInfo; //Запрос для вывода инфо о ресторанах
    QueryRestInfo.exec("SELECT id,namer,discr,address FROM rtdm.rest");
    //Заполняем таблицу restWidget
    ui->restWidget->setColumnCount(4);
    for(int i = 0; QueryRestInfo.next(); i++)
    {
        ui->restWidget->insertRow(i);
        ui->restWidget->setItem(i,1, new QTableWidgetItem(QueryRestInfo.value(1).toString()));
        ui->restWidget->setItem(i,2, new QTableWidgetItem(QueryRestInfo.value(2).toString()));
        ui->restWidget->setItem(i,3, new QTableWidgetItem(QueryRestInfo.value(3).toString()));
        ui->restWidget->setHorizontalHeaderLabels(QString("Выбор;Название ресторана;Описание;Адрес").split(";"));
        ui->restWidget->resizeColumnsToContents();
        ui->restWidget->resizeRowsToContents();

        //Создаем чекбоксы
        QWidget *checkBoxWidget = new QWidget();
        QCheckBox *checkBox = new QCheckBox();
        checkBoxList.append(checkBox);
        rest_id = QueryRestInfo.value(0).toInt();
        checkBox->setProperty("id", rest_id);

        QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); //Создаем слой с чекбоксами
        layoutCheckBox->addWidget(checkBox);            //Добавляем чекбоксы в слой
        layoutCheckBox->setAlignment(Qt::AlignCenter);  //Устанавливаем чекбоксы по центру
        layoutCheckBox->setContentsMargins(0,0,0,0);
        ui->restWidget->setCellWidget(i,0, checkBoxWidget);

    }
    ui->restWidget->resizeColumnsToContents(); //Меняем размер колонок
}
///Сохранение изменений в профиле
void client::on_pushButton_save_clicked()
{
    if(model->submitAll()){
        model->database().transaction();
        model->database().commit();
        QMessageBox::information(this,"Сохранено","Изменения сохранены");}
    else
        QMessageBox::warning(this,"error",db->lastError().text());
    model->database().rollback();
    updateTables(); //Обновляем таблицы
}
///Создание заказа
void client::on_createOrder_clicked()
{
    //checkDish-индекс блюда, checkId - индекс ресторана, clientId - клиент
    QString addressDeliv = ui->lineEdit_address->text().trimmed(); //Куда привезти
    dish_checked.clear(); //Очищаем ранее выбранные блюда
    for(auto checkbox : checkBoxList2) //Перебираем все чекбоксы в таблице с блюдами
    {
        bool isChecked = checkbox->isChecked(); //Флаг выбранного чекбокса
        if(isChecked) //Если чекбокс выбрали
        {
            dish_checked.append(checkbox->property("id").toInt()); //Добавляем id чекбокса в список с выбранными чекбоксами
        }
    }
    //Выводим QMessageBox с информацией об ошибках
    if (dish_checked.isEmpty())
    {
        QMessageBox::information(this,"Ошибка","Выберите блюда");
        return;
    }
    if (addressDeliv.isEmpty()){
        QMessageBox::information(this,"Информация","Введите адрес доставки");
        return;
    }  
    for (int i = 0; i < ui->dishes_table->rowCount(); ++i) //Перебираем таблицу с блюдами
    {
        //Берем строку и проверяем выделен ли чекбокс
        QWidget* widget = ui->dishes_table->cellWidget(i, 0);
        QCheckBox* checkBox = widget->findChild<QCheckBox*>();
        if(!checkBox->isChecked()) //если не выделен
        {
            continue;
        }
        //Если чек бокс выделен
         widget = ui->dishes_table->cellWidget(i, 3);
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(ui->dishes_table->cellWidget(i, 3));
        //        QString dish_id = checkBox->property("id").toString();
        int quantity = spinBox->value(); //Берем значение спинбокса
        if(quantity < 1) //Ошибка, если спинбокс 0
        {
            QMessageBox::warning(this, "Ошибка", "Количество блюд должно быть большо 0");
            return;
        }
    }
    //Подтверждаем информацию
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Создание заказа", "Вы уверены, что хотите создать заказ ?",
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply ==QMessageBox::No ) return;

    // Конец валидаций, дальше  запросы
    QList<QString> queries;
    QSqlQuery query("INSERT INTO rtdm.client_order(client_id,order_date, order_address) "
                    "VALUES ('"+QString::number(clientId)+"', current_timestamp,'"+addressDeliv+"') RETURNING id");

    bool success = query.first();
    if(!success){
        QMessageBox::warning(this, "Ошибка", "Не получилось создать заказ");
        return;
    }
    int id = query.value(0).toInt();
    //Перебираем таблицу с блюдами,если блюдо выбрано добавляем заказ в бд, количество блюд в бд
    for (int i = 0; i < ui->dishes_table->rowCount(); ++i)
    {
        QWidget* widget = ui->dishes_table->cellWidget(i, 0);
        QCheckBox* checkBox = widget->findChild<QCheckBox*>();

        if(!checkBox->isChecked()) {
            continue;
        }

        QString dish_id = checkBox->property("id").toString();
        widget = ui->dishes_table->cellWidget(i, 3);

        QSpinBox* spinBox = qobject_cast<QSpinBox*>(  ui->dishes_table->cellWidget(i, 3));
        QString quantity = QString::number(spinBox->value());
        QSqlQuery query_dishes;

          queries.append("INSERT INTO rtdm.cook_tasks(dishes_id, order_id, rest_id, quantity ) "
                                    "VALUES ( "+dish_id+", "+QString::number(id)+", "+QString::number(rest_checked[0])+", "+quantity+")");
    }
    QSqlQuery query_dishes;
    for(QString queryText : queries)
    {
        bool success =
        query_dishes.exec(queryText);
        if(!success){
            QMessageBox::warning(this, "Ошибка", "Один из запросов провалился: " + queryText);
        }
    }

    queries.clear();
    dish_checked.clear();
    QMessageBox::information(this,"Информация","Вы создали заказ");
//    ui->restWidget->setRowCount(0);
//    ui->restWidget->setColumnCount(0);
//    ui->restWidget->clear();
    ui->lineEdit_address->clear();
    updateTables();
}
///Переход на вторую страницу заказа
void client::on_pushButton_next_clicked()
{

    //Перебираем лист с чекбоксами
    for(auto checkbox : checkBoxList)
    {
        if(checkbox->isChecked()) //Если чекбокс выбран, добавляем его в лист с выбранными ресторанами
        {
            rest_checked.append(checkbox->property("id").toInt());
        }
    }
    //Выводим QMessageBox с ошибками
    if (rest_checked.size()!= 1){
        QMessageBox::information(this,"Ошибка","Выберите один ресторан. Вы выбрали " + QString::number(rest_checked.size()));
        rest_checked.clear();
        return;
    }
    QSqlQuery qwer; //Вывод инфо о блюдах ресторана
    bool success = qwer.exec("SELECT dishes_name,price,id FROM rtdm.dishess WHERE dishess.rest_id="+QString::number(rest_checked[0])+"");
  if(!success) {
      QMessageBox::warning(this, "Ошибка","Ошибка при получении данных о блюдах ресторана ("+QString::number(rest_checked[0])+")");
      return;
  }
    ui->dishes_table->setColumnCount(4);
    // Перед добавлением чекбоксов в массив, очищаем массив
    checkBoxList2.clear();
    for(int i = 0; qwer.next(); i++)
    {
        //Выводим инфо из запроса в таблицу
        ui->dishes_table->insertRow(i);
        ui->dishes_table->setItem(i,1, new QTableWidgetItem(qwer.value(0).toString()));
        ui->dishes_table->setItem(i,2, new QTableWidgetItem(qwer.value(1).toString()));
        ui->dishes_table->setHorizontalHeaderLabels(QString("Выбор;Название блюда;Цена (руб);Количество порций").split(";"));
        ui->dishes_table->resizeColumnsToContents();
        ui->dishes_table->resizeRowsToContents();

        //Создаем чекбоксы, задаем им в свойства id из БД
        QWidget *checkBoxWidget1 = new QWidget();
        QCheckBox *checkBox1 = new QCheckBox();      //Декларируем чекбоксы
        checkBoxList2.append(checkBox1);
        dish_id = qwer.value(2).toInt();
        checkBox1->setProperty("id", dish_id);
        //Создаем слой с чекбоксами
        QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget1);
        layoutCheckBox->addWidget(checkBox1);
        layoutCheckBox->setAlignment(Qt::AlignCenter);
        layoutCheckBox->setContentsMargins(0,0,0,0);
        ui->dishes_table->setCellWidget(i,0, checkBoxWidget1);

        QSpinBox *spinBox = new QSpinBox(); //Добавление спин бокса
        spinBox->setMinimum(0);
        spinBox->setMaximum(50);
        ui->dishes_table->setCellWidget(i, 3, spinBox);

    }
    //Скрываем/показываем информацию, также очищаем таблицы
    ui->pushButton_nazad->show();
    ui->dishes_table->show();
    ui->label_dish->show();
    ui->label_address->show();
    ui->lineEdit_address->show();
    ui->createOrder->show();
    ui->label_rest->hide();
    ui->restWidget->hide();
    ui->pushButton_next->hide();
    ui->restWidget->setRowCount(0);
    ui->restWidget->setColumnCount(0);
    ui->restWidget->clear();
//    updateTables();  //Обновляем таблицы
}
///Переход на первую страницу заказа
void client::on_pushButton_nazad_clicked()
{
    //Скрываем/показываем информацию, также очищаем таблицы
    ui->dishes_table->hide();
    ui->label_dish->hide();
    ui->label_rest->show();
    ui->restWidget->show();
    ui->pushButton_next->show();
    ui->pushButton_nazad->hide();
    ui->createOrder->hide();
    ui->label_address->hide();
    ui->lineEdit_address->hide();
    ui->restWidget->setRowCount(0);
    ui->restWidget->setColumnCount(0);
    ui->restWidget->clear();
    ui->dishes_table->setRowCount(0);
    ui->dishes_table->setColumnCount(0);
    ui->dishes_table->clear();
    ui->lineEdit_address->clear();
    checkBoxList.clear();
    rest_checked.clear();
    updateTables(); //Обновляем таблицы
}
///Выход из аккаунта
void client::on_pushButton_exit_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Выход", "Вы уверены, что хотите выйти?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        MainWindow* mainw = new MainWindow();
        mainw->show();
        client::close();
        delete  db;
        db=nullptr;
    } else {
        return;
    }
}
///Очистка таблиц
void client::clear_tables()
{
    ui->order_history->clear();
    ui->order_history->setRowCount(0);

    ui->restWidget->clear();
    ui->restWidget->setRowCount(0);
}


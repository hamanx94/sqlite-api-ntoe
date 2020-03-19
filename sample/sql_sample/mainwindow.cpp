#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sqlapi.h"

#define d_FIELD_SET_LIST_SIZE    (10)

tFIELD_DEF SAMPLE_TABLE[] =
{
    {"Number", _TEXT, _PRIMARY_KEY},
    {"Name", _TEXT, _NOT_NULL},
    {"Address", _TEXT, _NOT_NULL},
};

#define d_STRUCT_SIZE(x)  (sizeof(x) / sizeof(x[0]))
//#define d_TABLE1_SIZE   d_FIELD_SIZE(SAMPLE_TABLE)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    char errmsg[256];

    sql_db_connect("sql_sample.db", errmsg);
    sql_table_create("sql_sample.db", "table1", d_STRUCT_SIZE(SAMPLE_TABLE), SAMPLE_TABLE, errmsg);

    tFIELD field_set[]=
    {
        {"Number", 256, nullptr},
        {"Name", 256, nullptr},
        {"Address", 256, nullptr},
    };

    tFIELD_IO_SET field_set_list[d_FIELD_SET_LIST_SIZE];
    for(int i = 0 ; i < (d_FIELD_SET_LIST_SIZE - 1) ; i++)
    {
        field_set_list[i].status = d_STATUS_EMPTY;
        for(int j = 0 ; j < d_STRUCT_SIZE(field_set) ; j++)
        {
            strcpy(field_set_list[i].field[j].FieldName, field_set[j].FieldName);
            field_set_list[i].field[j].FieldDataLength = field_set[j].FieldDataLength;
            field_set_list[i].field[j].FieldData = (char *)malloc(field_set[j].FieldDataLength);
            memset(field_set_list[i].field[j].FieldData, 0, field_set[j].FieldDataLength);
        }
    }
    field_set_list[d_FIELD_SET_LIST_SIZE - 1].status = d_STATUS_LOCK;

    sql_field_read("sql_sample.db", "table1", nullptr, d_STRUCT_SIZE(field_set), field_set_list, errmsg);
    sql_db_close("sql_sample.db");

    for(int i = 0 ; i < (d_FIELD_SET_LIST_SIZE - 1) ; i++)
        for(int j = 0 ; j < d_STRUCT_SIZE(field_set) ; j++)
            free(field_set_list[i].field[j].FieldData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

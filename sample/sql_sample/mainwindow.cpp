#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sqlapi.h"

#define d_FIELD_SET_LIST_SIZE    (10)

static tFIELD_DEF SAMPLE_TABLE[] =
{
    {"Number", _TEXT, _PRIMARY_KEY},
    {"Name", _TEXT, _NONE},
    {"Address", _TEXT, _NONE},
};

#define d_STRUCT_SIZE(x)  (sizeof(x) / sizeof(x[0]))

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    char errmsg[256];
    int total, total_item;
    bool isOK;
    FieldData *fieldDataList;

    sql_db_connect("sql_sample.db", errmsg);
    sql_table_create("sql_sample.db", "table1", d_STRUCT_SIZE(SAMPLE_TABLE), SAMPLE_TABLE, errmsg);

    tFIELD fieldWriteList[] =
    {
        {"Number", _TEXT, "666"},
        {"Name", _TEXT, "haman666"},
        {"Address", _TEXT, "USA"}
    };
    sql_data_write("sql_sample.db", "table1", fieldWriteList, d_STRUCT_SIZE(fieldWriteList), errmsg);

    tFIELD fieldUpdateList[] =
    {
        {"Number", _TEXT, "2"},
        {"Address", _TEXT, "USA"}
    };
    sql_data_update("sql_sample.db", "table1", fieldUpdateList, d_STRUCT_SIZE(fieldUpdateList), "Name='hamanx94'", errmsg);

    FieldName fieldNameList[]=
    {
        {"Number"},
        {"Name"},
        {"Address"},
    };
    isOK = sql_field_read("sql_sample.db", "table1", fieldNameList, d_STRUCT_SIZE(fieldNameList), nullptr, &total, nullptr, errmsg);
    if(isOK)
    {
        total_item = d_STRUCT_SIZE(fieldNameList) * total;
        fieldDataList = (FieldData *)calloc(total_item, (sizeof(FieldData)));
        isOK = sql_field_read("sql_sample.db", "table1", fieldNameList, d_STRUCT_SIZE(fieldNameList), fieldDataList, &total, nullptr, errmsg);
        if(isOK)
        {
            for(int i = 0 ; i < total_item ; i++)
                free(fieldDataList[i]);
        }
        free(fieldDataList);
    }

    sql_db_close("sql_sample.db");
}

MainWindow::~MainWindow()
{
    delete ui;
}

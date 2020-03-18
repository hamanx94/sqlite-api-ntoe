#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "sqlapi.h"

#define d_DB_SIZE       (256)
#define d_DB_CMD_SIZE   (16 * 1024)

typedef struct
{
    char dbfilename[256];
    sqlite3 *handle;
}tDB_LIST;

static tDB_LIST db_list[d_DB_SIZE];

static int find_db(char *dbfilename)
{
    int index;

    index = 0;
    while((index < d_DB_SIZE))
    {
        if(strcmp(db_list[index].dbfilename, dbfilename) == 0)
            break;
        index++;
    }

    if(index == d_DB_SIZE)
        return -1;

    return  index;
}

static int add_db(char *dbfilename)
{
    int index;

    index = 0;
    while(index < d_DB_SIZE)
    {
        if(strlen(db_list[index].dbfilename) == 0)
        {
            strcpy(db_list[index].dbfilename, dbfilename);
            break;
        }
        index++;
    }

    if(index == d_DB_SIZE)
        return -1;

    return  index;
}

bool sql_db_connect(char *dbfilename, char *err)
{
    int index;
    int rc;

    index = find_db(dbfilename);
    if(index != -1)
        return false;

    index = add_db(dbfilename);
    if(index == -1)
        return false;

    rc = sqlite3_open(db_list[index].dbfilename, &db_list[index].handle);
    if(rc)
    {
        strcpy(err, (char *)sqlite3_errmsg(db_list[index].handle));
        return false;
    }

    return true;
}

bool sql_table_create(char *dbfilename, char *tablename, int FieldCount, tFIELD_DEF *stFieldDefs, char *err)
{
    char *zErrMsg = nullptr;
    int rc;
    char str[512];
    int i, index;

    memset(str, 0, sizeof(str));
    sprintf(str, "create table IF NOT EXISTS %s (", tablename);

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    for (i = 0 ; i < FieldCount ; i++)
    {
        memcpy(&str[strlen(str)], stFieldDefs[i].FieldName, strlen(stFieldDefs[i].FieldName));
        switch(stFieldDefs[i].emDataType)
        {
            case _INTEGER:
                sprintf(&str[strlen(str)], " INTEGER");
                break;
            case _TEXT:
                sprintf(&str[strlen(str)], " TEXT");
                break;
            case _REAL:
                sprintf(&str[strlen(str)], " REAL");
                break;
            case _BLOB:
                sprintf(&str[strlen(str)], " BLOB");
                break;
            case _NULL:
                sprintf(&str[strlen(str)], " NULL");
                break;
            case _NUMERIC:
                sprintf(&str[strlen(str)], " NUMERIC");
                break;
        }

        if ((stFieldDefs[i].emConstraint & _NOT_NULL) == _NOT_NULL)
            sprintf(&str[strlen(str)], " NOT NULL");

        if ((stFieldDefs[i].emConstraint & _PRIMARY_KEY)== _PRIMARY_KEY)
        {
            sprintf(&str[strlen(str)], " PRIMARY KEY");

            if ((stFieldDefs[i].emConstraint & _AUTOINCREMENT) == _AUTOINCREMENT)
                sprintf(&str[strlen(str)], " AUTOINCREMENT");
        }

        if ((stFieldDefs[i].emConstraint & _UNIQUE) == _UNIQUE)
            sprintf(&str[strlen(str)], " UNIQUE");

        if (i != FieldCount - 1)
            str[strlen(str)] = ',';
        else
            str[strlen(str)] = ')';
    }

    rc = sqlite3_exec(db_list[index].handle, str, nullptr, nullptr, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

static int read_callback(void *data, int argc, char **argv, char **azColName)
{
    tFIELD_IO_SET *para = (tFIELD_IO_SET *)data;
    tFIELD *current;
    int offset;

    offset = 0;
    while(1)
    {
        if(para[offset].status == d_STATUS_EMPTY)
        {
            para[offset].status = d_STATUS_BEEN_SET;
            current = para[offset].field;
            break;
        }
        else if(para[offset].status == d_STATUS_LOCK)
            return d_STATUS_LOCK;

        offset++;
    }

    for(int i = 0 ; i < argc ; i++)
    {
        for(int j = 0 ; j < argc ; j++)
        {
            if(strcmp(current[i].FieldName, azColName[j]) == 0)
            {
                strcpy(current[i].FieldData, argv[j]);
                break;
            }
        }
    }

    return 0;
}

bool sql_field_read(char *dbfilename, char *tablename, char *condition, int FieldCount, tFIELD_IO_SET *stOutList, char *err)
{
    char *zErrMsg = nullptr;
    int i, index, rc;
    char cmd[d_DB_CMD_SIZE];

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    strcpy(cmd, "SELECT ");
    for(i = 0 ; i < FieldCount ; i++)
    {
        strcat(cmd, stOutList[0].field[i].FieldName);
        if(i == (FieldCount - 1))
            strcat(cmd, " ");
        else
            strcat(cmd, ", ");
    }
    strcat(cmd, "FROM ");
    strcat(cmd, tablename);
    if(condition)
    {
        strcat(cmd, " ");
        strcat(cmd, condition);
    }
    strcat(cmd, ";");

    rc = sqlite3_exec(db_list[index].handle, cmd, read_callback, (void *)stOutList, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

void sql_db_close(char *filename)
{
    int index = 0;

    while(index < d_DB_SIZE)
    {
        if(db_list[index].handle && strcmp(db_list[index].dbfilename, filename) == 0)
        {
            sqlite3_close(db_list[index].handle);
            db_list[index].handle = nullptr;
            db_list[index].dbfilename[0] = 0;
            break;
        }
        index ++;
    }
}

void sql_close_all()
{
    int index = 0;

    while(index < d_DB_SIZE)
    {
        if(db_list[index].handle)
        {
            sqlite3_close(db_list[index].handle);
            db_list[index].handle = nullptr;
            db_list[index].dbfilename[0] = 0;
        }
        index ++;
    }
}

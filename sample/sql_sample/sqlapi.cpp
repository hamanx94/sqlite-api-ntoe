#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "sqlapi.h"

#define d_DB_SIZE       (256)
#define d_DB_CMD_SIZE   (16 * 1024)

static int g_Count;

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
    FieldData *para = (FieldData *)data;
    int offset;

    offset = g_Count * argc;
    g_Count++;
    if(para == nullptr)
    {
        return 0;
    }

    for(int i = 0 ; i < argc ; i++)
    {
        para[offset] = (FieldData)malloc(strlen(argv[i]) + 1);
        strcpy(para[offset], argv[i]);
        offset++;
    }

    return 0;
}

bool sql_field_read(char *dbfilename, char *tablename, FieldName *fieldNameList, int fieldNameCount, FieldData *fieldDataList , int *fieldDataCount, char *condition, char *err)
{
    char *zErrMsg = nullptr;
    int i, index, rc;
    char cmd[d_DB_CMD_SIZE];

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    strcpy(cmd, "SELECT ");
    for(i = 0 ; i < fieldNameCount ; i++)
    {
        strcat(cmd, fieldNameList[i]);
        if(i == (fieldNameCount - 1))
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

    g_Count = 0;
    if(fieldDataList == NULL)
        rc = sqlite3_exec(db_list[index].handle, cmd, read_callback, nullptr, &zErrMsg);
    else
        rc = sqlite3_exec(db_list[index].handle, cmd, read_callback, (void *)fieldDataList, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    *fieldDataCount = g_Count;
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

bool sql_data_write(char *dbfilename, char *tablename, tFIELD_DEF *stFieldDefs, int fieldDefCount, FieldData *fieldDataList, char *err)
{
    char *zErrMsg = nullptr;
    int i, index, rc;
    char cmd[d_DB_CMD_SIZE];

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    sprintf(cmd, "INSERT INTO %s (", tablename);
    for(i = 0 ; i < fieldDefCount ; i++)
    {
        strcat(cmd, stFieldDefs[i].FieldName);
        if(i == (fieldDefCount - 1))
            strcat(cmd, ") ");
        else
            strcat(cmd, ", ");
    }
    strcat(cmd, "VALUES (");
    for(i = 0 ; i < fieldDefCount ; i++)
    {
        if(stFieldDefs[i].emDataType == _INTEGER)
            strcat(cmd, fieldDataList[i]);
        else
        {
            strcat(cmd, "'");
            strcat(cmd, fieldDataList[i]);
            strcat(cmd, "'");
        }

        if(i == (fieldDefCount - 1))
            strcat(cmd, ") ");
        else
            strcat(cmd, ", ");
    }

    strcat(cmd, ";");
    rc = sqlite3_exec(db_list[index].handle, cmd, nullptr, nullptr, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

bool sql_data_update(char *dbfilename, char *tablename, tFIELD *fieldList, int fieldCount, char *condition, char *err)
{
    char *zErrMsg = nullptr;
    int i, index, rc;
    char cmd[d_DB_CMD_SIZE];

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    sprintf(cmd, "UPDATE %s SET ", tablename);
    for(i = 0 ; i < fieldCount ; i++)
    {
        strcat(cmd, fieldList[i].FieldName);
        strcat(cmd, "=");
        if(fieldList[i].emDataType == _INTEGER)
            strcat(cmd, fieldList[i].FieldData);
        else
        {
            strcat(cmd, "'");
            strcat(cmd, fieldList[i].FieldData);
            strcat(cmd, "'");
        }
        if(i == (fieldCount - 1))
            strcat(cmd, " ");
        else
            strcat(cmd, ", ");
    }
    strcat(cmd, "WHERE ");
    strcat(cmd, condition);
    strcat(cmd, ";");

    rc = sqlite3_exec(db_list[index].handle, cmd, nullptr, nullptr, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

bool sql_exec(char *dbfilename, char *cmd, char *err)
{
    char *zErrMsg = nullptr;
    int index, rc;

    index = find_db(dbfilename);
    if(index == -1)
        return false;

    rc = sqlite3_exec(db_list[index].handle, cmd, nullptr, nullptr, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        strcpy(err, zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

bool sql_commit(char *dbfilename, char *err)
{
    return sql_exec(dbfilename, "BEGIN", err);
}

bool sql_begin(char *dbfilename, char *err)
{
    return sql_exec(dbfilename, "COMMIT", err);
}

// Copyright(c) 2015 - 2017, Ryft Systems, Inc. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
// following conditions are met :
//
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
// disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
// disclaimer in the documentation and / or other materials provided with the distribution.
//
// All advertising materials mentioning features or use of this software must display the following acknowledgement : 
// 
// This product includes software developed by Ryft Systems, Inc.
// 
// Neither the name of Ryft Systems, Inc.nor the names of its contributors may be used to endorse or promote products 
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY RYFT SYSTEMS, INC. ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL RYFT SYSTEMS, INC.BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
// DAMAGE.
//
// GNU General Public License, Version 2, June 1991 
// Copyright (c) 1989, 1991 Free Software Foundation, Inc. / 51 Franklin St. 5th Floor, Boston, MA 02110 - 1301, USA
#include "wrapper.h"
using namespace RYFT_ODBCWrap;

ODBC_Wrapper::ODBC_Wrapper() : __initialized(false), __dbc(NULL), __stmt(NULL)
{
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &__env) == SQL_SUCCESS) {
        SQLSetEnvAttr(__env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)2UL, 0);
        __initialized = true;
    }
}

ODBC_Wrapper::~ODBC_Wrapper()
{
    if (__initialized)
        SQLFreeHandle(SQL_HANDLE_ENV, __env);
    __initialized = false;
}

bool ODBC_Wrapper::connect(string& dsn, string& uid, string& pwd) 
{
    RETCODE retcode;

    if (!__initialized)
        return false;

    if (__connected)
        return true;

    retcode = SQLAllocConnect(__env, &__dbc);
    if (retcode != SQL_SUCCESS) {
        __odbcError();
        return false;
    }

    retcode = SQLConnect(__dbc, (SQLCHAR *)dsn.c_str(), SQL_NTS, (SQLCHAR *)uid.c_str(), SQL_NTS, (SQLCHAR *)pwd.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS) {
        __odbcError();
        return false;
    }

    __connected = true;
    return true;
}

void ODBC_Wrapper::disconnect()
{
    if (__connected) {
        SQLDisconnect(__dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, __dbc);
    }
    __connected = false;
}

bool ODBC_Wrapper::isConnected()
{
    return __connected;
}

bool ODBC_Wrapper::describeCatalog(ODBC_ResultSet& tables)
{
    SQLAllocHandle(SQL_HANDLE_STMT, __dbc, &__stmt);
    SQLTables(__stmt, (SQLCHAR *)"%", SQL_NTS, NULL, 0, NULL, 0, NULL, 0);

    __processResultSet(tables);

    SQLFreeHandle(SQL_HANDLE_STMT, __stmt);
    __stmt = NULL;
    return true;
}

bool ODBC_Wrapper::describeTable(string& table, ODBC_ResultSet& columns)
{
    SQLAllocHandle(SQL_HANDLE_STMT, __dbc, &__stmt);
    SQLColumns(__stmt, NULL, 0, NULL, 0, (SQLCHAR *)table.c_str(), SQL_NTS, NULL, 0);
    __processResultSet(columns);

    SQLFreeHandle(SQL_HANDLE_STMT, __stmt);
    __stmt = NULL;
    return true;
}

bool ODBC_Wrapper::describeDatatypes(ODBC_ResultSet& datatypes)
{
    SQLAllocHandle(SQL_HANDLE_STMT, __dbc, &__stmt);
    SQLGetTypeInfo(__stmt, 0);

    __processResultSet(datatypes);

    SQLFreeHandle(SQL_HANDLE_STMT, __stmt);
    __stmt = NULL;
    return true;
}

bool ODBC_Wrapper::describeProcedures(ODBC_ResultSet& procedures)
{
    SQLAllocHandle(SQL_HANDLE_STMT, __dbc, &__stmt);
    SQLProcedures(__stmt, NULL, 0, NULL, 0, NULL, 0);

    __processResultSet(procedures);

    SQLFreeHandle(SQL_HANDLE_STMT, __stmt);
    __stmt = NULL;
    return true;
}

bool ODBC_Wrapper::execQuery(string& query, ODBC_ResultSet& resultSet)
{
    SQLSMALLINT numParams = 0;
    SQLRETURN retcode;
    bool ret = false;

    if (!__connected)
        return false;

    resultSet.clear();

    SQLAllocHandle(SQL_HANDLE_STMT, __dbc, &__stmt);
    retcode = SQLExecDirect(__stmt, (SQLCHAR *)query.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        __odbcError();
        goto exit;
    }
    
    ret = __processResultSet(resultSet);

exit:
    SQLFreeStmt((SQLHSTMT)__stmt, SQL_UNBIND);
    SQLFreeStmt((SQLHSTMT)__stmt, SQL_CLOSE);

    SQLFreeHandle(SQL_HANDLE_STMT, __stmt);
    __stmt = NULL;

    return ret;
}

ODBC_Errors ODBC_Wrapper::odbcErrors()
{
    ODBC_Errors odbcErrors = __odbcErrors;
    __odbcErrors.clear();
    return odbcErrors;
}

// private
void ODBC_Wrapper::__odbcError()
{
    char	sqlState[10];
    char	sqlMessage[SQL_MAX_MESSAGE_LENGTH];
    SDWORD	nativeCode;
    SWORD	msglen;
    RETCODE	retcode;

    do {

        retcode = SQLError(__env, __dbc, __stmt, (UCHAR *)sqlState, &nativeCode, (UCHAR *)sqlMessage, SQL_MAX_MESSAGE_LENGTH - 1, &msglen);
        if (retcode == SQL_ERROR)
            return;

        if (retcode != SQL_NO_DATA_FOUND) {
            ODBC_Error odbcError;
            sqlMessage[msglen] = '\0';
            odbcError.nativeCode(nativeCode);
            odbcError.sqlMessage(sqlMessage);
            odbcError.sqlState(sqlState);
            __odbcErrors.push_back(odbcError);
        }

    } while (retcode != SQL_NO_DATA_FOUND);
}

bool ODBC_Wrapper::__processResultSet(ODBC_ResultSet& resultSet)
{
    SQLRETURN retcode = SQL_NO_DATA;
    SQLSMALLINT sqlCols;
    char *colName;
    SQLSMALLINT colNameSiz = 0;
    SQLSMALLINT colNameLen;
    SQLSMALLINT sqlType;
    SQLULEN colLength;
    SQLLEN displayLen;
    SQLLEN octetLen;
    SQLSMALLINT decDigits;
    SQLSMALLINT nullable;
    SQLLEN rowCount;

    SQLNumResultCols(__stmt, &sqlCols);
    if (sqlCols) {
        // there is a result set, process
        retcode = SQLGetInfo(__dbc, SQL_MAX_COLUMN_NAME_LEN, &colNameSiz, sizeof(SWORD), NULL);
        if (!colNameSiz)
            colNameSiz = ODBC_ResultCol::MAX_COLUMN_NAME_LENGTH;
        colName = new char[colNameSiz];
        for (SQLSMALLINT colIdx = 1; colIdx <= sqlCols; colIdx++) {
            ODBC_ResultCol resultCol;
            resultCol.colIdx(colIdx);

            SQLDescribeCol(__stmt, colIdx, (UCHAR *)colName, colNameSiz, &colNameLen, &sqlType, &colLength, &decDigits, &nullable);
            SQLColAttribute(__stmt, colIdx, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displayLen);
            SQLColAttribute(__stmt, colIdx, SQL_DESC_OCTET_LENGTH, NULL, 0, NULL, (SQLLEN *)&octetLen);

            resultCol.colName(colName);
            resultCol.sqlType(sqlType);
            resultCol.colLength(colLength);
            resultCol.decDigits(decDigits);
            resultCol.nullable(CONVERT_BOOL(nullable));
            resultCol.displayLen(displayLen);
            resultCol.octetLen(octetLen);

            resultSet.__pushResultCol(resultCol);

            ODBC_Binding binding;
            resultSet.__allocBindResult(colIdx, &binding);
            SQLBindCol(__stmt, colIdx, binding.cType, binding.sqlPtr, binding.sqlLen, binding.sqlLenPtr);
        }
        delete colName;

        while ((retcode = SQLFetch(__stmt)) == SQL_SUCCESS)
            resultSet.__latchBindResult();

        if (retcode != SQL_NO_DATA)
            __odbcError();
    }
    else {
        SQLRowCount(__stmt, &rowCount);
        resultSet.setUpdateCount(rowCount);
    }

    return retcode == SQL_NO_DATA;
}
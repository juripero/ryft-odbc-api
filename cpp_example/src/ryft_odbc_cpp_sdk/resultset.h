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
#pragma once

#define CONVERT_BOOL(i) (i == 1 ? true : false)

class ODBC_ResultCol {
public:
    ODBC_ResultCol() : __colIdx(0), __sqlType(0), __nullable(false), __colLength(0), __decDigits(0), __displayLen(0) { }

    static const int MAX_COLUMN_NAME_LENGTH = 256;

    void colIdx(SQLSMALLINT colIdx)
    {
        __colIdx = colIdx;
    }
    SQLSMALLINT colIdx()
    {
        return __colIdx;
    }

    void colName(string colName)
    {
        __colName = colName;
    }
    string& colName()
    {
        return __colName;
    }

    void sqlType(SQLSMALLINT sqlType)
    {
        __sqlType = sqlType;
    }
    SQLSMALLINT sqlType()
    {
        return __sqlType;
    }

    void nullable(bool nullable)
    {
        __nullable = nullable;
    }
    bool nullable()
    {
        return __nullable;
    }

    void colLength(SQLULEN colLength)
    {
        __colLength = colLength;
    }
    SQLULEN colLength() 
    {
        return __colLength;
    }

    void decDigits(SQLSMALLINT decDigits)
    {
        __decDigits = decDigits;
    }
    SQLSMALLINT decDigits()
    {
        return __decDigits;
    }

    void displayLen(SQLLEN displayLen)
    {
        __displayLen = displayLen;
    }
    SQLLEN displayLen()
    {
        return __displayLen;
    }

    void octetLen(SQLLEN octetLen)
    {
        __octetLen = octetLen;
    }
    SQLLEN octetLen()
    {
        return __octetLen;
    }

private:
    SQLSMALLINT     __colIdx;
    string          __colName;
    SQLSMALLINT     __sqlType;
    SQLULEN         __colLength;
    SQLSMALLINT     __decDigits;
    SQLLEN          __displayLen;
    SQLLEN          __octetLen;
    bool            __nullable;
};
typedef vector<ODBC_ResultCol> ODBC_ResultCols;

const size_t c_minStep = 512;
typedef unsigned char bufferCls;
class buffer {
public:
    buffer() : __idx(0), __size(0), __capacity(0), __ptr(NULL) {
    }
    buffer(const buffer &buf) {
        *this = buf;
    }
    buffer(const bufferCls *il, size_t ll) : __idx(0) {
        __ptr = new bufferCls[ll];
        __capacity = ll;
        __size = ll;
        memcpy(__ptr, il, ll * sizeof(bufferCls));
    }
    ~buffer() {
        delete[] __ptr;
        __ptr = NULL;
    }
    bufferCls& operator[] (size_t n) {
        return __ptr[n];
    }
    buffer& operator= (const buffer& buf) {
        __ptr = new bufferCls[buf.__capacity];
        memcpy(__ptr, buf.__ptr, buf.__size * sizeof(bufferCls));
        __size = buf.__size;
        __capacity = buf.__capacity;
        __idx = buf.__idx;
        return *this;
    }
    inline void append(const buffer& buf) {
        __reserve(__size + buf.__size);
        memcpy(&__ptr[__size], buf.__ptr, buf.__size * sizeof(bufferCls));
        __size += buf.__size;
    }
    inline void clear() _NOEXCEPT{
        __size = 0;
        __idx = 0;
    }
    inline bool eof() {
        return __idx >= __size;
    }
    inline UCHAR getUCHAR() {
        UCHAR uc = (UCHAR)__ptr[__idx++];
        return uc;
    }
    inline USHORT getUSHORT() {
        USHORT us = MAKEWORD(__ptr[__idx + 1], __ptr[__idx]);
        __idx += sizeof(USHORT);
        return us;
    }
    inline UINT32 getULONG() {
        UINT32 ul = 0;
        for (int i = 0; i < sizeof(UINT32); i++) {
            ul = ul << 8;
            ul |= __ptr[__idx + i];
        }
        __idx += sizeof(UINT32);
        return ul;
    }
    inline ULONGLONG getULONGLONG() {
        ULONGLONG ull = 0;
        for (int i = 0; i < sizeof(ULONGLONG); i++) {
            ull = ull << 8;
            ull |= __ptr[__idx + i];
        }
        __idx += sizeof(ULONGLONG);
        return ull;
    }
    inline void get(bufferCls *dst, size_t length) {
        memcpy(dst, (const char *)&__ptr[__idx], length);
        __idx += length;
    }
    inline string getString() {
        string str;
        size_t usl = MAKEWORD(__ptr[__idx + 1], __ptr[__idx]);
        __idx += sizeof(USHORT);
        str.assign((const char *)&__ptr[__idx], usl);
        __idx += usl;
        return str;
    }
    inline void putUCHAR(UCHAR uc) {
        __push_back(uc);
    }
    inline void putUSHORT(USHORT us) {
        __push_back(HIBYTE(us));
        __push_back(LOBYTE(us));
    }
    inline void putULONG(UINT32 ul) {
        BYTE *pb = (BYTE *)&ul;
        for (int i = sizeof(UINT32); i > 0; i--)
            __push_back(pb[i - 1]);
    }
    inline void putULONGLONG(ULONGLONG ull) {
        BYTE *pb = (BYTE *)&ull;
        for (int i = sizeof(ULONGLONG); i > 0; i--)
            __push_back(pb[i - 1]);
    }
    inline void put(const bufferCls *src, size_t length) {
        __reserve(__size + length);
        memcpy((char *)&__ptr[__size], src, length);
        __size += length;
    }
    inline void putString(const bufferCls *str, size_t length) {
        putUSHORT((USHORT)length);
        put(str, length);
    }
    inline void reserve(size_t n) {
        __reserve(n);
    }
    inline void resize(size_t n) {
        __reserve(n);
        memset(&__ptr[__size], 0, __capacity - __size);
        __size = n;
    }
    inline void seek(int idx) {
        __idx = idx;
    }
    inline size_t seek() {
        return __idx;
    }
    inline size_t size() {
        return __size;
    }

protected:
    size_t           __idx;
    size_t           __capacity;
    size_t           __size;
    bufferCls *      __ptr;

private:
    inline void __push_back(const bufferCls value) {
        __reserve(__size + sizeof(bufferCls));
        __ptr[__size++] = value;
    }
    inline void __reserve(size_t n) {
        if (__capacity < n) {
            size_t step = max(n, __capacity + c_minStep);
            bufferCls *ptr = new bufferCls[step];
            if (__ptr)
                memcpy(ptr, __ptr, __size * sizeof(bufferCls));
            delete[] __ptr;
            __ptr = ptr;
            __capacity = step;
        }
    }
};

class ODBC_Binding {
public:
    SQLSMALLINT     cType;
    SQLPOINTER      sqlPtr;
    SQLLEN          sqlLen;
    SQLLEN *        sqlLenPtr;
}; 

class IBinding {
    friend class ODBC_ResultSet;

protected:
    static const unsigned char NULL_DATA = 0xFF;

    virtual bool  __next() = 0;

    virtual void __push_back() = 0;

    virtual bool __alloc_bind(ODBC_Binding *binding) = 0;

    virtual void __reset()
    {
        __data.seek(0);
    }

    virtual string __get_string() = 0;

    buffer  __data;
    bool    __isNull;
};

class BindString : public IBinding {
public:
    BindString(SQLULEN sizMax, bool nullable) : __sizMax(sizMax), __nullable(nullable) 
    { 
        __sqlPtr = new char[__sizMax];
    }
   ~BindString() 
    {
        delete __sqlPtr;
    }

protected:
    virtual bool  __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            __str = __data.getString();
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        binding->cType = SQL_C_CHAR;
        binding->sqlPtr = __sqlPtr;
        binding->sqlLen = __sizMax;
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        case SQL_NTS:
            __sqlLen = strlen((char *)__sqlPtr);
            // fall through
        default:
            __data.putUCHAR(0);
            __data.putString((const bufferCls *)__sqlPtr, (size_t)__sqlLen);
            break;
        }
    }

    virtual string __get_string()
    {
        if (__isNull)
            return "NULL";

        return __str;
    }

    SQLULEN     __sizMax;
    bool        __nullable;

    SQLPOINTER  __sqlPtr;
    SQLLEN      __sqlLen;

    string      __str;
};

class BindInteger : public IBinding {
public:
    static const int sizeof_SMALLINT = 2;
    static const int sizeof_INTEGER = 4;
    static const int sizeof_BIGINT = 20;

    static const int MAX_COLUMNS_BIGINT = 20;

    BindInteger(SQLULEN sizeof_, bool nullable) : __sizeof_(sizeof_), __nullable(nullable) { }

protected:
    virtual bool __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            if (__sizeof_ == sizeof_SMALLINT) {
                __sqlSmallInt = __data.getUSHORT();
            }
            if (__sizeof_ == sizeof_INTEGER) {
                __sqlInteger = __data.getULONG();
            }
            else if (__sizeof_ == sizeof_BIGINT) {
                __sqlLen = __data.getUSHORT();
                __data.get(__sqlBigInt, __sqlLen);
                __sqlBigInt[__sqlLen] = '\0';
            }
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        if (__sizeof_ == sizeof_SMALLINT) {
            binding->cType = SQL_SMALLINT;
            binding->sqlPtr = &__sqlSmallInt;
        }
        else if (__sizeof_ == sizeof_INTEGER) {
            binding->cType = SQL_INTEGER;
            binding->sqlPtr = &__sqlInteger;
        }
        else if (__sizeof_ == sizeof_BIGINT) {
            binding->cType = SQL_C_CHAR;
            binding->sqlPtr = __sqlBigInt;
        }
        binding->sqlLen = __sizeof_;
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        default:
            __data.putUCHAR(0);
            if (__sizeof_ == sizeof_SMALLINT) {
                __data.putUSHORT(__sqlSmallInt);
            }
            else if (__sizeof_ == sizeof_INTEGER) {
                __data.putULONG(__sqlInteger);
            }
            else if (__sizeof_ == sizeof_BIGINT)
                __data.putString(__sqlBigInt, (size_t)__sqlLen);
            break;
        }
    }

    virtual string __get_string()
    {
        if (__isNull)
            return "NULL";

        char str[MAX_COLUMNS_BIGINT + 1] = "";
        if (__sizeof_ == sizeof_SMALLINT) {
            sprintf_s(str, MAX_COLUMNS_BIGINT + 1, "%hd", __sqlSmallInt);
        }
        if (__sizeof_ == sizeof_INTEGER) {
            sprintf_s(str, MAX_COLUMNS_BIGINT + 1, "%d", __sqlInteger);
        }
        else if (__sizeof_ == sizeof_BIGINT)
            sprintf_s(str, MAX_COLUMNS_BIGINT + 1, "%s", __sqlBigInt);

        return str;
    }

    SQLULEN     __sizeof_;
    bool        __nullable;

    SQLLEN      __sqlLen;
    SQLSMALLINT __sqlSmallInt;
    SQLINTEGER  __sqlInteger;
    bufferCls   __sqlBigInt[sizeof_BIGINT];
};

class BindDouble : public IBinding {
public:
    static const int sizeof_SMALLINT = 2;
    static const int sizeof_INTEGER = 4;
    static const int sizeof_BIGINT = 20;

    static const int MAX_COLUMNS_DOUBLE = 24;

    BindDouble(SQLULEN sizeof_, bool nullable) : __sizeof_(sizeof_), __nullable(nullable) { }

protected:
    virtual bool __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            __data.get((bufferCls *)&__sqlDouble, __sqlLen);
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        binding->cType = SQL_DOUBLE;
        binding->sqlPtr = &__sqlDouble;
        binding->sqlLen = __sizeof_;
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        default:
            __data.putUCHAR(0);
            __data.put((bufferCls *)&__sqlDouble, sizeof(double));
            break;
        }
    }

    virtual string __get_string()
    {
        if (__isNull)
            return "NULL";

        char str[MAX_COLUMNS_DOUBLE + 1] = "";
        sprintf_s(str, MAX_COLUMNS_DOUBLE + 1, "%f", __sqlDouble);

        return str;
    }

    SQLULEN     __sizeof_;
    bool        __nullable;

    SQLLEN      __sqlLen;
    double      __sqlDouble;
};

class BindDate : public IBinding {
public:
    static const int MAX_COLUMNS_DATE = 10;

    BindDate(SQLULEN sizMax, bool nullable) : __sizMax(sizMax), __nullable(nullable) {}

protected:
    virtual bool __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            __data.get((bufferCls *)&__sqlds, sizeof(SQL_DATE_STRUCT));
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        binding->cType = SQL_C_DATE;
        binding->sqlPtr = &__sqlds;
        binding->sqlLen = sizeof(SQL_DATE_STRUCT);
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        default:
            __data.putUCHAR(0);
            __data.put((bufferCls *)&__sqlds, sizeof(SQL_DATE_STRUCT));
            break;
        }
    }

    virtual string __get_string()
    {
        char str[MAX_COLUMNS_DATE + 1] = "";
        sprintf_s(str, MAX_COLUMNS_DATE + 1, "%04d-%02d-%02d", __sqlds.year, __sqlds.month, __sqlds.day);
        return str;
    }

    SQLULEN         __sizMax;
    bool            __nullable;

    SQL_DATE_STRUCT __sqlds;
    SQLLEN          __sqlLen;
};

class BindTime : public IBinding {
public:
    static const int MAX_COLUMNS_TIME = 8;

    BindTime(SQLULEN sizMax, bool nullable) : __sizMax(sizMax), __nullable(nullable) {}

protected:
    virtual bool __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            __data.get((bufferCls *)&__sqlts, sizeof(SQL_TIME_STRUCT));
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        binding->cType = SQL_C_TIME;
        binding->sqlPtr = &__sqlts;
        binding->sqlLen = sizeof(SQL_TIME_STRUCT);
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        default:
            __data.putUCHAR(0);
            __data.put((bufferCls *)&__sqlts, sizeof(SQL_TIME_STRUCT));
            break;
        }
    }

    virtual string __get_string()
    {
        char str[MAX_COLUMNS_TIME + 1] = "";
        sprintf_s(str, MAX_COLUMNS_TIME + 1, "%02d:%02d:%02d", __sqlts.hour, __sqlts.minute, __sqlts.second);
        return str;
    }

    SQLULEN         __sizMax;
    bool            __nullable;

    SQL_TIME_STRUCT __sqlts;
    SQLLEN          __sqlLen;
};

class BindDateTime : public IBinding {
public:
    static const int MAX_COLUMNS_TIMESTAMP = 19;

    BindDateTime(SQLULEN sizMax, bool nullable) : __sizMax(sizMax), __nullable(nullable) {}

protected:
    virtual bool __next()
    {
        if (__data.eof())
            return false;

        if (__data.getUCHAR() == NULL_DATA) {
            __isNull = true;
        }
        else {
            __data.get((bufferCls *)&__sqltss, sizeof(SQL_TIMESTAMP_STRUCT));
            __isNull = false;
        }
        return true;
    }

    virtual bool __alloc_bind(ODBC_Binding *binding)
    {
        binding->cType = SQL_C_TIMESTAMP;
        binding->sqlPtr = &__sqltss;
        binding->sqlLen = sizeof(SQL_TIMESTAMP_STRUCT);
        binding->sqlLenPtr = &__sqlLen;
        return true;
    }

    virtual void __push_back()
    {
        switch (__sqlLen) {
        case SQL_NULL_DATA:
            __data.putUCHAR(NULL_DATA);
            break;
        default:
            __data.putUCHAR(0);
            __data.put((bufferCls *)&__sqltss, sizeof(SQL_TIMESTAMP_STRUCT));
            break;
        }
    }

    virtual string __get_string()
    {
        char str[MAX_COLUMNS_TIMESTAMP + 1] = "";
        sprintf_s(str, MAX_COLUMNS_TIMESTAMP + 1, "%04d-%02d-%02d %02d:%02d:%02d", __sqltss.year, __sqltss.month, __sqltss.day, 
            __sqltss.hour, __sqltss.minute, __sqltss.second);
        return str;
    }

    SQLULEN                 __sizMax;
    bool                    __nullable;

    SQL_TIMESTAMP_STRUCT    __sqltss;
    SQLLEN                  __sqlLen;
};

class ODBC_ResultSet {
    friend class ODBC_Wrapper;

public:
    ODBC_ResultSet() : __resultRows(0) {}

    void clear() {
        int i = 0;
        __resultCols.clear();
        for (; i < __bindings.size(); i++) {
            delete __bindings[i];
        }
        __bindings.clear();
        __resultRows = 0;
    }

    void first() 
    {
        for (int i = 0; i < __bindings.size(); i++) {
            if (__bindings[i])
                __bindings[i]->__reset();
        }
    }

    bool next()
    {
        if (!__bindings.size())
            return false;

        for (int i = 0; i < __bindings.size(); i++) {
            if (__bindings[i])
                if(!__bindings[i]->__next())
                    return false;
        }
        return true;
    }

    void setUpdateCount(SQLLEN updateCount)
    {
        __resultRows = updateCount;
    }

    SQLLEN getRowCount()
    {
        return __resultRows;
    }

    string getString(SQLSMALLINT colIdx) 
    {
        int idx = colIdx - 1;
        if (__bindings[idx])
            return __bindings[idx]->__get_string();
        return "";
    }

    size_t getColumnCount() 
    {
        return __resultCols.size();
    }

    SQLLEN getColumnDisplaySize(int colIdx)
    {
        int idx = colIdx - 1;
        return __resultCols[idx].displayLen();
    }

    string& getColumnLabel(int colIdx)
    {
        int idx = colIdx - 1;
        return __resultCols[idx].colName();
    }

    int getColumnType(int colIdx)
    {
        int idx = colIdx - 1;
        return __resultCols[idx].sqlType();
    }

    bool saveAsCSV(string& file)
    {
        string colval;
        ofstream os(file, ofstream::out | ofstream::trunc);
        if (!os.is_open())
            return false;
        first();
        while (next()) {
            for (int idx = 1; idx <= getColumnCount(); idx++) {
                if (getColumnType(idx) == SQL_VARCHAR) {
                    colval = '"' + __copyEscapedString(getString(idx)) + '"';
                }
                else
                    colval = getString(idx);
                os << colval;
                if (idx != getColumnCount())
                    os << ',';
            }
            os << "\n";
        }
        os.close();
        return true;
    }

protected:
    void __pushResultCol(ODBC_ResultCol& resultCol)
    {
        __resultCols.push_back(resultCol);
        __bindings.push_back(NULL);
    }

    bool __allocBindResult(SQLSMALLINT colIdx, ODBC_Binding *binding)
    {
        int i = 0;
        for (; i < __resultCols.size(); i++) {
            if (__resultCols[i].colIdx() == colIdx)
                break;
        }
        if (i == __resultCols.size())
            return false;
        delete __bindings[i];
        switch (__resultCols[i].sqlType()) {
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
            __bindings[i] = new BindInteger(__resultCols[i].octetLen(), __resultCols[i].nullable());
            break;
        case SQL_DOUBLE:
            __bindings[i] = new BindDouble(__resultCols[i].octetLen(), __resultCols[i].nullable());
            break;
        case SQL_DATE:
            __bindings[i] = new BindDate(__resultCols[i].colLength(), __resultCols[i].nullable());
            break;
        case SQL_TIME:
            __bindings[i] = new BindTime(__resultCols[i].colLength(), __resultCols[i].nullable());
            break;
        case SQL_TIMESTAMP:
            __bindings[i] = new BindDateTime(__resultCols[i].colLength(), __resultCols[i].nullable());
            break;
        case SQL_CHAR:
        case SQL_WCHAR:
        case SQL_VARCHAR:
        case SQL_WVARCHAR:
            __bindings[i] = new BindString(__resultCols[i].colLength(), __resultCols[i].nullable());
            break;
        default:
            break;
        }
        return __bindings[i]->__alloc_bind(binding);
    }

    void __latchBindResult()
    {
        for (int i = 0; i < __bindings.size(); i++) {
            if (__bindings[i])
                __bindings[i]->__push_back();
        }
        __resultRows++;
    }

    string __copyEscapedString(string& src)
    {
        string dest;
        string::iterator itr;
        for (itr = src.begin(); itr != src.end(); ++itr) {
            if (*itr == '"') {
                dest += "\"\"";
            }
            else
                dest += *itr;
        }
        return dest;
    }

    ODBC_ResultCols     __resultCols;
    vector<IBinding*>   __bindings;

    SQLLEN              __resultRows;
};

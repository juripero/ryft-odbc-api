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
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

namespace RYFT_ODBCWrap {

    #include "error.h"
    #include "resultset.h"
    #include "catalog.h"

    class ODBC_Wrapper {
    public:
        ODBC_Wrapper();
       ~ODBC_Wrapper();

        bool connect(string& dsn, string& uid, string& pwd);
        void disconnect();
        bool isConnected();

        bool describeCatalog(ODBC_ResultSet& tables);
        bool describeTable(string& table, ODBC_ResultSet& columns);
        bool describeDatatypes(ODBC_ResultSet& datatypes);
        bool describeProcedures(ODBC_ResultSet& procedures);

        bool execQuery(string& query, ODBC_ResultSet& resultSet);

        ODBC_Errors odbcErrors();

    private:
        SQLHENV     __env;
        bool        __initialized;

        SQLHDBC     __dbc;
        bool        __connected;

        SQLHSTMT    __stmt;

        ODBC_Errors __odbcErrors;
        void        __odbcError();

        bool        __processResultSet(ODBC_ResultSet& resultSet);
    };
};
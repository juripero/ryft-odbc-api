// sql_console.cpp : Defines the entry point for the console application.
//
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
#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#include "wrapper.h"
using namespace RYFT_ODBCWrap;

class Utils {
public:
    static void tell(string& str) 
    {
        cout << str;
    }
    static void tell(char *str) 
    {
        cout << str;
    }
    static void tell(int a) 
    {
        cout << a;
    }
    static void tell(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

class ICommandProcessor {
public:
    virtual bool canHandle(string& cmd) = 0;

    virtual bool process(vector<string> args) = 0;

    virtual void help() = 0;
};
vector<ICommandProcessor *> __commandProcs;

ODBC_Wrapper __odbcWrap;
ODBC_ResultSet __resultSet;

string __prompt = "disconnected";

#define NO_LIMIT -1
SQLLEN __limit = -1;
SQLLEN __columns = -1;

inline bool getinput(string& prompt, string& in)
{
    cout << "[" + prompt + "] ";
    getline(cin, in);
    return !in.empty();
}

void printHello(char *app)
{
    char *exe = strrchr(app, '\\');
    if (exe)
        cout << (exe + 1) + string(":");
    Utils::tell("RYFT - SDK Sample Application\n");
    Utils::tell(".help for more information\n");
}

void dumpTable(ODBC_ResultSet& resultSet)
{
    string str;
    SQLULEN limit = __limit;
    SQLULEN columns = __columns;
    vector<SQLULEN> colDispWidth;
    int colCount = (int)resultSet.getColumnCount();
    if (!colCount)
        return;
    int idx;
    for (idx = 0; idx < colCount; idx++) {
        str = resultSet.getColumnLabel(idx + 1);
        SQLULEN colDisp = max(resultSet.getColumnDisplaySize(idx + 1), (int)str.length());
        colDispWidth.push_back(min(colDisp,columns));
    }

    Utils::tell("\n");
    for (idx = 0; idx < colCount; idx++) {
        str = "+";
        str.append(colDispWidth[idx], '-');
        Utils::tell(str);
    }
    Utils::tell("+\n");
    for (idx = 0; idx < colCount; idx++) {
        str = "|";
        string label = resultSet.getColumnLabel(idx + 1);
        if (label.length() > colDispWidth[idx]) {
            str.append(label, 0, colDispWidth[idx] - 1);
            str.append("&");
        }
        else {
            str.append(label);
            str.append(colDispWidth[idx] - label.length(), ' ');
        }
        Utils::tell(str);
    }
    Utils::tell("|\n");
    for (idx = 0; idx < colCount; idx++) {
        str = "+";
        str.append(colDispWidth[idx], '-');
        Utils::tell(str);
    }
    Utils::tell("+\n");

    for (resultSet.first(); limit && resultSet.next(); --limit) {
        for (idx = 0; idx < colCount; idx++) {
            str = "|";
            string value = resultSet.getString(idx + 1);
            if (value.length() > colDispWidth[idx]) {
                str.append(value, 0, colDispWidth[idx]-1);
                str.append("&");
            }
            else {
                str.append(value);
                str.append(colDispWidth[idx] - value.length(), ' ');
            }
            Utils::tell(str);
        }
        Utils::tell("|\n");
    }
    for (idx = 0; idx < colCount; idx++) {
        str = "+";
        str.append(colDispWidth[idx], '-');
        Utils::tell(str);
    }
    Utils::tell("+\n");
}

inline vector<string> tokenize(string& in, char c = ' ')
{
    bool in_quotes = false;
    vector<string> tokens;
    string token;

    const char *str = in.c_str();
    while (*str) {
        for (; *str; str++) {
            if (*str == '\'') {
                if (*(str + 1) == '\'') {
                    str++;
                }
                else { // if not a literal
                    in_quotes = !in_quotes;
                    continue;
                }
            }
            if (*str == c && !in_quotes) {
                str++;
                break;
            }
            token += *str;
        }
        tokens.push_back(token);
        token.clear();
    }
    return tokens;
}

class HelpProc : public ICommandProcessor {
public:
    virtual bool canHandle(string& cmd)
    {
        if (cmd == ".help") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args) 
    {
        Utils::tell("\nSupported commands:\n\n");
        vector<ICommandProcessor *>::iterator commandProcsItr;
        for (commandProcsItr = __commandProcs.begin(); commandProcsItr != __commandProcs.end(); commandProcsItr++)
            (*commandProcsItr)->help();
        Utils::tell("\nAny other input not preceeded with '.' is treated as SQL and passed along to the current connection.\n\n");
        return true;
    }

    virtual void help() 
    {
        Utils::tell("  .help\t\t\t-- displays this message\n");
    }
};

class CloseProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".close") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        __odbcWrap.disconnect();
        __prompt = "disconnected";
        return true;
    }

    virtual void help()
    {
        Utils::tell("  .close\t\t-- closes current connection\n");
    }
};

class OpenProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".open") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        string str;
        if (args.size() < 2) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        if (args.size() < 3) {
            str.clear();
            args.push_back(str);
        }
        if (args.size() < 4) {
            str.clear();
            args.push_back(str);
        }
        bool ret = __odbcWrap.connect(args[1], args[2], args[3]);
        if (__odbcWrap.isConnected())
            __prompt = args[1];
        return ret;
    }

    virtual void help()
    {
        Utils::tell("  .open DSN [UID] [PWD]\t-- opens a connection to DSN using UID and PWD\n");
    }
};

class CatalogProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".catalog") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        ODBC_ResultSet tables;
        bool ret = __odbcWrap.describeCatalog(tables);
        if (ret) {
            dumpTable(tables);
            Utils::tell("Returned %d row(s)\n", tables.getRowCount());
        }
        return ret;
    }

    virtual void help()
    {
        Utils::tell("  .catalog\t\t-- lists tables\n");
    }
};

class DescribeProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".describe") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        if (args.size() < 2) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        ODBC_ResultSet columns;
        bool ret = __odbcWrap.describeTable(args[1], columns);
        if (ret) {
            dumpTable(columns);
            Utils::tell("Returned %d row(s)\n", columns.getRowCount());
        }
        return ret;
    }

    virtual void help()
    {
        Utils::tell("  .describe TABLE\t-- lists columns in TABLE\n");
    }
};

class DatatypesProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".datatypes") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        ODBC_ResultSet datatypes;
        bool ret = __odbcWrap.describeDatatypes(datatypes);
        if (ret) {
            dumpTable(datatypes);
            Utils::tell("Returned %d row(s)\n", datatypes.getRowCount());
        }
        return ret;
    }

    virtual void help()
    {
        Utils::tell("  .datatypes\t\t-- lists datatypes\n");
    }
};

class ProceduresProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".procedures") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        ODBC_ResultSet procedures;
        bool ret = __odbcWrap.describeProcedures(procedures);
        if (ret) {
            dumpTable(procedures);
            Utils::tell("Returned %d row(s)\n", procedures.getRowCount());
        }
        return ret;
    }

    virtual void help()
    {
        Utils::tell("  .procedures\t\t-- lists procedures\n");
    }
};

class LimitProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".limit") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        if (args.size() < 2) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        else
            __limit = atoi(args[1].c_str());
        Utils::tell("current limit set to %d\n", (int)__limit);
        return true;
    }

    virtual void help()
    {
        Utils::tell("  .limit LIMIT\t\t-- sets row output limit to LIMIT(-1 = full)\n");
    }
};

class ColumnsProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".columns") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        if (args.size() < 2) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        else
            __columns = atoi(args[1].c_str());
        Utils::tell("current column limit set to %d\n", (int)__columns);
        return true;
    }

    virtual void help()
    {
        Utils::tell("  .columns LIMIT\t-- sets column output limit to LIMIT(-1 = full)\n");
    }
};

class ExportProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".export") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        ODBC_ResultSet resultSet;
        if (args.size() < 3) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        else {
            DWORD fetchStart = GetTickCount();
            bool ret = __odbcWrap.execQuery(args[1], resultSet);
            DWORD fetchEnd = GetTickCount();
            if (ret) {
                if (resultSet.getColumnCount()) {
                    resultSet.saveAsCSV(args[2]);
                    Utils::tell("exported %d row(s) in %d milliseconds\n", (int)resultSet.getRowCount(), (int)(fetchEnd - fetchStart));
                }
                else
                    Utils::tell("no result set, could not export\n");
            }
            else {
                ODBC_Errors odbcErrors = __odbcWrap.odbcErrors();
                ODBC_Errors::iterator itr;
                for (itr = odbcErrors.begin(); itr != odbcErrors.end(); itr++) {
                    Utils::tell("sqlState = " + itr->sqlState() + "\n");
                    Utils::tell(itr->sqlMessage() + "\n");
                }
            }
        }
        return true;
    }

    virtual void help()
    {
        Utils::tell("  .export SQL FILE\t-- save SQL as .csv FILE\n");
    }
};

class RunProc : public ICommandProcessor {
public:
    virtual bool canHandle(string &cmd)
    {
        if (cmd == ".run") {
            return true;
        }
        return false;
    }

    virtual bool process(vector<string> args)
    {
        ODBC_ResultSet resultSet;
        if (args.size() < 2) {
            Utils::tell(args[0] + " - invalid usage\n");
            help();
            return false;
        }
        else {
            ifstream is(args[1]);
            string in_line;
            if (!is.is_open()) {
                Utils::tell("could not open file \'" + args[1] + "'\n");
                return false;
            }
            while (!is.eof()) {
                getline(is, in_line);
                DWORD fetchStart = GetTickCount();
                bool ret = __odbcWrap.execQuery(in_line, resultSet);
                DWORD fetchEnd = GetTickCount();
                if (ret) {
                    if (resultSet.getColumnCount()) {
                        dumpTable(resultSet);
                        Utils::tell("returned %d row(s) in %d milliseconds\n", (int)resultSet.getRowCount(), (int)(fetchEnd - fetchStart));
                    }
                    else
                        Utils::tell("%d row(s) updated in %d milliseconds\n", (int)resultSet.getRowCount(), (int)(fetchEnd - fetchStart));
                }
                else {
                    ODBC_Errors odbcErrors = __odbcWrap.odbcErrors();
                    ODBC_Errors::iterator itr;
                    for (itr = odbcErrors.begin(); itr != odbcErrors.end(); itr++) {
                        Utils::tell("sqlState = " + itr->sqlState() + "\n");
                        Utils::tell(itr->sqlMessage() + "\n");
                    }
                }
            }
            is.close();
        }
        return true;
    }

    virtual void help()
    {
        Utils::tell("  .run FILE\t\t-- executes SQL from FILE\n");
    }
};

int main(int argc, char* argv[])
{
    string in;
    ODBC_ResultSet resultSet;
    vector<ICommandProcessor *>::iterator commandProcsItr;

    printHello(argv[0]);

    __commandProcs.push_back(new HelpProc());
    __commandProcs.push_back(new OpenProc());
    __commandProcs.push_back(new CloseProc());
    __commandProcs.push_back(new CatalogProc());
    __commandProcs.push_back(new DescribeProc());
    __commandProcs.push_back(new DatatypesProc());
    __commandProcs.push_back(new ProceduresProc());
    __commandProcs.push_back(new LimitProc());
    __commandProcs.push_back(new ColumnsProc());
    __commandProcs.push_back(new ExportProc());
    __commandProcs.push_back(new RunProc());

    bool didExecute;
    while(getinput(__prompt, in)) {
        didExecute = true;
        if (in[0] == '.') {
            // parse commands
            vector<string> args = tokenize(in);
            for (commandProcsItr = __commandProcs.begin(); commandProcsItr != __commandProcs.end(); commandProcsItr++) {
                if ((*commandProcsItr)->canHandle(args[0])) {
                    didExecute = (*commandProcsItr)->process(args);
                    break;
                }
            }
            if (commandProcsItr == __commandProcs.end()) {
                Utils::tell(args[0] + " - invalid command\n");
                Utils::tell(".help for more information\n");
            }
        }
        else {
            // SQL
            DWORD fetchStart = GetTickCount();
            didExecute = __odbcWrap.execQuery(in, resultSet);
            DWORD fetchEnd = GetTickCount();
            if (didExecute) {
                if (resultSet.getColumnCount()) {
                    dumpTable(resultSet);
                    Utils::tell("returned %d row(s) in %d milliseconds\n", (int)resultSet.getRowCount(), (int)(fetchEnd - fetchStart));
                }
                else
                    Utils::tell("%d row(s) updated in %d milliseconds\n", (int)resultSet.getRowCount(), (int)(fetchEnd - fetchStart));
            }
        }

        if (!didExecute) {
            ODBC_Errors odbcErrors = __odbcWrap.odbcErrors();
            ODBC_Errors::iterator itr;
            for (itr = odbcErrors.begin(); itr != odbcErrors.end(); itr++) {
                Utils::tell("sqlState = " + itr->sqlState() + "\n");
                Utils::tell(itr->sqlMessage() + "\n");
            }
        }
    }
    __odbcWrap.disconnect();
	return 0;
}

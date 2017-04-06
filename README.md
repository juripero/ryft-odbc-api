# ryft-odbc-api

The RYFT ODBC API project contains sample sources to demonstrate the programmatic use of the ODBC 
and JDBC interfaces when connecting to a RYFT ODBC server installation. The project contains two samples, 
both of which implement an “SQL Console”, a CLI capable of performing queries against the RYFT ODBC 
server data source and displaying the result sets that are returned by the server.

## c++ SQL Console (cpp_example)

The c++ example was developed on a Windows environment and uses the RYFT ODBC driver installation to connect to the server backend. There are two components which make up the example code:

1.	sql_console – project and source files for the CLI interface
2.	ryft_odbc_cpp_sdk – project and source files for a c++ library which roughly mirror the class and API set for JDBC data source and result set management.

Project and solution files for Visual Studio 2013 are provided with the sample in the /vs2013 subdirectory.

### CLI Interface

Help is available from the command-line interface by typing ```.help```

```
[disconnected] .help

Supported commands:

  .help                 -- displays this message
  .open DSN [UID] [PWD] -- opens a connection to DSN using UID and PWD
  .close                -- closes current connection
  .catalog              -- lists tables
  .describe TABLE       -- lists columns in TABLE
  .datatypes            -- lists datatypes
  .procedures           -- lists procedures
  .limit LIMIT          -- sets row output limit to LIMIT(-1 = full)
  .export SQL FILE      -- save SQL as .csv FILE
  .run FILE             -- executes SQL from FILE

Any other input not preceeded with '.' is treated as SQL and passed along to the current connection.

[disconnected]
```

NOTE: You can test query performance by issuing the ```.limit``` command with a small limit and then executing the query you wish to performance test. The query will be fully executed, however, only the number of lines specified in your limit will be displayed to the screen.

```
[A Ryft] .limit 1
current limit set to 1
[A Ryft] select date_of_birth from Passengers where Name like '-h2(Michele)'

+--------------------------------------------------------------------------------+
|Date_of_Birth                                                                   |
+--------------------------------------------------------------------------------+
|07-12-1959                                                                      |
+--------------------------------------------------------------------------------+
returned 3 row(s) in 515 milliseconds
[A Ryft]
```

## JAVA Sample (java_example)

The JAVA sample consists of a single JAVA project which also presents a CLI version of an SQL console application, however, the 
syntax and usage of this interface differs slightly from the c++ SQL console.

The JAVA sample makes use of the Cliche Command Line Shell Project (https://code.google.com/archive/p/cliche/). This project is licensed under the MIT License (https://opensource.org/licenses/mit-license.php).

The JAVA sample sources were developed in the Eclipse JEE IDE and Eclipse project files are provided for your use. 

NOTE: Both the Cliche and Ryft JDBC Client JAR files are Referenced Libraries. You can create a single JAR containing all required classes by executing the Eclipse Export command and exporting a "Runnable JAR file" from the Java export destinations.

### CLI Interface

To start the JAVA CLI, open a command window by running cmd.exe. In the command you can execute the JAR by typing:
```
D:\Ryft>java -jar sql-console.jar
```

Available commands will be listed to the CLI using ```?list```:
```
?list to list commands
[cmd]> ?list
abbrev  name    params
l       limit   (limit)
c       close   ()
o       open    (ip_and_addr, username, password)
o       open    ()
o       open    (ip_and_addr)
ca      catalog ()
d       describe        (table)
d       datatypes       ()
p       procedures      ()
s       sql     (query)
e       export  (query, csvFile)
[cmd]>
```

Help on a particular command is available by typing ```help``` and the command name:
```
[cmd]> ?help sql
Command: sql
Abbrev:  s
Params:  (query)
Description: executes SQL query
Number of parameters: 1
query   String  valid SQL query
[cmd]>
```

To open the RYFT data source, you must provide the IP address and the port number for the ODBC server (the default server port for an ODBC installation is port 7409):
```
[cmd]> open '172.16.13.3:7409' myuser mypassword
connected
```

In order to execute SQL in the JAVA CLI, you must issue the ```sql``` command. The SQL statement itself must be enclosed
in single or double quotes:
```
[cmd]> limit 1
limit set to 1
[cmd]> sql 'select date_of_birth from Passengers where Name like ''-h2(Michele)'''
+--------------------------------------------------------------------------------+
|DoB                                                                             |
+--------------------------------------------------------------------------------+
|07-12-1959                                                                      |
+--------------------------------------------------------------------------------+
returned 3 row(s) in 491 milliseconds
[cmd]>
```

To exit the JAVA CLI type ```exit```.

## Release Notes
** 1.0.0 (April 2, 2017)
* Initial release of the sample sources

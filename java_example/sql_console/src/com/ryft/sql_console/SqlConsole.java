// SqlConsole.java : SQL console application class
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
package com.ryft.sql_console;

import java.io.BufferedReader;
import java.io.FileWriter;
import java.io.InputStreamReader;

import asg.cliche.*;

import java.sql.*;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.lang.Math;

public class SqlConsole {

	static final String RYFT_DRIVER = "com.simba.client.core.jdbc42.SCJDBC42Driver";
	static final String CONNECT_URL = "jdbc:simba://";

	private Connection __dbc;
	
	private boolean __isConnected = false;
	
	private static int __limit = -1;
	
	private String prompt(String prompt) {
		String line = null;
		System.out.print("[" + prompt + "] ");
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		try {
			line = in.readLine();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return line;
	}
	
	@Command(description="open a datasource")
	public void open() {
		String ip = prompt("ip address:port");
		String uid = prompt("username");
		String pwd = prompt("password");
		open(ip, uid, pwd);
	}
	
	@Command(description="open a datasource - use this form of open if your datasource does not require authentication")
	public void open(@Param(name="ip_and_addr", description="ipAddress:port of datasource") String ipAndAddr) {
		open(ipAndAddr, null, null);
	}
	
	@Command(description="open a datasource - use this form of open if your datasource requires authentication")
	public void open(@Param(name="ip_and_addr", description="ip address:port of datasource") String ipAndPort, 
		@Param(name="username", description="User name for datasource") String uid, 
		@Param(name="password", description="User password for datasource") String pwd) {
		
		String url = CONNECT_URL + ipAndPort;
		try {
			// register
			Class.forName(RYFT_DRIVER);
			// open
			__dbc = DriverManager.getConnection(url, uid, pwd);
			
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
			return;
		} catch (SQLException e) {
			__sqlErrors(e);
			return;
		}
		__isConnected = true;
		System.out.print("connected\n");
	}
	
	@Command(description="close a datasource")
	public void close() {
		try {
			__dbc.close();
		} catch (SQLException e) {
			__sqlErrors(e);
			return;
		}
		__isConnected = false;
		System.out.print("disconnected\n");
	}
	
	@Command(description="lists tables in datasource")
	public void catalog() {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		try {
			DatabaseMetaData meta = __dbc.getMetaData();
			ResultSet rs = meta.getTables(null, null,"%", null);
			__dumpTable(rs, true);
		} catch (SQLException e) {
			__sqlErrors(e);
		}		
	}
	
	@Command(description="lists columns in TABLE") 
	public void describe(@Param(name="table",description="table to retrieve metadata information") String table) {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		try {
			DatabaseMetaData meta = __dbc.getMetaData();
			ResultSet rs = meta.getColumns(null, null, table, null);
			__dumpTable(rs, true);
		} catch (SQLException e) {
			__sqlErrors(e);
		}		
	}

	@Command(description="lists datatypes supported by datasource") 
	public void datatypes() {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		try {
			DatabaseMetaData meta = __dbc.getMetaData();
			ResultSet rs = meta.getTypeInfo();
			__dumpTable(rs, true);
		} catch (SQLException e) {
			__sqlErrors(e);
		}
	}
	
	@Command(description="lists procedures supported by datasource") 
	public void procedures() {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		try {
			DatabaseMetaData meta = __dbc.getMetaData();
			ResultSet rs = meta.getProcedures(null,null,null);
			__dumpTable(rs, true);
		} catch (SQLException e) {
			__sqlErrors(e);
		}		
	}

	@Command(description="executes SQL query")
	public void sql(@Param(name="query",description="valid SQL query") String query) {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		try {
			long end_time;
			long start_time = System.currentTimeMillis();
			Statement stmt = __dbc.createStatement();
			if(stmt.execute(query)) {
				ResultSet rs = stmt.getResultSet();
				end_time = System.currentTimeMillis();
				int rowCount = __dumpTable(rs, false);
				System.out.print("returned " + rowCount + " row(s) in " + 
						(end_time - start_time) + " milliseconds\n");
			}
			else {
				end_time = System.currentTimeMillis();
				System.out.print(stmt.getUpdateCount() +" row(s) inserted/updated/deleted in " + 
						(end_time - start_time) + " milliseconds\n");
			}
			stmt.close();
		} catch (SQLException e) {
			__sqlErrors(e);
		}		
	}
	
	@Command(description="exports result set to CSV file")
	public void exportCSV(@Param(name="query",description="valid SQL query") String query, 
			@Param(name="csvFile",description="fully qualified path to CSV file") String csvFile) {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		long end_time;
		long start_time = System.currentTimeMillis();
		try {
			Statement stmt = __dbc.createStatement();
			ResultSet rs = stmt.executeQuery(query);
			end_time = System.currentTimeMillis();
			int rowCount = __saveAsCSV(csvFile, rs);
			System.out.print("exported " + rowCount + " row(s) in " + 
					(end_time - start_time) + " milliseconds\n");
			stmt.close();
		} catch (SQLException e) {
			__sqlErrors(e);
		}			
	}

	@Command(description="exports result set to JSON file")
	public void exportJSON(@Param(name="query",description="valid SQL query") String query, 
			@Param(name="jsonFile",description="fully qualified path to JSON file") String jsonFile) {
		if(!__isConnected) {
			System.out.print("disconnected\n");
			return;
		}
		long end_time;
		long start_time = System.currentTimeMillis();
		try {
			Statement stmt = __dbc.createStatement();
			ResultSet rs = stmt.executeQuery(query);
			end_time = System.currentTimeMillis();
			int rowCount = __saveAsJSON(jsonFile, rs);
			System.out.print("exported " + rowCount + " row(s) in " + 
					(end_time - start_time) + " milliseconds\n");
			stmt.close();
		} catch (SQLException e) {
			__sqlErrors(e);
		}			
	}	
	
	@Command(description="sets row output limit to LIMIT(-1 = full)") 
	public void limit(@Param(name="limit",description="limit of result set rows to output") int limit) {
		__limit = limit;
		System.out.print("limit set to " + limit + "\n");
	}
	
	public static void main(String[] args) throws IOException {
		ShellFactory.createConsoleShell("[cmd]", "?list to list commands", new SqlConsole())
			.commandLoop();
	}
	
	private int __dumpTable(ResultSet rs, boolean no_limit) {
		int idx;
		String str;
		int colCount = 0;
		int rowCount = 0;
		int limit = __limit;
		ResultSetMetaData rsmd = null;
		if(no_limit == true)
			limit = -1;
		try {
			rsmd = rs.getMetaData();
			colCount = rsmd.getColumnCount();
			if(colCount == 0)
				return 0;
			
			int[] colDispWidth = new int[colCount];
			for(idx = 0; idx < colCount; idx++) {
				str = rsmd.getColumnLabel(idx + 1);
				int colDisp = Math.max(rsmd.getColumnDisplaySize(idx +1), str.length());
				colDispWidth[idx] = colDisp;
			}
			System.out.print("\n");
		
			for (idx = 0; idx < colCount; idx++) {
				str = "+";
				char[] repeat = new char[colDispWidth[idx]];
				Arrays.fill(repeat,  '-');
				str += new String(repeat);
				System.out.print(str);
			}
			System.out.print("+\n");
			
			for (idx = 0; idx < colCount; idx++) {
				str = "|";
				String label = null;
				label = rsmd.getColumnLabel(idx + 1);
				str += label;
				char[] repeat = new char[colDispWidth[idx] - label.length()];
				Arrays.fill(repeat, ' ');
				str += new String(repeat);
				System.out.print(str);
			}
			System.out.print("|\n");
			
			for (idx = 0; idx < colCount; idx++) {
				str = "+";
				char[] repeat = new char[colDispWidth[idx]];
				Arrays.fill(repeat,  '-');
				str += new String(repeat);
				System.out.print(str);
			}
			System.out.print("+\n");
			
			while (rs.next()) {
				rowCount++;
				if(limit-- != 0) {
					for (idx = 0; idx < colCount; idx++) {
						str = "|";
						String value = rs.getString(idx + 1);
						if(value == null) {
							value = "NULL";
						}
						str += value;
						char[] repeat = new char[colDispWidth[idx] - value.length()];
						Arrays.fill(repeat, ' ');
						str += new String(repeat);
						System.out.print(str);
					}
					System.out.print("|\n");
				}
			}
			
			for (idx = 0; idx < colCount; idx++) {
				str = "+";
				char[] repeat = new char[colDispWidth[idx]];
				Arrays.fill(repeat,  '-');
				str += new String(repeat);
				System.out.print(str);
			}		
			System.out.print("+\n");
			
		} catch (SQLException e) {
			__sqlErrors(e);
		}
		return rowCount; 
	}
	
	private int __saveAsCSV(String csvFile, ResultSet rs) {
		int idx;
		int colCount = 0;
		int rowCount = 0;
		try {
			colCount = rs.getMetaData().getColumnCount();
			if(colCount == 0)
				return 0;
		
			FileWriter writer = null;
			writer = new FileWriter(csvFile);
			List<String> values = new ArrayList<String>();

			List<String> headers = new ArrayList<String>();
			for(idx = 0; idx < colCount; idx++) {
				String header;
				header = rs.getMetaData().getColumnLabel(idx + 1);
				headers.add(header);
			}	
			CSVUtils.writeLine(writer, headers);
			
			while(rs.next()) {
				rowCount++;
				values.clear();
				for(idx = 0; idx < colCount; idx++) {
					String value;
					value = rs.getString(idx+1);
					values.add(value);
				}
				CSVUtils.writeLine(writer, values);
			}
			writer.close();
		} catch (SQLException e) {
			__sqlErrors(e);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return rowCount;
	}
	
	private int __saveAsJSON(String jsonFile, ResultSet rs) {
		int idx;
		int colCount = 0;
		int rowCount = 0;
		try {
			colCount = rs.getMetaData().getColumnCount();
			if(colCount == 0)
				return 0;
		
			FileWriter writer = null;
			writer = new FileWriter(jsonFile);
			writer.append("[\r\n");
			
			List<String> headers = new ArrayList<String>();
			for(idx = 0; idx < colCount; idx++) {
				String header;
				header = rs.getMetaData().getColumnLabel(idx + 1);
				headers.add(header);
			}		
			List<String> values = new ArrayList<String>();
			
			while(rs.next()) {
				if(rowCount > 0) {
					writer.append(",\r\n");
				}
				rowCount++;
				values.clear();
				for(idx = 0; idx < colCount; idx++) {
					String value;
					value = rs.getString(idx+1);
					values.add(value);
				}
				JSONUtils.writeRecord(writer, headers, values);
			}
			writer.append("\r\n]\r\n");
			writer.close();
		} catch (SQLException e) {
			__sqlErrors(e);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return rowCount;
	}
	
	private void __sqlErrors(SQLException e) {
        System.err.println("SQLState: " +
            ((SQLException)e).getSQLState());

        System.err.println("Error Code: " +
            ((SQLException)e).getErrorCode());

        System.err.println("Message: " + e.getMessage());
	}
}

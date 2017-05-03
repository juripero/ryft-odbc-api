// JSONUtils.java : CSV writing utils
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

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.StringTokenizer;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

public class JSONUtils {

	private static ArrayList<String> __colToList(String colHeader)
	{
		ArrayList<String> strings = new ArrayList<String>();
		for(String string: colHeader.split("\\.")) {
			strings.add(string);
		}
		return strings;
	}
	
	public static void writeRecord(Writer w, List<String> headers, List<String> values) throws IOException
	{
		JSONObject root = new JSONObject();
		Deque<Object> jsonObjs = new ArrayDeque<Object>();
		jsonObjs.push(root);
		
		int prevLevel = 0;
		ArrayList<String> prevCol = new ArrayList<String>();
		
		int __idx = 0;
		Object jobj = null;		
		for(String header: headers) {
			String value = values.get(__idx++);
			
			// segment column header and find match level with previous column header
			ArrayList<String> thisCol = __colToList(header);
			int matchLevel = 0;
			for(int i = 0; i < Math.min(prevCol.size(), thisCol.size()); i++, matchLevel++) {
				if(thisCol.get(i).equals(prevCol.get(i)) == false) 
					break;
			}
			
			// pop previously "finalized" JSON objects off the work stack
			for(; prevLevel > matchLevel; prevLevel--) {
				jobj = jsonObjs.pop();
			}
			
			// "push" new levels onto the work stack
			for (int __newLevel = thisCol.size() - 1; __newLevel > matchLevel; matchLevel++) {
				if(thisCol.get(matchLevel+1).equals("[]")) {
					jobj = new JSONArray();
				}
				else {
					jobj = new JSONObject();
				}
				if(jsonObjs.peek() instanceof JSONArray) {
					((JSONArray)jsonObjs.peek()).add(jobj);
				}
				else if(jsonObjs.peek() instanceof JSONObject) {
					((JSONObject)jsonObjs.peek()).put(thisCol.get(matchLevel), jobj);
				}
				jsonObjs.push(jobj);
			}
			prevLevel = matchLevel;
			
			// add current column value to topmost element on the jsonObjs stack
			if(jsonObjs.peek() instanceof JSONArray) {
				((JSONArray)jsonObjs.peek()).add(value);
			}
			else if(jsonObjs.peek() instanceof JSONObject) {
				((JSONObject)jsonObjs.peek()).put(thisCol.get(thisCol.size()-1), value);
			}
			prevCol = thisCol;
		}
		// root is the bottom
		w.write(root.toJSONString());
	}
}

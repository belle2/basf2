package org.belle2.daq.database;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import org.belle2.daq.io.SocketDataReader;
import org.belle2.daq.io.SocketDataWriter;

public class DBLoader {

	static public DBObject load(String dbhost, int dbport, String table, String config) {
		DBObject obj = new DBObject();
		Socket socket = null;
		try {
			socket = new Socket(dbhost, dbport);
			SocketDataWriter writer = new SocketDataWriter(socket);
			writer.writeInt(1);
			writer.writeString(table + "/" + config);
			SocketDataReader reader = new SocketDataReader(socket);
			reader.readObject(obj);
			//obj.print();
		} catch (UnknownHostException e) {
			//e.printStackTrace();
		} catch (IOException e) {
			//e.printStackTrace();
		}
		if (socket != null) {
			try {
				socket.close();
			} catch (IOException e) {}
		}
		return obj;
	}

	static public String [] loadList(String dbhost, int dbport, String table, String config) {
		System.out.println(dbhost + ":" + dbport);
		Socket socket = null;
		String [] ss = null;
		try {
			socket = new Socket(dbhost, dbport);
			SocketDataWriter writer = new SocketDataWriter(socket);
			writer.writeInt(3);
			writer.writeString(table + "/" + config);
			SocketDataReader reader = new SocketDataReader(socket);
			int nconf = reader.readInt();
			ss = new String [nconf];
			for (int i = 0; i < nconf; i++) {
				ss[i] = reader.readString();
			}
		} catch (Exception e) {
			//e.printStackTrace();
		}
		if (socket != null) {
			try {
				socket.close();
			} catch (IOException e) {}
		}
		return ss;
	}
}

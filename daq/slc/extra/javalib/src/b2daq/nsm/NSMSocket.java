package b2daq.nsm;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import b2daq.database.ConfigObject;
import b2daq.io.SocketDataReader;
import b2daq.io.SocketDataWriter;

public class NSMSocket {
	private SocketDataReader m_reader;
	private SocketDataWriter m_writer;
	private Socket m_socket;

	public NSMSocket(String hostname, int port) throws UnknownHostException, IOException {
		m_socket = new Socket(hostname, port);
		m_reader = new SocketDataReader(m_socket);
		m_writer = new SocketDataWriter(m_socket);
	}

	public void setSocket(String hostname, int port)
			throws UnknownHostException, IOException {
		m_socket = new Socket(hostname, port);
		m_reader = new SocketDataReader(m_socket);
		m_writer = new SocketDataWriter(m_socket);
	}

	public void request(NSMMessage msg) throws IOException {
		try {
			synchronized (m_writer) {
				m_writer.writeObject(msg);
			}	
		} catch (IOException e) {
			m_socket.close();
			throw e;
		}
	}

	public void requestGet(NSMData data) throws IOException {
		try {
			synchronized (m_writer) {
				NSMMessage msg = new NSMMessage(NSMCommand.NSMGET);
				msg.setData(data.getName()+ " " + data.getFormat());
				msg.setNParams(1);
				msg.setParam(0, data.getRevision());
				m_writer.writeObject(msg);
			}
		} catch (IOException e) {
			m_socket.close();
			throw e;
		}
	}

	public void requestGet(ConfigObject obj) throws IOException {
		try {
			synchronized (m_writer) {
				NSMMessage msg = new NSMMessage(NSMCommand.DBGET);
				if (obj.getName().length() > 0 || obj.getId() > 0) {
					System.out.println("obj.getId() = "+obj.getId());
					if (obj.getId() > 0) {
						msg.setNParams(1);
						msg.setParam(0, obj.getId());
					}
					msg.setData("config " + obj.getNode()+ " " + obj.getName() + " " + obj.getTable());
				} else {
					msg.setData("configlist " + obj.getNode()+ " " + obj.getTable());
				}
				m_writer.writeObject(msg);
			}
		} catch (IOException e) {
			m_socket.close();
			throw e;
		}
	}

	public void requestSet(ConfigObject obj) throws IOException {
		try {
			synchronized (m_writer) {
				NSMMessage msg = new NSMMessage(NSMCommand.DBSET);
				msg.setData(obj);
				m_writer.writeObject(msg);
			}
		} catch (IOException e) {
			m_socket.close();
			throw e;
		}
	}

	public NSMMessage wait(NSMMessage msg) throws IOException {
		try {
			synchronized (m_reader) {
				m_reader.readObject(msg);
			}
		} catch (IOException e) {
			m_socket.close();
			throw e;
		}
		return msg;
	}
}

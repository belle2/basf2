/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;

import org.belle2.daq.database.DBObject;
import org.belle2.daq.io.SocketDataReader;
import org.belle2.daq.io.SocketDataWriter;

/**
 *
 * @author tkonno
 */
public class NSMCommunicator extends Thread {

	static private final ArrayList<NSMCommunicator> g_coms = new ArrayList<>();

	private ArrayList<NSMRequestHandler> m_handler = new ArrayList<>();
	private String m_hostname = "";
	private int m_port = 9090;
	private String m_nsmnode = "";
	private String m_nsmhost = "";
	private int m_nsmport = 9122;
	private SocketDataReader m_reader;
	private SocketDataWriter m_writer;
	private Socket m_socket;
	private int retry = 0;
	private ArrayList<NSMMessage> m_msg;
	private NSMCommunicator m_com;
	private boolean m_closed = false;

	public static ArrayList<NSMCommunicator> get() {
		return g_coms;
	}

	public void reconnect(String hostname, int port, String nsmnode,
			String nsmhost, int nsmport) {
		close();
		interrupt();
		m_hostname = hostname;
		m_port = port;
		m_nsmnode = nsmnode;
		m_nsmhost = nsmhost;
		m_nsmport = nsmport;
		start();
	}

	private void reconnect() throws IOException {
		if (retry == 0) {
			System.out.println("connecting : " + m_hostname + ":" + m_port + " "
					+ m_nsmnode + " " + m_nsmhost + ":" + m_nsmport);
		}
		m_socket = new Socket(m_hostname, m_port);
		System.out.println("connected ");
		retry = 0;
		m_reader = new SocketDataReader(m_socket);
		m_writer = new SocketDataWriter(m_socket);
		m_writer.writeString(m_nsmnode);
		m_writer.writeString(m_nsmhost);
		m_writer.writeInt(m_nsmport);
	}

	public void add(NSMRequestHandler handler) {
		synchronized (m_handler) {
			m_handler.add(handler);
		}
	}

	public void remove(NSMRequestHandler handler) {
		synchronized (m_handler) {
			m_handler.remove(handler);
		}
	}

	public boolean isConnected() {
		return m_socket != null;
	}

	public void close() {
		try {
			if (m_socket != null) {
				m_socket.close();
			}
		} catch (IOException ex) {
		}
		m_socket = null;
	}

	@Override
	public void run() {
		m_com = this;
		new Thread() {
			public void run() {
				while (true) {
					System.err.println("debug : 1-1");
					if (m_socket == null) {
						try {
							System.err.println("debug : 1-2");
							reconnect();
							synchronized (m_handler) {
								int nhandlers = m_handler.size();
								for (int n = 0; n < nhandlers; n++) {
									System.err.println("debug : 1-3");
									m_handler.get(n).connected(m_com);
								}
							}
						} catch (IOException ex) {
							retry++;
							close();
							try {
								Thread.sleep(5000);
							} catch (InterruptedException ex1) {
								close();
								g_coms.remove(this);
								m_closed = true;
								return;
							}
						}
					}
					try {
						while (true) {
							System.err.println("debug : 1-4");
							NSMMessage msg = waitMessage();
							synchronized (m_msg) {
								System.err.println("debug : 1-5");
								m_msg.add(msg);
							}
						}
					}catch (IOException ex) {
						close();
						try {
							Thread.sleep(10000);
						} catch (InterruptedException ex1) {
							close();
							System.out.println("closed");
							g_coms.remove(this);
							m_closed = true;
							return;
						}
						if (retry <= 1) {
							System.err.println(m_hostname+":"+m_port+" closed");
						}
					}
				}
			}
		}.start();
		while (true) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {

			}
			if (m_closed == true) return;
			synchronized (m_msg) {
				for (NSMMessage msg : m_msg) {
					System.err.println("debug : 2-1");
					NSMCommand cmd = new NSMCommand(msg.getReqName());
					System.err.println("debug : 2-2");
					if (cmd.equals(NSMCommand.ERROR)) {
						//Logger logger = Logger.getLogger(msg.getNodeName());
						//logger.log(Level.WARNING, msg.getData());
						continue;
					}
					if (cmd.equals(NSMCommand.OK)) {
						//Logger logger = Logger.getLogger(msg.getNodeName());
						//logger.log(Level.INFO, msg.getData());
						continue;
					}
					//ArrayList<NSMRequestHandler> handlers = new ArrayList<>();
					synchronized (m_handler) {
						for (NSMRequestHandler handler : m_handler) {
							System.err.println("debug : 2-3");
							try {
								if (!handler.isOnce() || !handler.isDone()) {
									System.err.println("debug : 2-4");
									if (handler.handle(cmd, msg, this)) {
										handler.isDone(true);
									}
								}
							} catch (Exception e) {
								e.printStackTrace();
							}

						}
						/*
						int nhandlers = m_handler.size();
						for (int i = 0; i < nhandlers; i++) {
							handlers.add(m_handler.get(i));
						}
						for (NSMRequestHandler handler : handlers) {
							try {
								if (handler.handle(cmd, msg, this)) {
									if (handler.isOnce()) {
										m_handler.remove(handler);
									}
								}
							} catch (Exception e) {
								e.printStackTrace();
							}
						}
						*/
					}
				}
				try {
					System.err.println("debug : 2-5");
					m_msg.clear();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}
	}

	public void request(NSMMessage msg) throws IOException {
		if (m_socket == null) {
			throw new IOException("No socket is available");
		}
		synchronized (m_writer) {
			m_writer.writeObject(msg);
		}
	}

	public void requestDATAGet(NSMData data) throws IOException {
		requestDATAGet(data.getName(), data.getFormat());
	}

	public void requestDATAGet(String dataname, String format)
			throws IOException {
		NSMMessage msg = new NSMMessage(NSMCommand.DATAGET);
		msg.setData(dataname + " " + format);
		msg.setNParams(1);
		msg.setParam(0, -1);
		request(msg);
	}

	public void requestList(String table, String node, String prefix)
			throws IOException {
		request(new NSMMessage(NSMCommand.DBLISTGET, table + "/" + node + "@"
				+ prefix));
	}

	public void requestList(String table, String node)
			throws IOException {
		request(new NSMMessage(NSMCommand.DBLISTGET, table + "/" + node));
	}

	public void requestDBGet(String config, boolean isfull)
			throws IOException {
		int[] pars = { (isfull ? 1 : 0) };
		request(new NSMMessage(NSMCommand.DBGET, pars, config));
	}

	public void requestDBSet(String table, DBObject obj) throws IOException {
		synchronized (m_writer) {
			m_writer.writeObject(new NSMMessage(NSMCommand.DBGET, table));
			m_writer.writeObject(obj);
		}
	}

	public void requestVGet(String node, String vname)
			throws IOException {
		request(new NSMMessage(node, NSMCommand.VGET, vname));
	}

	public void requestVListGet(String node) throws IOException {
		request(new NSMMessage(node, NSMCommand.VLISTGET));
	}

	public void requestVSet(String node, NSMVar var) throws IOException {
		if (m_writer != null) {
			synchronized (m_writer) {
				m_writer.writeObject(new NSMMessage(node, NSMCommand.VSET));
				m_writer.writeObject(var);
			}
		} else {
			throw new IOException("Socket to "+ m_hostname + ":" + m_port + " is not avaialble");
		}
	}

	private NSMMessage waitMessage() throws IOException {
		NSMMessage msg = new NSMMessage();
		if (m_reader != null) {
			m_reader.readObject(msg);
			return msg;
		}
		throw new IOException("Socket to "+ m_hostname + ":" + m_port + " is not avaialble");
	}

	public String getHostNamePort() {
		return m_hostname+":"+m_port;
	}

	public ArrayList<NSMRequestHandler> getHandlers() {
		return m_handler;
	}

	public void setHandlers(ArrayList<NSMRequestHandler> handlers) {
		m_handler.clear();
		if (handlers != null) {
			m_handler.addAll(handlers);
		}
	}

	@SuppressWarnings("deprecation")
	public static void closeAll() {
		for (NSMCommunicator com : g_coms) {
			com.close();
			com.stop();
		}
		g_coms.clear();
	}

	static public void addHandlerAll(NSMRequestHandler handler) {
		synchronized (g_coms) {
			for (NSMCommunicator com : g_coms) {
				com.add(handler);
			}
		}
	}

	public static void removeHandlerAll(NSMRequestHandler handler) {
		synchronized (g_coms) {
			for (NSMCommunicator com : g_coms) {
				com.remove(handler);
			}
		}
	}

}

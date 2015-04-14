/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.core.LogLevel;
import b2daq.database.ConfigObject;
import b2daq.io.SocketDataReader;
import b2daq.io.SocketDataWriter;
import b2daq.logger.core.LogMessage;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author tkonno
 */
public class NSMCommunicator extends Thread {

    private static final NSMCommunicator g_com = new NSMCommunicator();

    private final ArrayList<NSMRequestHandler> m_handler = new ArrayList<>();
    private String m_hostname = "";
    private int m_port = 9090;
    private String m_nsmnode = "";
    private String m_nsmhost = "";
    private int m_nsmport = 9122;
    private SocketDataReader m_reader;
    private SocketDataWriter m_writer;
    private Socket m_socket;

    public static NSMCommunicator get() {
        return g_com;
    }

    public void reconnect(String hostname, int port, String nsmnode,
            String nsmhost, int nsmport) throws IOException {
        m_hostname = hostname;
        m_port = port;
        m_nsmnode = nsmnode;
        m_nsmhost = nsmhost;
        m_nsmport = nsmport;
        start();
    }

    public void reconnect() throws IOException {
        m_socket = new Socket(m_hostname, m_port);
        m_reader = new SocketDataReader(m_socket);
        m_writer = new SocketDataWriter(m_socket);
        m_writer.writeString(m_nsmnode);
        m_writer.writeString(m_nsmhost);
        m_writer.writeInt(m_nsmport);
    }

    public void add(NSMRequestHandler handler) {
        synchronized (m_handler) {
            if (m_socket != null && m_socket.isConnected()) {
                handler.connected();
            }
            m_handler.add(handler);
        }
    }

    public void close() {
        try {
            if (m_socket != null) {
                m_socket.close();
            }
        } catch (IOException ex) {
        }
    }

    @Override
    public void run() {
        while (true) {
            if (m_socket == null || !m_socket.isConnected()) {
                try {
                    reconnect();
                    synchronized (m_handler) {
                        int nhandlers = m_handler.size();
                        for (int n = 0; n < nhandlers; n++) {
                            m_handler.get(n).connected();
                        }
                    }
                } catch (IOException ex) {
                    close();
                    Logger.getLogger(NSMCommunicator.class.getName()).log(Level.SEVERE, null, ex);
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException ex1) {
                    }
                }
            }
            try {
                while (true) {
                    NSMMessage msg = waitMessage();
                    NSMCommand cmd = new NSMCommand(msg.getReqName());
                    ArrayList<NSMRequestHandler> handlers = new ArrayList();
                    synchronized (m_handler) {
                        int nhandlers = m_handler.size();
                        for (int i = 0; i < nhandlers; i++) {
                            handlers.add(m_handler.get(i));
                        }
                        for (NSMRequestHandler handler : handlers) {
                            if (handler.handle(cmd, msg) && handler.isOnce()) {
                                m_handler.remove(handler);
                            }
                        }
                    }
                }
            } catch (IOException ex) {
                Logger.getLogger(NSMCommunicator.class.getName()).log(Level.SEVERE, null, ex);
                log(new LogMessage("LOCAL", LogLevel.ERROR,
                        "Lost connection to NSM2Socket"));
                close();
                try {
                    Thread.sleep(10000);
                } catch (Exception ex2) {
                }
            }
        }
    }

    public void request(NSMMessage msg) throws IOException {
        while (m_socket == null || !m_socket.isConnected()) {
            try {
                Thread.sleep(10000);
            } catch (Exception ex2) {
            }
        }
        m_writer.writeObject(msg);
    }

    public void requestDATAGet(NSMData data) throws IOException {
        requestDATAGet(data.getName(), data.getFormat());
    }

    public void requestDATAGet(String dataname, String format) throws IOException {
        NSMMessage msg = new NSMMessage(NSMCommand.DATAGET);
        msg.setData(dataname + " " + format);
        msg.setNParams(1);
        msg.setParam(0, -1);
        request(msg);
    }

    public void requestList(String table, String node, String prefix) throws IOException {
        request(new NSMMessage(NSMCommand.DBLISTGET, table + "/" + node + "@" + prefix));
    }

    public void requestList(String table, String node) throws IOException {
        request(new NSMMessage(NSMCommand.DBLISTGET, table + "/" + node));
    }

    public void requestDBGet(String config, boolean isfull) throws IOException {
        int[] pars = {(isfull ? 1 : 0)};
        request(new NSMMessage(NSMCommand.DBGET, pars, config));
    }

    public void requestDBSet(String table, ConfigObject obj) throws IOException {
        request(new NSMMessage(NSMCommand.DBGET, table));
        m_writer.writeObject(obj);
    }

    public void requestVGet(String node, String vname) throws IOException {
        request(new NSMMessage(node, NSMCommand.VGET, vname));
    }

    public void requestVListGet(String node) throws IOException {
        request(new NSMMessage(node, NSMCommand.VLISTGET));
    }

    public void requestVSet(String node, NSMVar var) throws IOException {
        request(new NSMMessage(node, NSMCommand.VSET));
        m_writer.writeObject(var);
    }

    private NSMMessage waitMessage() throws IOException {
        NSMMessage msg = new NSMMessage();
        m_reader.readObject(msg);
        return msg;
    }

    private void log(final LogMessage log) {
        System.out.println(log.getMessage());
        for (NSMRequestHandler handler : m_handler) {
            try {
                ((NSMLogHandler) handler).handleLog(log);
            } catch (Exception e) {
            }
        }
    }

}

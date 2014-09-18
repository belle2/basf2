package b2daq.nsm;

import java.io.IOException;
import java.net.Socket;

import b2daq.database.ConfigObject;
import b2daq.io.SocketDataReader;
import b2daq.io.SocketDataWriter;

public class NSMSocket {

    private SocketDataReader m_reader;
    private SocketDataWriter m_writer;
    private Socket m_socket;

    public NSMSocket() {
    }

    public boolean reconnect(String hostname, int port, String nodename,
            String nsm2host, int nsm2port) {
        try {
            m_socket = new Socket(hostname, port);
            System.out.println(m_socket.getLocalAddress().getHostName() + ":" + m_socket.getLocalPort());
            m_reader = new SocketDataReader(m_socket);
            m_writer = new SocketDataWriter(m_socket);
            m_writer.writeString(nodename);
            m_writer.writeString(nsm2host);
            m_writer.writeInt(nsm2port);
            return true;
        } catch (IOException e) {
            close();
            return false;
        }
    }

    public boolean request(NSMMessage msg) {
        try {
            synchronized (m_writer) {
                m_writer.writeObject(msg);
            }
            return true;
        } catch (IOException e) {
            close();
            return false;
        }
    }

    public boolean requestNSMGet(NSMData data) {
        return requestNSMGet(data.getName(), data.getFormat(), data.getRevision());
    }

    public boolean requestNSMGet(String dataname, String format, int revision) {
        NSMMessage msg = new NSMMessage(NSMCommand.NSMGET);
        msg.setData(dataname + " " + format);
        msg.setNParams(1);
        msg.setParam(0, revision);
        return request(msg);
    }

    public boolean requestList(String nodename) {
        return request(new NSMMessage(NSMCommand.LISTGET, nodename));
    }

    public boolean requestDBGet(ConfigObject obj) {
        return requestDBGet(obj.getNode(), obj.getTable(), obj.getName(), obj.getId());
    }

    public boolean requestDBGet(String node, String table, String name, int id) {
        NSMMessage msg = new NSMMessage(NSMCommand.DBGET);
        if (name.length() > 0 || id > 0) {
            if (id > 0) {
                msg.setNParams(1);
                msg.setParam(0, id);
            } else {
                msg.setNParams(0);
            }
            msg.setData("config " + node + " " + name + " " + table);
        } else {
            msg.setData("configlist " + node + " " + table);
        }
        return request(msg);
    }

    public boolean requestDBSet(ConfigObject obj) {
        NSMMessage msg = new NSMMessage(NSMCommand.DBSET);
        msg.setData(obj);
        return request(msg);
    }

    public NSMMessage waitMessage() {
        try {
            NSMMessage msg = new NSMMessage();
            synchronized (m_reader) {
                m_reader.readObject(msg);
            }
            return msg;
        } catch (IOException e) {
            e.printStackTrace();
            close();
            return null;
        }
    }

    protected void close() {
        try {
            m_socket.close();
        } catch (Exception e) {
        }
    }

    public boolean isConnected() {
        if (m_socket != null) {
            return m_socket.isConnected();
        } else {
            return false;
        }
    }
}

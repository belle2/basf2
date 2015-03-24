package b2daq.nsm;

import b2daq.core.Serializable;
import java.io.IOException;
import java.net.Socket;

import b2daq.io.SocketDataReader;
import b2daq.io.SocketDataWriter;
import java.util.logging.Level;
import java.util.logging.Logger;

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

    boolean readObject(Serializable obj) {
        try {
            m_reader.readObject(obj);
            return true;
        } catch (IOException ex) {
            Logger.getLogger(NSMSocket.class.getName()).log(Level.SEVERE, null, ex);
            close();
        }
        return false;
    }

    boolean writeObject(Serializable obj) {
        try {
            m_writer.writeObject(obj);
            return true;
        } catch (IOException ex) {
            Logger.getLogger(NSMSocket.class.getName()).log(Level.SEVERE, null, ex);
            close();
        }
        return false;
    }
}

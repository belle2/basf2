/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.core.LogLevel;
import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import java.util.ArrayList;
import java.util.HashMap;
import javafx.application.Platform;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleObjectProperty;

/**
 *
 * @author tkonno
 */
public class NSMListenerService extends Thread {

    private static NSMListenerService g_service = null;

    private static final ArrayList<NSMObserver> m_observer = new ArrayList<>();
    private static final NSMSocket m_socket = new NSMSocket();
    private static final BooleanProperty connection = new SimpleBooleanProperty(false);
    private static final ObjectProperty nsmMessage = new SimpleObjectProperty();
    private static final HashMap<String, NSMData> m_data_m = new HashMap<>();
    private static final HashMap<String, ConfigObject> m_obj_m = new HashMap<>();
    private static boolean is_closecalled = false;
    private static NSMConfig m_cofig;

    private NSMListenerService() {

    }

    public static void add(NSMObserver observable) {
        m_observer.add(observable);
    }

    public static void setNSMConfig(NSMConfig config) {
        m_cofig = config;
    }

    public static NSMConfig getNSMConfig() {
        return m_cofig;
    }

    public static NSMData getData(String name) {
        if (m_data_m.containsKey(name)) {
            return m_data_m.get(name);
        } else {
            return null;
        }
    }

    public static ConfigObject getDB(String name) {
        if (m_obj_m.containsKey(name)) {
            return m_obj_m.get(name);
        } else {
            return null;
        }
    }

    public static BooleanProperty connectionProperty() {
        return connection;
    }

    public static void close() {
        is_closecalled = true;
        m_socket.close();
        connection.set(false);
    }

    public static boolean isCloseCalled() {
        return is_closecalled;
    }

    public static boolean isConnected() {
        return m_socket.isConnected();
    }

    @Override
    public void run() {
        NSMConfig config = getNSMConfig();
        is_closecalled = false;
        int ntry = 0;
        while (true) {
            if (is_closecalled) {
                return;
            }
            if (connection.get() == true || m_socket.reconnect(config.getHostname(),
                    config.getPort(), config.getNsmNode(),
                    config.getNsmHost(), config.getNsmPort())) {
                connection.set(true);
                handleOnConnected();
                log(new LogMessage("LOCAL", LogLevel.DEBUG,
                        "Connected to NSM2Socket"));
                ntry = 0;
            } else {
                log(new LogMessage("LOCAL", LogLevel.WARNING,
                        "Failed to connect to NSM2Socket"));
                ntry++;
                if (ntry > 2) {
                    log(new LogMessage("LOCAL", LogLevel.FATAL,
                            "Gave up retry. Try \"Reconnect\" by hand"));
                    is_closecalled = true;
                    handleOnDisConnected();
                    return;
                }
                connection.set(false);
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException ex1) {
                }
                continue;
            }
            while (true) {
                NSMMessage msg = waitMessage();
                nsmMessage.set(msg);
                handleOnReceived(msg);
                if (msg == null) {
                    log(new LogMessage("LOCAL", LogLevel.ERROR,
                            "Lost connection to NSM2Socket"));
                    connection.set(false);
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException ex1) {
                    }
                    break;
                }
                if (is_closecalled) {
                    return;
                }
            }
            if (m_socket != null) {
                m_socket.close();
                connection.set(false);
            }
        }
    }

    public static boolean request(NSMMessage msg) {
        return m_socket.request(msg);
    }

    public static boolean requestNSMGet(NSMData data) {
        return m_socket.requestNSMGet(data);
    }

    public static boolean requestNSMGet(String dataname, String format, int revision) {
        return m_socket.requestNSMGet(dataname, format, revision);
    }

    public static boolean requestList(String nodename) {
        return m_socket.requestList(nodename);
    }

    public static boolean requestDBGet(ConfigObject obj) {
        return m_socket.requestDBGet(obj);
    }

    public static boolean requestDBGet(String node, String table, String name, int id) {
        return m_socket.requestDBGet(node, table, name, id);
    }

    public static boolean requestDBGet(String node, int id) {
        return m_socket.requestDBGet(node, "", "", id);
    }

    public static boolean requestDBGet(String node, String table) {
        return m_socket.requestDBGet(node, table, "", 0);
    }

    public static boolean requestDBSet(ConfigObject obj) {
        return m_socket.requestDBSet(obj);
    }

    private static NSMMessage waitMessage() {
        NSMMessage msg = m_socket.waitMessage();
        if (msg != null) {
            NSMCommand command = new NSMCommand();
            command.copy(msg.getReqName());
            if (command.equals(NSMCommand.DBSET)) {
                System.out.println(msg.getNodeName());
                ConfigObject obj = getDB(msg.getNodeName());
                if (obj == null) {
                    obj = new ConfigObject();
                    m_obj_m.put(msg.getNodeName(), obj);
                }
                msg.getData(obj);
            } else if (command.equals(NSMCommand.NSMSET)) {
                NSMData data = getData(msg.getNodeName());
                if (data == null) {
                    data = new NSMData();
                    m_data_m.put(msg.getNodeName(), data);
                }
                msg.getData(data);
                //data.print();
            }
        }
        return msg;
    }

    private static void handleOnConnected() {
        Platform.runLater(() -> {
            for (NSMObserver obs : m_observer) {
                try {
                    obs.handleOnConnected();
                } catch (Exception e) {
                    e.printStackTrace();
                    log(new LogMessage("LOCAL", LogLevel.ERROR,
                            "Internal exception " + e.getMessage()));
                }
            }
        });
    }

    private static void handleOnReceived(final NSMMessage msg) {
        Platform.runLater(() -> {
            for (NSMObserver obs : m_observer) {
                try {
                    obs.handleOnReceived(msg);
                } catch (Exception e) {
                    e.printStackTrace();
                    log(new LogMessage("LOCAL", LogLevel.ERROR,
                            "Internal exception " + e.getMessage()));
                }
            }
        });
    }

    private static void handleOnDisConnected() {
        Platform.runLater(() -> {
            for (NSMObserver obs : m_observer) {
                try {
                    obs.handleOnDisConnected();
                } catch (Exception e) {
                    e.printStackTrace();
                    log(new LogMessage("LOCAL", LogLevel.ERROR,
                            "Internal exception " + e.getMessage()));
                }
            }
        });
    }

    private static void log(final LogMessage log) {
        System.out.println(log.getMessage());
        Platform.runLater(() -> {
            for (NSMObserver obs : m_observer) {
                obs.log(log);
            }
        });
    }

    public static void restart() {
        if (g_service == null || !g_service.isAlive()) {
            NSMConfig config = getNSMConfig();
            if (m_socket.reconnect(config.getHostname(),
                    config.getPort(), config.getNsmNode(),
                    config.getNsmHost(), config.getNsmPort())) {
                connection.set(true);
            } else {
                connection.set(false);
            }
            g_service = new NSMListenerService();
            g_service.start();
        }
    }

}

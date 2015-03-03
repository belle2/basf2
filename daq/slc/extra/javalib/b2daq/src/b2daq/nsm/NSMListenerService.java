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

/**
 *
 * @author tkonno
 */
public class NSMListenerService extends Thread {

    private static NSMListenerService g_service = null;

    private static final ArrayList<NSMObserver> g_observer = new ArrayList<>();
    private static final NSMSocket g_socket = new NSMSocket();
    private static boolean g_connection = false;
    private static final HashMap<String, NSMData> g_data_m = new HashMap<>();
    private static final HashMap<String, ConfigObject> g_obj_m = new HashMap<>();
    private static final HashMap<String, ArrayList<String>> g_confname_m = new HashMap<>();
    private static boolean g_closecalled = false;
    private static NSMConfig g_cofig;

    private NSMListenerService() {

    }

    public static void add(NSMObserver observable) {
        g_observer.add(observable);
    }

    public static void setNSMConfig(NSMConfig config) {
        g_cofig = config;
    }

    public static NSMConfig getNSMConfig() {
        return g_cofig;
    }

    public static NSMData getData(String name) {
        if (g_data_m.containsKey(name)) {
            return g_data_m.get(name);
        } else {
            return null;
        }
    }

    public static ConfigObject getDB(String name) {
        if (g_obj_m.containsKey(name)) {
            return g_obj_m.get(name);
        } else {
            return null;
        }
    }

    public static ArrayList<String> getList(String name) {
        if (g_confname_m.containsKey(name)) {
            return g_confname_m.get(name);
        } else {
            return null;
        }
    }

    public static void close() {
        g_closecalled = true;
        g_socket.close();
        g_connection = false;
    }

    public static boolean isCloseCalled() {
        return g_closecalled;
    }

    public static boolean isConnected() {
        return g_socket.isConnected();
    }

    @Override
    public void run() {
        NSMConfig config = getNSMConfig();
        g_closecalled = false;
        int ntry = 0;
        while (true) {
            if (g_closecalled) {
                return;
            }
            if (g_connection == true || g_socket.reconnect(config.getHostname(),
                    config.getPort(), config.getNsmNode(),
                    config.getNsmHost(), config.getNsmPort())) {
                g_connection = true;
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
                    g_closecalled = true;
                    handleOnDisConnected();
                    return;
                }
                g_connection = false;
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException ex1) {
                }
                continue;
            }
            while (true) {
                NSMMessage msg = waitMessage();
                handleOnReceived(msg);
                if (msg == null) {
                    log(new LogMessage("LOCAL", LogLevel.ERROR,
                            "Lost connection to NSM2Socket"));
                    g_connection = false;
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException ex1) {
                    }
                    break;
                }
                if (g_closecalled) {
                    return;
                }
            }
            if (g_socket != null) {
                g_socket.close();
                g_connection = false;
            }
        }
    }

    public static boolean request(NSMMessage msg) {
        return g_socket.request(msg);
    }

    public static boolean requestNSMGet(NSMData data) {
        return requestNSMGet(data.getName(), data.getFormat(), data.getRevision());
    }

    public static boolean requestNSMGet(String dataname, String format, int revision) {
        NSMMessage msg = new NSMMessage(NSMCommand.NSMDATAGET);
        msg.setData(dataname + " " + format);
        msg.setNParams(1);
        msg.setParam(0, revision);
        return request(msg);
    }

    public static boolean requestList(String table, String node, String prefix) {
        return request(new NSMMessage(NSMCommand.DBLISTGET, table+"/"+node+"@"+prefix));
    }

    public static boolean requestList(String table, String node) {
        return request(new NSMMessage(NSMCommand.DBLISTGET, table+"/"+node));
    }

    public static boolean requestDBGet(String config) {
        return request(new NSMMessage(NSMCommand.DBGET, config));
    }

    public static boolean requestDBSet(String table, ConfigObject obj) {
        if (request(new NSMMessage(NSMCommand.DBGET, table))) {
            return g_socket.writeObject(obj);
        }
        return false;
    }

    public static boolean requestVGet(String node, String vname) {
        return request(new NSMMessage(node, NSMCommand.VGET, vname));
    }

    public static boolean requestVSet(String node, NSMVar var) {
        if (request(new NSMMessage(node, NSMCommand.VSET))) {
            return g_socket.writeObject(var);
        }
        return false;
    }

    private static NSMMessage waitMessage() {
        NSMMessage msg = g_socket.waitMessage();
        if (msg != null) {
            NSMCommand command = new NSMCommand();
            command.copy(msg.getReqName());
            if (command.equals(NSMCommand.DBSET)) {
                g_obj_m.put(msg.getNodeName(), (ConfigObject)msg.getObject());
            } else if (command.equals(NSMCommand.DBLISTSET)) {
                if (msg.getNParams() > 0 && msg.getParam(0) > 0) {
                    String [] namelist = msg.getData().split("\n");
                    ArrayList<String> name_v = new ArrayList<>();
                    for (String name : namelist) {
                        name_v.add(name);
                    }
                    g_confname_m.put(msg.getNodeName(), name_v);
                }
            } else if (command.equals(NSMCommand.VSET)) {
            } else if (command.equals(NSMCommand.NSMDATASET)) {
                g_data_m.put(msg.getNodeName(), (NSMData)msg.getObject());
            }
        }
        return msg;
    }

    private static void handleOnConnected() {
        for (NSMObserver obs : g_observer) {
            try {
                obs.handleOnConnected();
            } catch (Exception e) {
                e.printStackTrace();
                log(new LogMessage("LOCAL", LogLevel.ERROR,
                        "Internal exception " + e.getMessage()));
            }
        }
    }

    private static void handleOnReceived(final NSMMessage msg) {
        for (NSMObserver obs : g_observer) {
            try {
                obs.handleOnReceived(msg);
            } catch (Exception e) {
                e.printStackTrace();
                log(new LogMessage("LOCAL", LogLevel.ERROR,
                        "Internal exception " + e.getMessage()));
            }
        }
    }

    private static void handleOnDisConnected() {
        for (NSMObserver obs : g_observer) {
            try {
                obs.handleOnDisConnected();
            } catch (Exception e) {
                e.printStackTrace();
                log(new LogMessage("LOCAL", LogLevel.ERROR,
                        "Internal exception " + e.getMessage()));
            }
        }
    }

    private static void log(final LogMessage log) {
        System.out.println(log.getMessage());
        for (NSMObserver obs : g_observer) {
            obs.log(log);
        }
    }

    public static void restart() {
        if (g_service == null || !g_service.isAlive()) {
            NSMConfig config = getNSMConfig();
            if (g_socket.reconnect(config.getHostname(),
                    config.getPort(), config.getNsmNode(),
                    config.getNsmHost(), config.getNsmPort())) {
                g_connection = true;
            } else {
                g_connection = false;
            }
            g_service = new NSMListenerService();
            g_service.start();
        }
    }

}

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm.ui;

import b2daq.core.LogLevel;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.util.ArrayList;
import javafx.application.Platform;

/**
 *
 * @author tkonno
 */
public class NSMListenerGUIHandler implements NSMObserver {

    private static final NSMListenerGUIHandler g_handler = new NSMListenerGUIHandler();

    public static NSMListenerGUIHandler get() {
        return g_handler;
    }
    
    private final ArrayList<NSMObserver> m_observer = new ArrayList<>();

    public void add(NSMObserver observable) {
        m_observer.add(observable);
    }

    public ArrayList<NSMObserver> getObservers() {
        return m_observer;
    }

    @Override
    public void handleOnConnected() {
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

    @Override
    public void handleOnReceived(NSMMessage msg) {
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

    @Override
    public void handleOnDisConnected() {
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

    @Override
    public void log(final LogMessage log) {
        System.out.println(log.getMessage());
        Platform.runLater(() -> {
            for (NSMObserver obs : m_observer) {
                obs.log(log);
            }
        });
    }

}

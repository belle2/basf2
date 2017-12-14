/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm.ui;

import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMRequestHandler;
import java.util.ArrayList;
import javafx.application.Platform;

/**
 *
 * @author tkonno
 */
public class NSMRequestHandlerUI extends NSMRequestHandler {

    private final ArrayList<NSMRequestHandler> m_handler = new ArrayList<>();
    private boolean m_available = false;

    private final static NSMRequestHandlerUI g_handler = new NSMRequestHandlerUI();

    public final static NSMRequestHandlerUI get() {
        return g_handler;
    }

    private NSMRequestHandlerUI() {
        super(false);
    }

    @Override
    public boolean connected() {
        m_available = true;
        synchronized (m_handler) {
            int nhandlers = m_handler.size();
            Platform.runLater(() -> {
                for (int n = 0; n < nhandlers; n++) {
                    final NSMRequestHandler handler = m_handler.get(n);
                    handler.connected();
                }
            });
        }
        return true;
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        ArrayList<NSMRequestHandler> handlers = new ArrayList();
        synchronized (m_handler) {
            int nhandlers = m_handler.size();
            for (int i = 0; i < nhandlers; i++) {
                handlers.add(m_handler.get(i));
            }
            Platform.runLater(() -> {
                for (NSMRequestHandler handler : handlers) {
                    if (handler.handle(cmd, msg) && handler.isOnce()) {
                        m_handler.remove(handler);
                    }
                }
            });
        }
        return true;
    }

    public void add(NSMRequestHandler handler) {
        synchronized (m_handler) {
            if (m_available) {
                Platform.runLater(() -> {
                    handler.connected();
                });
            }
            m_handler.add(handler);
        }
    }

}

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import java.io.IOException;

/**
 *
 * @author tkonno
 */
public abstract class NSMVSetHandler extends NSMRequestHandler {

    protected String m_node = "";
    protected String m_name = "";
    protected int m_type = NSMVar.NONE;

    private NSMVSetHandler(boolean once) {
        super(once);
    }

    public NSMVSetHandler(boolean once, String node, String name, int type) {
        super(once);
        m_node = node.toUpperCase();
        m_name = name;
        m_type = type;
    }

    @Override
    public boolean connected() {
        try {
            NSMCommunicator.get().requestVGet(m_node, m_name);
        } catch (IOException e) {

        }
        return true;
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(NSMCommand.VSET)) {
            NSMVar var = (NSMVar) msg.getObject();
            if ((m_node.isEmpty() || m_node.equals(var.getNode()))
                    && (m_name.isEmpty() || m_name.equals(var.getName()))
                    && (m_type == NSMVar.NONE || m_type == var.getType())) {
                return handleVSet(var);
            }
        }
        return false;
    }

    abstract public boolean handleVSet(NSMVar var);

}

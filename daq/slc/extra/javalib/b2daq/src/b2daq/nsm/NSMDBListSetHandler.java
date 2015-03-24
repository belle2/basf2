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
public abstract class NSMDBListSetHandler extends NSMRequestHandler {

    protected String m_node = "";
    protected String m_name = "";

    public NSMDBListSetHandler(boolean once, String node, String name) {
        super(once);
        m_node = node.toUpperCase();
        m_name = name;
    }

    public NSMDBListSetHandler(boolean once, String node) {
        this(once, node, "");
    }

    public NSMDBListSetHandler(String node, String name) {
        this(true, node, name);
    }

    public NSMDBListSetHandler(String node) {
        this(true, node, "");
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(NSMCommand.DBLISTSET)) {
            String [] list = msg.getData().split("\n");
            if (m_node.equals(msg.getNodeName())) {
                return handleDBListSet(list);
            }
        }
        return false;
    }

    @Override
    public boolean connected() {
        try {
            NSMCommunicator.get().requestList("ecl", m_node, m_name);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    abstract public boolean handleDBListSet(String [] list);

}

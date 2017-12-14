/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

import java.io.IOException;

/**
 *
 * @author tkonno
 */
public abstract class NSMDBListSetHandler extends NSMRequestHandler {

    protected String m_node = "";
    protected String m_name = "";
    protected String m_table = "";

    public NSMDBListSetHandler(boolean once, String table, String node, String name) {
        super(once);
        m_table = table;
        m_node = node.toUpperCase();
        m_name = name;
    }

    public NSMDBListSetHandler(boolean once, String table, String node) {
        this(once, table, node, "");
    }

    public NSMDBListSetHandler(String table, String node, String name) {
        this(true, table, node, name);
    }

    public NSMDBListSetHandler(String table, String node) {
        this(true, table, node, "");
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        if (cmd.equals(NSMCommand.DBLISTSET)) {
            String [] list = msg.getData().split("\n");
            if (m_node.equals(msg.getNodeName())) {
                return handleDBListSet(list, com);
            }
        }
        return false;
    }

    @Override
    public boolean connected(NSMCommunicator com) {
        try {
        	if (com != null)
        		com.requestList(m_table, m_node, m_name);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    abstract public boolean handleDBListSet(String [] list, NSMCommunicator com);

}

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

import java.io.IOException;

import org.belle2.daq.database.DBObject;

/**
 *
 * @author tkonno
 */
public abstract class NSMDBSetHandler extends NSMRequestHandler {

    protected String m_table = "";
    protected String m_node = "";
    protected String m_name = "";
    protected boolean m_isfull = false;
    
    public NSMDBSetHandler(boolean once, String table, String node, String name, boolean isfull) {
        super(once);
        m_table = table;
        m_node = node.toUpperCase();
        m_name = name;
        m_isfull = isfull;
    }

    public NSMDBSetHandler(boolean once, String table, String node, boolean isfull) {
        this(once, table, node, "", isfull);
    }

    public NSMDBSetHandler(String table, String node, String name, boolean isfull) {
        this(true, table, node, name, isfull);
    }

    public NSMDBSetHandler(String table, String node, boolean isfull) {
        this(true, table, node, "", isfull);
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        if (cmd.equals(NSMCommand.DBSET)) {
            DBObject obj = (DBObject) msg.getObject();
            String confname = obj.getName();
            if (confname.equals(m_node+"@"+m_name)) {
                return handleDBSet((DBObject) msg.getObject(), com);
            }
        }
        return false;
    }

    @Override
    public boolean connected(NSMCommunicator com) {
        try {
        	if (com != null)
        		com.requestDBGet(m_table + "/" + m_node + "@" + m_name, m_isfull);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    abstract public boolean handleDBSet(DBObject obj, NSMCommunicator com);

}

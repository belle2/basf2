/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.database.ConfigObject;
import java.io.IOException;

/**
 *
 * @author tkonno
 */
public abstract class NSMDBSetHandler extends NSMRequestHandler {

    protected String m_node = "";
    protected String m_name = "";
    protected boolean m_isfull = false;
    
    public NSMDBSetHandler(boolean once, String node, String name, boolean isfull) {
        super(once);
        m_node = node.toUpperCase();
        m_name = name;
        m_isfull = isfull;
    }

    public NSMDBSetHandler(boolean once, String node, boolean isfull) {
        this(once, node, "", isfull);
    }

    public NSMDBSetHandler(String node, String name, boolean isfull) {
        this(true, node, name, isfull);
    }

    public NSMDBSetHandler(String node, boolean isfull) {
        this(true, node, "", isfull);
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(NSMCommand.DBSET)) {
            ConfigObject obj = (ConfigObject) msg.getObject();
            String confname = obj.getName();
            if (obj.getName().equals(m_node+"@"+m_name)) {
                return handleDBSet((ConfigObject) msg.getObject());
            }
        }
        return false;
    }

    @Override
    public boolean connected() {
        try {
            NSMCommunicator.get().requestDBGet("ecl/" + m_node + "@" + m_name, m_isfull);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    abstract public boolean handleDBSet(ConfigObject obj);

}

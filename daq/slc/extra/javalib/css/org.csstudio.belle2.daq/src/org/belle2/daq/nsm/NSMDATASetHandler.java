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
public class NSMDATASetHandler extends NSMRequestHandler {

    protected String m_name = "";
    protected String m_format = "";

    public NSMDATASetHandler(boolean once, String name, String format) {
        super(once);
        m_name = name.toUpperCase();
        m_format = format;
    }

    public NSMDATASetHandler(String name, String format) {
        super(false);
        m_name = name.toUpperCase();
        m_format = format;
    }

    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        if (cmd.equals(NSMCommand.DATASET)) {
            NSMData data = (NSMData) msg.getObject();
            if (m_name.equals(data.getName())) {
                return handleDataSet(data, com);
            }
        }
        return false;
    }

    @Override
    public boolean connected(NSMCommunicator com) {
        try {
        	if (com != null)
        		com.requestDATAGet(m_name, m_format);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return true;
    }

    public boolean handleDataSet(NSMData data, NSMCommunicator com) {
        return true;
    }

}

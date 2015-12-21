/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.runcontrol;

import org.belle2.daq.nsm.NSMCommand;
import org.belle2.daq.nsm.NSMCommunicator;
import org.belle2.daq.nsm.NSMMessage;
import org.belle2.daq.nsm.NSMRequestHandler;

/**
 *
 * @author tkonno
 */
public abstract class RCConfigureHandler extends NSMRequestHandler {
    
    
    public RCConfigureHandler(boolean once) {
        super(once);
    }
    
    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        if (cmd.equals(RCCommand.CONFIGURE)) {
            return configure(msg.getData(), com);
        }
        return false;
    }
    
    abstract boolean configure(String configname, NSMCommunicator com);
    
}

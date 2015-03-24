/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.runcontrol.core;

import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMRequestHandler;

/**
 *
 * @author tkonno
 */
public abstract class RCConfigureHandler extends NSMRequestHandler {
    
    
    public RCConfigureHandler(boolean once) {
        super(once);
    }
    
    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(RCCommand.CONFIGURE)) {
            return configure(msg.getData());
        }
        return false;
    }
    
    abstract boolean configure(String configname);
    
}

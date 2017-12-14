/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

/**
 *
 * @author tkonno
 */
public abstract class NSMOKHandler extends NSMRequestHandler {

    public NSMOKHandler() {
        super(false);
    }
    
    public NSMOKHandler(boolean once) {
        super(once);
    }
    
    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        NSMState state = new NSMState(msg.getData());
        if (cmd.equals(NSMCommand.OK) && !state.equals(NSMState.UNKNOWN)) {
            return handleOK(msg.getNodeName(), state, com);
        } else {
            return false;
        }
    }
    
    abstract boolean handleOK(String nodename, NSMState state, NSMCommunicator com);
    
}

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

/**
 *
 * @author tkonno
 */
public abstract class NSMRequestHandler {

    private boolean m_once;
    
    abstract public boolean connected();
    abstract public boolean handle(NSMCommand cmd, NSMMessage msg);

    public NSMRequestHandler(boolean once) {
        m_once = once;
    }
    
    public boolean isOnce() {
        return m_once;
    }
    
}

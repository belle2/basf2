/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.logger.core.LogMessage;

/**
 *
 * @author tkonno
 */
public interface NSMObserver {

    public void handleOnConnected();

    public void handleOnReceived(NSMMessage msg);

    public void handleOnDisConnected();

    public void log(LogMessage log);

}

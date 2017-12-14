/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.core.LogLevel;
import b2daq.logger.core.LogMessage;
import java.util.Date;

/**
 *
 * @author tkonno
 */
public abstract class NSMLogHandler extends NSMRequestHandler {
    
    public NSMLogHandler() {
        super(false);
    }
    
    public NSMLogHandler(boolean once) {
        super(once);
    }
    
    @Override
    public boolean handle(NSMCommand cmd, NSMMessage msg) {
        if (cmd.equals(NSMCommand.LOG)) {
            String [] data = msg.getData().split("\n", 2);
            String node = data[0];
            String log = data[1];
            LogMessage lmsg = new LogMessage(node, LogLevel.Get(msg.getParam(0)), new Date(1000l * msg.getParam(1)), log);
            return handleLog(lmsg);
        }
        return false;
    }
    
    abstract public boolean handleLog(LogMessage lsmg);
}

package org.belle2.daq.nsm;

import java.util.Date;

import org.belle2.daq.base.LogLevel;
import org.belle2.daq.base.LogMessage;

public abstract class NSMLogHandler extends NSMRequestHandler {

	public NSMLogHandler(boolean once) {
		super(once);
	}

	@Override
	public boolean connected(NSMCommunicator com) {
		return false;
	}

	@Override
	public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
        if (cmd.equals(NSMCommand.LOG)) {
            String node = msg.getNodeName();
            String log = msg.getData();
            LogMessage lmsg = new LogMessage(node, LogLevel.Get(msg.getParam(0)), new Date(1000l * msg.getParam(1)), log);
            return handleLog(lmsg, com);
        }
        return false;
    }
    
    abstract public boolean handleLog(LogMessage lsmg, NSMCommunicator com);

}

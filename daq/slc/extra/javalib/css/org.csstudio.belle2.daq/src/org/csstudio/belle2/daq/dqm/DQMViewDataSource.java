package org.csstudio.belle2.daq.dqm;

import org.epics.pvmanager.ChannelHandler;
import org.epics.pvmanager.DataSource;

public class DQMViewDataSource extends DataSource {

	public final static String PLUGIN_ID="org.csstudio.belle2.daq";
	
	public DQMViewDataSource() {
		super(false);
	}

	@Override
	protected ChannelHandler createChannel(String channelName) {
		// dqm://<packname>:<histoname>:<parname>
		// parname : entries, means, rms, runno, expno, timestamp, rcstate 
		return new DQMViewChannelHandler(channelName);
	}

}

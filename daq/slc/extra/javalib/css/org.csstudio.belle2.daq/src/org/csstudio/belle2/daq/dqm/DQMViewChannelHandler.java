package org.csstudio.belle2.daq.dqm;

import java.text.MessageFormat;
import java.util.HashMap;
import java.util.Map;

import org.belle2.daq.dqm.Histo;
import org.epics.pvmanager.ChannelWriteCallback;
import org.epics.pvmanager.MultiplexedChannelHandler;
import org.epics.vtype.VNumber;
import org.epics.vtype.VString;
import org.epics.vtype.ValueFactory;

public class DQMViewChannelHandler extends MultiplexedChannelHandler<Object, Object> {

	private Number currentValue = null;
	private String currentString = null;
	private String dirname = "";
	private String histname = "";
	private String parname = "";

	public DQMViewChannelHandler(String channelName) {
		super(channelName);
		String [] s = channelName.split(":");
		if (s.length > 2) {
			dirname = s[0];
			histname = s[1];
			parname = s[2];
		} else if (s.length > 1) {
			dirname = "";
			histname = s[0];
			parname = s[1];
		} else if (s.length > 0) {
			dirname = "";
			histname = s[0];
			parname = "title";
		}
		DQMViewCommunicator.add(this);
	}

	public void update() {
		for (DQMViewCommunicator store : DQMViewCommunicator.get()) {
			for (String dname : store.getHists().keySet()) {
				if (dname.equals(dirname)) {
					for (Histo h : store.getHists().get(dname)) {
						if (h.getName().equals(histname)) {
							switch (parname) {
							case "entries":
								updateValue(h.getEntries());
								break;
							case "mean":
								updateValue(h.getMean());
								break;
							case "rms":
								updateValue(h.getRMS());
								break;
							case "title":
								updateValue(ValueFactory.newVString(
										h.getTitle(), ValueFactory.alarmNone(),
										ValueFactory.timeNow()));
								break;
							default:
								if (parname.startsWith("bincontent")) {
									updateValueWithBinContent(h);
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	
	private void updateValueWithBinContent(Histo h) {
		try {
			try {
				if (parname.contains("][")) {
					MessageFormat mf = new MessageFormat("bincontent[{0}][{1}]");
					Object[] result = mf.parse(parname);
					int nx= Integer.parseInt((String)result[0]);
					int ny= Integer.parseInt((String)result[1]);
					updateValue(h.getBinContent(nx, ny));
					return;
				}
			} catch (Exception e) {
				System.err.println(parname);
				e.printStackTrace();
			}
			MessageFormat mf = new MessageFormat("bincontent[{0}]");
			Object[] result = mf.parse(parname);
			int nx= Integer.parseInt((String)result[0]);
			updateValue(h.getBinContent(nx));
			return;
		} catch (Exception e) {
			System.err.println(parname);
			e.printStackTrace();
		}
	}

  public Map<String, Object> getProperties() {
    Map<String, Object> properties = new HashMap<String, Object>();
    properties.put("Directory ", dirname);
    properties.put("Histogram name", histname);
    properties.put("Parameter type", parname);
    return properties;
  }

  private void updateValue(double val) {
		updateValue(ValueFactory.newVDouble(val, ValueFactory.alarmNone(), 
				ValueFactory.timeNow(), ValueFactory.displayNone()));
	}

	private void updateValue(final VNumber newValue) {
		if ((null != currentValue) && (newValue.getValue().equals(currentValue))) {
			return;
		}
		currentValue = newValue.getValue();
		processMessage(newValue);
	}

	private void updateValue(final VString newValue) {
		if ((null != currentString) && (newValue.getValue().equals(currentString))) {
			return;
		}
		currentString = newValue.getValue();
		processMessage(newValue);
	}
	
	@Override
	protected void write(Object newValue, ChannelWriteCallback callback) {
		// TODO Auto-generated method stub

	}

	@Override
	protected void connect() {
		processConnection(new Object());
		currentValue = null;
		currentString = null;
		update();
	}

	@Override
	protected void disconnect() {
		processConnection(null);
	}

}

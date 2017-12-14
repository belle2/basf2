package org.csstudio.belle2.daq.nsm;

import java.io.IOException;
import java.util.HashMap;

import org.belle2.daq.hvcontrol.HVCommand;
import org.belle2.daq.nsm.NSMCommand;
import org.belle2.daq.nsm.NSMMessage;
import org.belle2.daq.nsm.NSMRequestHandler;
import org.belle2.daq.nsm.NSMVar;
import org.belle2.daq.runcontrol.RCCommand;
import org.epics.pvmanager.ChannelWriteCallback;
import org.epics.pvmanager.MultiplexedChannelHandler;
import org.epics.vtype.VNumber;
import org.epics.vtype.VString;
import org.epics.vtype.ValueFactory;

public class NSMChannelHandler extends MultiplexedChannelHandler<Object, Object> {

	static HashMap<String, NSMRequestHandler> g_handlers = new HashMap<>();
	static HashMap<String, Object> g_values = new HashMap<>();

	private Object currentValue = null;
	private String currentString = null;
	private String m_nodename = "";
	private String m_parname = "";
	private String m_type = "";

	public NSMChannelHandler(final String pvname) throws Exception {
		super(pvname);
		String[] str = pvname.split(":", 3);
		m_nodename = str[1];
		m_type = str[0];
		synchronized (g_handlers) {
			// for the first NSM variable that's requested
			if (!g_handlers.containsKey(pvname)) {
				if (m_type.matches("data")) {
					String[] str2 = pvname.split(":", 4);
					m_parname = str2[3].replace(":", ".");
				} else if (m_type.matches("get")) {
					m_parname = str[2].replace(":", ".");
				} else if (m_type.matches("set")) {
					m_parname = str[2].replace(":", ".");
					//update(0);
				} else if (m_type.matches("req")) {
					if (str.length>2) m_parname = str[2];
					else m_parname = "";
					update("");
				} else {
					System.err.println("Unknown type : " + m_type + " " + pvname);
				}
			}
		}
	}

	@Override
	protected void connect() {
		processConnection(new Object());
		if (m_type.matches("data")) {
			if (currentValue != null)
				update(currentValue);
		} else if (m_type.matches("get")) {
			try {
				NSMDataSource.request(getChannelName(), m_nodename, m_parname);
			} catch (IOException e) {
			}
			if (currentValue != null)
				update(currentValue);
			else if (currentString != null)
				update(currentString);
		} else if (m_type.matches("set")) {
			try {
				NSMDataSource.request(getChannelName(), m_nodename, m_parname);
			} catch (IOException e) {
			}
			if (currentValue != null)
				update(currentValue);
			else if (currentString != null)
				update(currentString);
		} else if (m_type.matches("req")) {
			update("");
		} 
	}

	@Override
	protected void disconnect() {
		processConnection(null);
	}

	@Override
	protected void write(Object newValue, ChannelWriteCallback callback) {
		try {
			if (m_type.matches("set")) {
				if (currentValue != null) {
					if (currentValue instanceof Integer) {
						try {
							int val = 0;
							if (newValue instanceof String) {
								val = Integer.parseInt((String)newValue);
							} else if (newValue instanceof Number) {
								val = (int)((Number)newValue).intValue();
							}
							NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
							update(val);
						} catch (Exception e) {
							String str = (String)newValue;
							NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, str));
							update(str);
						}
						callback.channelWritten(null);
						return;
					} else if (currentValue instanceof Float || currentValue instanceof Double) {
						try {
							float val = 0;
							if (newValue instanceof String) {
								val = Float.parseFloat((String)newValue);
							} else if (newValue instanceof Number) {
								val = (float)((Number)newValue).floatValue();
							}
							NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
							update(val);
						} catch (Exception e) {
							String str = (String)newValue;
							NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, str));
							update(str);
						}
						callback.channelWritten(null);
						return;	
					} else if (currentValue instanceof Integer) {
						String val = "";
						if (newValue instanceof String) {
							val = (String) newValue;
						} else if (newValue instanceof Number) {
							val = ((Number) newValue).toString();
						}
						NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
						update(val);
						callback.channelWritten(null);
						return;
					}
				} if (newValue instanceof String) {
					String val = (String) newValue;
					NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
					update(val);
				} else if (newValue instanceof Integer) {
					int val = (Integer) newValue;
					NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
					update(val);
				} else if (newValue instanceof Float) {
					float val = (Float) newValue;
					NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, val));
					update(val);
				} else if (newValue instanceof Double) {
					double val = (Double) newValue;
					NSMDataSource.request(getChannelName(), m_nodename, new NSMVar(m_parname, (float)val));
					update(val);
				}
			} else if (m_type.matches("req")) {
				if (newValue instanceof String) {
					String cmdname = (String) newValue;
					if (m_parname.length() > 0) {
						cmdname = m_parname;
					}
					String val = (String) newValue;
					NSMCommand cmd = new RCCommand(cmdname.toUpperCase());
					if (cmd.equals(NSMCommand.UNKNOWN))
						cmd = new HVCommand(cmdname.toUpperCase());
					if (!cmd.equals(NSMCommand.UNKNOWN)) {
						NSMDataSource.request(getChannelName(), new NSMMessage(m_nodename, cmd, (m_parname.length()>0?val:"")));
					} else {
						System.out.println("Wrong command : " + cmdname);
					}
					update(val);
				}
			}
			callback.channelWritten(null);
		} catch (IOException e) {
			e.printStackTrace();
			callback.channelWritten(e);
		}
	}

	@Override
	protected boolean isConnected(Object payload) {
		return true;
	}

	@Override
	protected boolean isWriteConnected(Object payload) {
		return isConnected(payload);
	}

	public void updateValue(final VNumber newValue) {
		processMessage(newValue);
	}

	public void updateValue(final VString newValue) {
		processMessage(newValue);
	}

	public void update (Object obj) {
		VNumber vnum = null; 	
		if (obj instanceof Character || obj instanceof Short || obj instanceof Integer || obj instanceof Long) {
			int val = 0;
			if (obj instanceof Character) { 
				val = (int)((Character)obj);
			} else if (obj instanceof Short) {
				val = (int)((Short)obj);
			} else if (obj instanceof Integer) {
				val = (int)((Integer)obj);
			} else if (obj instanceof Long) {
				val = (int)((Long)obj).intValue();
			}
			currentValue = val;
			vnum = ValueFactory.newVInt(val, ValueFactory.alarmNone(), 
					ValueFactory.timeNow(), ValueFactory.displayNone());
		} else if (obj instanceof Float || obj instanceof Double) {
			double val = 0;
			if (obj instanceof Float) {
				val = (double)((Float)obj);
			} else if (obj instanceof Double) {
				val = (double)((Double)obj);
			}
			currentValue = val;
			vnum = ValueFactory.newVDouble(val, ValueFactory.alarmNone(), 
					ValueFactory.timeNow(), ValueFactory.displayNone());
		}
		if (vnum != null) {
			updateValue(vnum);
		}
	}

	public void update (String val) {
		currentString = val;
		updateValue(ValueFactory.newVString(val, ValueFactory.alarmNone(), ValueFactory.timeNow()));
	}

}

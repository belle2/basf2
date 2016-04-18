package org.csstudio.belle2.daq.nsm;

import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.logging.Logger;

import javax.security.auth.Subject;

import org.belle2.daq.nsm.NSMCommand;
import org.belle2.daq.nsm.NSMCommunicator;
import org.belle2.daq.nsm.NSMData;
import org.belle2.daq.nsm.NSMMessage;
import org.belle2.daq.nsm.NSMRequestHandler;
import org.belle2.daq.nsm.NSMVar;
import org.csstudio.opibuilder.preferences.StringTableFieldEditor;
import org.csstudio.security.SecuritySupport;
import org.eclipse.core.runtime.Platform;
import org.eclipse.core.runtime.preferences.IPreferencesService;
import org.epics.pvmanager.ChannelHandler;
import org.epics.pvmanager.DataSource;

public class NSMDataSource extends DataSource {

	static private NSMDataSource g_source = null;
	
	public final static String PLUGIN_ID = "org.csstudio.belle2.daq";

	private final Logger log = Logger.getLogger(NSMDataSource.class.getName());
	//private final ArrayList<NSMCommunicator> m_coms = new ArrayList<>();
	private final HashMap<String, NSMCommunicator> m_com_map = new HashMap<>();
	private final PVNSMVSetHandler m_sethandler = new PVNSMVSetHandler();
	private final PVNSMDATASetHandler m_datahandler = new PVNSMDATASetHandler();
		
	static public void request(String pvname, NSMMessage msg) throws IOException {
		if (g_source == null) return;
		String s [] = pvname.split(":");
		String chname = s[0]+"."+s[1];
		ArrayList<NSMCommunicator> coms = NSMCommunicator.get();
		synchronized (g_source) {
			if (g_source.m_com_map.containsKey(chname)) {
				g_source.m_com_map.get(chname).request(msg);
			} else {
				for (NSMCommunicator com : coms) {
					try {
						com.request(msg);
					} catch (IOException e) {
					}
				}
			}
		}
	}
	
	static public void request(String pvname, String nodename, String parname) throws IOException {
		if (g_source == null) return;
		String s [] = pvname.split(":");
		String chname = s[0]+"."+s[1];
		ArrayList<NSMCommunicator> coms = NSMCommunicator.get();
		synchronized (g_source) {
			if (g_source.m_com_map.containsKey(chname)) {
				g_source.m_com_map.get(chname).requestVGet(nodename, parname);
			} else {
				for (NSMCommunicator com : coms) {
					try {
						com.requestVGet(nodename, parname);
					} catch (IOException e) {
					}
				}
			}
		}
	}
	
	static public void request(String pvname, String nodename, NSMVar var) throws IOException {
		if (g_source == null) return;
		String s [] = pvname.split(":");
		String chname = s[0]+"."+s[1];
		ArrayList<NSMCommunicator> coms = NSMCommunicator.get();
		synchronized (g_source) {
			if (g_source.m_com_map.containsKey(chname)) {
				g_source.m_com_map.get(chname).requestVSet(nodename, var);
			} else {
				for (NSMCommunicator com : coms) {
					try {
						com.requestVSet(nodename, var);
					} catch (IOException e) {
					}
				}
			}
		}
	}
	public NSMDataSource() {
		super(true);
		if (g_source == null) {
			g_source = this;
			log.info("startNSM");
			startNSM();
		}
	}

	@Override
	protected ChannelHandler createChannel(String channelName) {
		try {
			// nsm://data:<DATA>:<FOMAT>:<PARNAME>
			// nsm://get:<NODE>:<VARNAME>
			// nsm://set:<NODE>:<VARNAME>
			// nsm://req:<NODE>
			String s [] = channelName.split(":");
			NSMChannelHandler channel = new NSMChannelHandler(channelName);
			if (s.length >= 3) {
				switch (s[0]) {
				case "data": {
					String[] ss = channelName.split(":", 4);
					m_datahandler.add(channel, ss[1], ss[2], ss[3]);
					break;
				}
				case "set": {
					String[] ss = channelName.split(":", 3);
					m_sethandler.add(channel, ss[1], ss[2]);
					break;
				}
				case "get": {
					String[] ss = channelName.split(":", 3);
					m_sethandler.add(channel, ss[1], ss[2]);
					break;
				}
				case "req": {
					break;
				}
				}
			} else if (s.length== 2 && s[0].equals("req")) {
			}
			return channel;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	static public void startNSM() {
		try {
			final IPreferencesService prefs = Platform.getPreferencesService();
			final String s = prefs.getString(PLUGIN_ID, PreferenceConstants.NSM, "localhost,9090,eb02pc,9123", null);
			synchronized (g_source) {
				List<String[]> ss = StringTableFieldEditor.decodeStringTable(s);
				NSMCommunicator.closeAll();
				int i = 0;
				for (String[] cs : ss) {
					try {
						String host = cs[0];
						int port = Integer.parseInt(cs[1]);
						String nsmhost = cs[2];
						int nsmport = Integer.parseInt(cs[3]);
						NSMCommunicator com = new NSMCommunicator();
						com.add(g_source.m_datahandler);
						com.add(g_source.m_sethandler);
						String nsmnode = "";
						if (nsmnode.isEmpty()) {
							nsmnode = InetAddress.getLocalHost().getHostName();
							Subject user = SecuritySupport.getSubject();
							String [] sss = nsmnode.split(".");
							if (sss.length > 1) nsmnode = sss[0];
							if (null != user)
								nsmnode += "_" + SecuritySupport.getSubjectName(user);
						}
						nsmnode = nsmnode.replaceAll("-", "_").replaceAll("\\.", "_") + "_" + i;
						com.reconnect(host, port, nsmnode, nsmhost, nsmport);
						NSMCommunicator.get().add(com);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		} catch (Exception e1) {
			e1.printStackTrace();
		}
	}

	private abstract class PVNSMRequestHandler extends NSMRequestHandler {

		protected HashMap<String, NSMChannelHandler> m_chmap = new HashMap<>();  
		protected ArrayList<NSMChannelHandler> m_channel = new ArrayList<>();  
		protected ArrayList<String> m_chname = new ArrayList<>();  
		
	    public PVNSMRequestHandler() {
	        super(false);
	    }
	    abstract protected void connected(NSMCommunicator com, String node, String name) throws IOException;

		public void add(NSMChannelHandler channel, String node, String pname) {
			String chname = node+"."+pname; 
			m_channel.add(channel);
			m_chname.add(chname);
			if (!m_chmap.containsKey(chname)) {
				ArrayList<NSMCommunicator> coms = NSMCommunicator.get();
				synchronized(coms) {
					for (NSMCommunicator com : coms) {
						try {
							connected(com, node, pname);
						} catch (IOException e) {
						}
					}
				}
				m_chmap.put(chname, channel);
			}
		}

		public boolean connected(NSMCommunicator com) {
			for (String chname : m_chmap.keySet()) {
				String s[] = chname.split(".");
				try {
					if (s.length > 1) {
						connected(com, s[0], s[1]);
					}
				} catch (IOException e) {
				}
			}
			return false;
		}
	}
	
	private class PVNSMVSetHandler extends PVNSMRequestHandler {

	    public PVNSMVSetHandler() {
	        super();
	    }

		@Override
		public void connected(NSMCommunicator com, String node, String name) throws IOException {
			com.requestVGet(node, name.replace(":", "."));
		}

		@Override
		public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
			if (cmd.equals(NSMCommand.VSET)) {
				NSMVar var = (NSMVar) msg.getObject();
				for (int i = 0; i < m_chname.size(); i++) {
					String chname = m_chname.get(i);
					NSMChannelHandler channel = m_channel.get(i);
					if (chname.equals(var.getNode() + "." + var.getName().replace(".", ":"))) {
						m_com_map.put(chname, com);
						switch (var.getType()) {
						case NSMVar.INT:
							channel.update(var.getInt());
							break;
						case NSMVar.FLOAT:
							channel.update(var.getFloat());
							break;
						case NSMVar.TEXT:
							channel.update(var.getText());
							break;
						}
					}
				}
				return true;
			}
			return false;
		}
		
	}
	
	private class PVNSMDATASetHandler extends PVNSMRequestHandler {

		private ArrayList<String> m_parname = new ArrayList<>();
		
	    public PVNSMDATASetHandler() {
	        super();
	    }

		public void add(NSMChannelHandler channel, String node, String format, String parname) {
			super.add(channel, node, format);
			m_parname.add(parname);
		}
		
		@Override
		public void connected(NSMCommunicator com, String node, String name) throws IOException {
			com.requestDATAGet(node, name);
		}

		@Override
		public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
			if (cmd.equals(NSMCommand.DATASET)) {
				NSMData data = (NSMData) msg.getObject();
				if (data == null) return false;
				for (int i = 0; i < m_chname.size(); i++) {
					String chname = m_chname.get(i);
					String parname = m_parname.get(i);
					NSMChannelHandler channel = m_channel.get(i);
					if (chname.equals(data.getName() + "." + data.getFormat())) {
						try {
							m_com_map.put(chname, com);
							Object obj = data.find(parname.replace(":", "."));
							if (obj == null) {
								System.out.println("No data for " + parname);
								continue;
							}
							channel.update(obj);
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
			}
			return false;
		}
		
	}
}

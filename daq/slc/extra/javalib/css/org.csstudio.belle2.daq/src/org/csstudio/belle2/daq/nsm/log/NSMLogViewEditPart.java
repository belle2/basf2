package org.csstudio.belle2.daq.nsm.log;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;

import javax.security.auth.Subject;

import org.belle2.daq.base.LogMessage;
import org.belle2.daq.nsm.NSMCommand;
import org.belle2.daq.nsm.NSMCommunicator;
import org.belle2.daq.nsm.NSMLogHandler;
import org.belle2.daq.nsm.NSMMessage;
import org.csstudio.opibuilder.editparts.AbstractPVWidgetEditPart;
import org.csstudio.opibuilder.properties.IWidgetPropertyChangeHandler;
import org.csstudio.security.SecuritySupport;
import org.eclipse.core.runtime.IProduct;
import org.eclipse.core.runtime.Platform;
import org.eclipse.draw2d.IFigure;

public class NSMLogViewEditPart extends AbstractPVWidgetEditPart {

	private static final HashMap<String, NSMCommunicator> m_com = new HashMap<>();
	private static final HashMap<NSMCommunicator, ArrayList<LogMessage>> m_msgs = new HashMap<>();
	private final NSMLogViewHandler m_handler = new NSMLogViewHandler();

	public NSMLogViewEditPart() {
	}

	public void activate() {
		super.activate();
		String nsmhost = getWidgetModel().getNSMHost();
		int nsmport = getWidgetModel().getNSMPort();
		NSMCommunicator com = null;
		ArrayList<LogMessage> msgs = null;
		if (m_com.containsKey(nsmhost+":"+nsmport)) {
			com = m_com.get(nsmhost+":"+nsmport);
			msgs = m_msgs.get(com);
			for (LogMessage lmsg : msgs) {
				((NSMLogViewFigure)figure).add(lmsg);
			}
		} else {
			com = new NSMCommunicator();
			String nsmnode = "";
			if (nsmnode.isEmpty()) {
				try {
					nsmnode = InetAddress.getLocalHost().getHostName();
				} catch (UnknownHostException e) {
					e.printStackTrace();
				}
				Subject user = SecuritySupport.getSubject();
				String [] sss = nsmnode.split(".");
				if (sss.length > 1) nsmnode = sss[0];
				if (null != user)
					nsmnode += "_"+SecuritySupport.getSubjectName(user);
			}
			nsmnode = nsmnode.replaceAll("-", "_").replaceAll("\\.", "_") + "_LOG";
			String host = getWidgetModel().getHost();
			int port = getWidgetModel().getPort();
			com.reconnect(host, port, nsmnode, nsmhost, nsmport);
			m_com.put(nsmhost+":"+nsmport, com);
			msgs = new ArrayList<LogMessage>();
			m_msgs.put(com, msgs);
			System.out.println("activate : "+nsmnode);
		}
		com.add(m_handler);
	}

	@Override
	public void deactivate() {
		super.deactivate();
		for (NSMCommunicator com : m_com.values()) {
			com.remove(m_handler);
		}
	}
	
	@Override
	public NSMLogViewModel getWidgetModel() {
		return (NSMLogViewModel) super.getWidgetModel();
	}
	
	@Override
	protected IFigure doCreateFigure() {
		NSMLogViewFigure figure = new NSMLogViewFigure(this);
		return figure;
	}

	@Override
	protected void registerPropertyChangeHandlers() {
		setPropertyChangeHandler(NSMLogViewModel.node, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue, final Object newValue, final IFigure figure) {
				if (newValue == null)
					return false;
				NSMLogViewFigure view = ((NSMLogViewFigure) figure);
				view.setPriority((String) newValue);
				return false;
			}
		});
	}

	private class NSMLogViewHandler extends NSMLogHandler {

		public NSMLogViewHandler() {
			super(false);
		}

		public boolean connected(NSMCommunicator com) {
			try {
				com.request(new NSMMessage(NSMCommand.LOGGET));
			} catch (IOException e) {
			}
			return true;
		}
		
		@Override
		public boolean handleLog(final LogMessage lmsg, NSMCommunicator com) {
			((NSMLogViewFigure)figure).add(lmsg);
			if (m_msgs.get(com) != null) {
				ArrayList<LogMessage> msgs = m_msgs.get(com);
				msgs.add(lmsg);
				while (msgs.size() > 200) {
					msgs.remove(0);
				}
			}
			return false;
		}
		
	}
	
}

package org.csstudio.belle2.daq.nsm.log;

import java.io.IOException;

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

	private final NSMCommunicator m_com = new NSMCommunicator();

	private NSMLogViewHandler m_handler;

	private String nsmnode;
	
	private static int g_nfigure = 0;
	
	public NSMLogViewEditPart() {
		nsmnode = "";
		if (nsmnode.isEmpty()) {
			IProduct product = Platform.getProduct();
			if (null != product) nsmnode = product.getName() + "_";
			Subject user = SecuritySupport.getSubject();
			if (null != user) nsmnode += SecuritySupport.getSubjectName(user);
		}
		nsmnode = nsmnode.replaceAll("-", "_").replaceAll("\\.", "_") + "_LOG_" + g_nfigure;
		g_nfigure++;
		m_handler = new NSMLogViewHandler(m_com);
		m_com.add(m_handler);
	}

	public void activate() {
		super.activate();
		m_com.close();
		//m_com.stop();
		String nsmhost = getWidgetModel().getNSMHost();
		int nsmport = getWidgetModel().getNSMPort();
		String host = getWidgetModel().getHost();
		int port = getWidgetModel().getPort();
		try {
			m_com.reconnect(host, port, nsmnode, nsmhost, nsmport);
		} catch (IOException e) {
			e.printStackTrace();
		}
		System.out.println("activate : "+nsmnode);
	}

	@Override
	public void deactivate() {
		super.deactivate();
		m_com.close();
		m_com.stop();
		System.out.println("deactivate : "+nsmnode);
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

		public NSMLogViewHandler(NSMCommunicator com) {
			super(false);
			m_com = com; 
		}

		public boolean connected() {
			try {
				m_com.request(new NSMMessage(NSMCommand.LOGGET));
			} catch (IOException e) {
			}
			return true;
		}
		
		@Override
		public boolean handleLog(final LogMessage lmsg) {
			((NSMLogViewFigure)figure).add(lmsg);
			return false;
		}
		
	}
	
}

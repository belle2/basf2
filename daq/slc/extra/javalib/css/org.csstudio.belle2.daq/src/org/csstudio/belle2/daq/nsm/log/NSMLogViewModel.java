package org.csstudio.belle2.daq.nsm.log;

import org.csstudio.opibuilder.model.AbstractPVWidgetModel;
import org.csstudio.opibuilder.properties.IntegerProperty;
import org.csstudio.opibuilder.properties.StringProperty;
import org.csstudio.opibuilder.properties.WidgetPropertyCategory;

public class NSMLogViewModel extends AbstractPVWidgetModel {

	public static final String ID = "NSMLogView";
	public static final String priority = "priority";
	public static final String nsmhost = "nsmhost";
	public static final String nsmport = "nsmport";
	public static final String host = "host";
	public static final String port = "port";
	public static final String node = "node";
	
	public final static WidgetPropertyCategory NSM = new WidgetPropertyCategory() {
		public String toString() {
			return "NSM";
		}
	};

	public NSMLogViewModel() {
		setSize(50, 100);
	}
	
	@Override
	protected void configureProperties() {
		addProperty(new StringProperty(priority, "Priority", WidgetPropertyCategory.Basic, "DEBUG"));
		addProperty(new StringProperty(node, "Node", WidgetPropertyCategory.Basic, ""));
		addProperty(new StringProperty(nsmhost, "NSMHost", NSM, "store01.b2nsm.kek.jp"));
		addProperty(new IntegerProperty(nsmport, "NSMPort", NSM, 9124));
		addProperty(new StringProperty(host, "Host", NSM, "store01.daqnet.kek.jp"));
		addProperty(new IntegerProperty(port, "Port", NSM, 9090));
	}

	@Override
	public String getTypeID() {
		return ID;
	}

	public String getPriority() {
		return getCastedPropertyValue(priority);
	}

	public String getNode() {
		return getCastedPropertyValue(node);
	}

	public String getNSMHost() {
		return getCastedPropertyValue(nsmhost);
	}

	public int getNSMPort() {
		return getCastedPropertyValue(nsmport);
	}

	public String getHost() {
		return getCastedPropertyValue(host);
	}

	public int getPort() {
		return getCastedPropertyValue(port);
	}

}

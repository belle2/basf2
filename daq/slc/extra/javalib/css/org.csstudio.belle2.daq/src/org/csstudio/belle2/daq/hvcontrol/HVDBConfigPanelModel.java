package org.csstudio.belle2.daq.hvcontrol;

import org.csstudio.opibuilder.model.AbstractPVWidgetModel;
import org.csstudio.opibuilder.properties.IntegerProperty;
import org.csstudio.opibuilder.properties.StringProperty;
import org.csstudio.opibuilder.properties.WidgetPropertyCategory;
//import org.eclipse.swt.graphics.RGB;

public class HVDBConfigPanelModel extends AbstractPVWidgetModel {

	public static final String ID = "HVDBConfigPanel";
	public static final String dbhost = "dbhost";
	public static final String dbport = "dbport";
	public static final String dbtable = "dbtable";
	public static final String hvnode = "hvnode";
	public static final String hvconfig = "hvconfig";
	
	public HVDBConfigPanelModel() {
		//setForegroundColor(new RGB(255, 0, 0));
		//setBackgroundColor(new RGB(0,0,255));
		setSize(50, 100);
	}
	
	@Override
	protected void configureProperties() {
		addProperty(new StringProperty(dbhost, "DBHost", WidgetPropertyCategory.Basic, "localhost"));
		addProperty(new IntegerProperty(dbport, "DBPort", WidgetPropertyCategory.Basic, 20020));
		addProperty(new StringProperty(dbtable, "DBTable", WidgetPropertyCategory.Basic, "daq"));
		addProperty(new StringProperty(hvnode,  "HVNode", WidgetPropertyCategory.Basic, ""));
		addProperty(new StringProperty(hvconfig,  "HVConifg", WidgetPropertyCategory.Basic, ""));
	}

	@Override
	public String getTypeID() {
		return ID;
	}

	public String getDBHost() {
		return getCastedPropertyValue(dbhost);
	}

	public int getDBPort() {
		return getCastedPropertyValue(dbport);
	}

	public String getDBTable() {
		return getCastedPropertyValue(dbtable);
	}

	public String getHVNode() {
		return getCastedPropertyValue(hvnode);
	}

	public String getHVConfig() {
		return getCastedPropertyValue(hvconfig);
	}

}

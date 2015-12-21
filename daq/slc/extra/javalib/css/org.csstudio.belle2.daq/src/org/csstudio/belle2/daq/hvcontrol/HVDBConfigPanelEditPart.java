package org.csstudio.belle2.daq.hvcontrol;

import org.csstudio.opibuilder.editparts.AbstractPVWidgetEditPart;
import org.csstudio.opibuilder.properties.IWidgetPropertyChangeHandler;
import org.eclipse.draw2d.IFigure;

public class HVDBConfigPanelEditPart extends AbstractPVWidgetEditPart {

	public HVDBConfigPanelEditPart() {
	}

	@Override
	public HVDBConfigPanelModel getWidgetModel() {
		return (HVDBConfigPanelModel) super.getWidgetModel();
	}
	
	@Override
	protected IFigure doCreateFigure() {
		HVDBConfigPanelFigure figure = new HVDBConfigPanelFigure(this);
		figure.setDBTable(getWidgetModel().getDBTable());
		figure.setDBHost(getWidgetModel().getDBHost());
		figure.setDBPort(getWidgetModel().getDBPort());
		figure.setHVNode(getWidgetModel().getHVNode());
		return figure;
	}

	@Override
	protected void registerPropertyChangeHandlers() {
		setPropertyChangeHandler(HVDBConfigPanelModel.dbhost, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue,
					final Object newValue,
					final IFigure figure) {
				if(newValue == null)
					return false;
				((HVDBConfigPanelFigure) figure).setDBHost((String)newValue);
				return false;
			}
		});
		setPropertyChangeHandler(HVDBConfigPanelModel.dbport, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue,
					final Object newValue,
					final IFigure figure) {
				if(newValue == null)
					return false;
				((HVDBConfigPanelFigure) figure).setDBPort((Integer)newValue);
				return false;
			}
		});
		setPropertyChangeHandler(HVDBConfigPanelModel.dbtable, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue,
					final Object newValue,
					final IFigure figure) {
				if(newValue == null)
					return false;
				((HVDBConfigPanelFigure) figure).setDBTable((String)newValue);
				return false;
			}
		});
		setPropertyChangeHandler(HVDBConfigPanelModel.hvnode, new IWidgetPropertyChangeHandler() {
			public boolean handleChange(final Object oldValue,
					final Object newValue,
					final IFigure figure) {
				if(newValue == null)
					return false;
				((HVDBConfigPanelFigure) figure).setHVNode((String)newValue);
				return false;
			}
		});
	}

}

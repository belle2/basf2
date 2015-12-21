package org.csstudio.belle2.daq.hvcontrol;

import org.belle2.daq.database.DBLoader;
import org.belle2.daq.database.DBObject;
import org.belle2.daq.database.FieldInfo;
import org.csstudio.opibuilder.editparts.AbstractBaseEditPart;
import org.csstudio.opibuilder.widgets.figures.AbstractSWTWidgetFigure;
import org.eclipse.swt.widgets.Composite;

import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.jface.viewers.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Table;
import org.eclipse.jface.layout.TableColumnLayout;
import org.eclipse.swt.widgets.TableColumn;

import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Combo;

public class HVDBConfigPanelFigure extends AbstractSWTWidgetFigure<Composite> {

	private Table table;
	private TableViewer tableViewer;
	private TableColumn m_col_property;
	private TableColumn m_col_value;
	private Text text_dbtable;
	private Text text_search;
	private Combo combo;
	private String[] list;
	//private PVReader<VString> pvr_config_standby;
	private String dbhost = "localhost";
	private int dbport = 20020;
	private String dbtable = "daq";
	private AbstractBaseEditPart m_editpart;

	public HVDBConfigPanelFigure(AbstractBaseEditPart editpart) {
		super(editpart);
		m_editpart = editpart;
	}

	@Override
	protected Composite createSWTWidget(Composite parent, int style) {
		Composite composite = new Composite(parent, SWT.NONE);
		GridLayout layout = new GridLayout();
		composite.setLayout(layout);
		text_dbtable = new Text(composite, SWT.BORDER);
		text_dbtable.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		text_dbtable.setText("konnohv");

		text_search = new Text(composite, SWT.BORDER);
		text_search.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		text_search.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent event) {
				dbtable = text_dbtable.getText();
				String config = text_search.getText();
				if (config.length() > 0) config += "@";
				try {
					list = DBLoader.loadList(dbhost, dbport, dbtable, config);
				} catch (Exception e) {
					return;
				}
				if (list == null) return;
				combo.getDisplay().asyncExec(new Runnable() {
					@Override
					public void run() {
						combo.removeAll();
						for (String s : list) {
							combo.add(s);
						}
					}
				});
			}
		});
		combo = new Combo(composite, SWT.NONE);
		combo.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		combo.addSelectionListener(new SelectionListener() {
			@Override
			public void widgetDefaultSelected(SelectionEvent e) {
			}

			@Override
			public void widgetSelected(SelectionEvent e) {
				Combo bCombo = (Combo) e.widget;
				dbtable = text_dbtable.getText();
				String config = bCombo.getText();
				table.getDisplay().asyncExec(new UpdateTable(DBLoader.load(dbhost, dbport, dbtable, config)));
				//if (pvr_config_standby != null) {
				//	pvr_config_standby.close();
				//}
				String[] ss = config.split("@");
				if (ss.length > 1)
					config = ss[1];
				m_editpart.setPropertyValue(HVDBConfigPanelModel.hvconfig, config);
			}
		});
		Composite composite1 = new Composite(composite, SWT.NONE);
		TableColumnLayout tcl_composite = new TableColumnLayout();
		composite1.setLayout(tcl_composite);
		composite1.setLayoutData(new GridData(GridData.FILL_BOTH));

		tableViewer = new TableViewer(composite1, SWT.BORDER | SWT.FULL_SELECTION);
		table = tableViewer.getTable();
		table.setHeaderVisible(true);
		table.setLinesVisible(true);

		m_col_property = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_property, new ColumnPixelData(150, true, true));
		m_col_property.setText("Property");

		m_col_value = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_value, new ColumnPixelData(150, true, true));
		m_col_value.setText("Value");

		return composite;
	}

	void set(String prefix, DBObject obj) {
		for (String name : obj.getFieldNames()) {
			if (obj.getProperty(name).getType() == FieldInfo.TEXT) {
				TableItem item = new TableItem(table, SWT.NULL);
				String label = (prefix.length() > 0) ? prefix + "." + name : name;
				item.setText(0, label);
				item.setText(1, obj.getText(name));
			} else if (obj.getProperty(name).getType() != FieldInfo.OBJECT) {
				TableItem item = new TableItem(table, SWT.NULL);
				String label = (prefix.length() > 0) ? prefix + "." + name : name;
				item.setText(0, label);
				item.setText(1, obj.getValueText(name));
			} else {
				int i = 0;
				int len = obj.getObjects(name).size();
				for (DBObject cobj : obj.getObjects(name)) {
					String label = ((len > 1) ? name + "[" + i + "]" : "");
					if (prefix.length() > 0)
						label = prefix + "." + name;
					set(label, cobj);
					i++;
				}
			}
		}
	}

	public String getDBHost() {
		return dbhost;
	}

	public void setDBHost(String dbhost) {
		this.dbhost = dbhost;
	}

	public int getDBPort() {
		return dbport;
	}

	public void setDBPort(int dbport) {
		this.dbport = dbport;
	}

	public String getDBTable() {
		return dbtable;
	}

	public void setDBTable(String dbtable) {
		this.dbtable = dbtable;
		text_dbtable.setText(dbtable);
	}

	public void setHVNode(String node) {
		if (node != null)
			text_search.setText(node);
	}

	class UpdateTable implements Runnable {
		private DBObject m_obj;

		public UpdateTable(DBObject obj) {
			m_obj = obj;
		}

		@Override
		public void run() {
			table.clearAll();
			table.removeAll();
			set("", m_obj);
		}
	}

}

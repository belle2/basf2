package org.csstudio.belle2.daq.nsm.log;

import org.belle2.daq.base.LogLevel;
import org.belle2.daq.base.LogMessage;
import org.belle2.daq.graphics.HtmlColor;
import org.belle2.daq.graphics.JavaGraphicDrawer;
import org.csstudio.opibuilder.editparts.AbstractBaseEditPart;
import org.csstudio.opibuilder.widgets.figures.AbstractSWTWidgetFigure;
import org.eclipse.swt.widgets.Composite;

import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.jface.viewers.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Table;
import org.eclipse.jface.layout.TableColumnLayout;
import org.eclipse.swt.widgets.TableColumn;

public class NSMLogViewFigure extends AbstractSWTWidgetFigure<Composite> {

	private Table table;
	private TableViewer tableViewer;
	private TableColumn m_col_date;
	private TableColumn m_col_from;
	private TableColumn m_col_level;
	private TableColumn m_col_message;
	private LogLevel m_loglevel = LogLevel.DEBUG;

	public NSMLogViewFigure(AbstractBaseEditPart editpart) {
		super(editpart);
	}

	@Override
	protected Composite createSWTWidget(Composite parent, int style) {
		Composite composite = new Composite(parent, SWT.NONE);
		GridLayout layout = new GridLayout();
		composite.setLayout(layout);

		Composite composite1 = new Composite(composite, SWT.NONE);
		TableColumnLayout tcl_composite = new TableColumnLayout();
		composite1.setLayout(tcl_composite);
		composite1.setLayoutData(new GridData(GridData.FILL_BOTH));

		tableViewer = new TableViewer(composite1, SWT.BORDER | SWT.FULL_SELECTION);
		table = tableViewer.getTable();
		table.setHeaderVisible(true);
		table.setLinesVisible(true);

		m_col_date = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_date, new ColumnPixelData(150, false));
		m_col_date.setText("Date");

		m_col_from = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_from, new ColumnPixelData(100, false));
		m_col_from.setText("From");

		m_col_level = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_level, new ColumnPixelData(80, false));
		m_col_level.setText("Level");

		m_col_message = new TableColumn(table, SWT.NONE);
		tcl_composite.setColumnData(m_col_message, new ColumnPixelData(150, true));
		m_col_message.setText("Message");

		return composite;
	}

	public void add(final LogMessage log) {
		if (log.getLevel().getLevel() < m_loglevel.getLevel())
			return;

		table.getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				try {
					int index = table.getTopIndex();
					if (index > 0) index++;
					if (index > 100) index = 100;
					TableItem item = new TableItem(table, SWT.NULL, 0);
					item.setText(0, log.getDateText());
					item.setText(1, log.getFrom());
					item.setText(2, log.getLevel().getLabel());
					item.setText(3, log.getMessage());
					item.setBackground(2, JavaGraphicDrawer.convert(log.getLevel().getFontColor()));
					item.setForeground(2, JavaGraphicDrawer.convert(HtmlColor.WHITE));
					table.setTopIndex(index);
					TableItem[] items = table.getItems();
					if (items.length > 100) {
						table.remove(100);
					}
				} catch (Exception e) {

				}
			}
		});
	}

	public void setPriority(String spri) {
		for (LogLevel level : LogLevel.LIST) {
			if (spri.equals(level.getLabel())) {
				m_loglevel = level;
				return;
			}
		}
	}

}

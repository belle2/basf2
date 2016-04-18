package org.csstudio.belle2.daq.dqm;

import org.csstudio.ui.util.swt.stringtable.RowEditDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class DQMConnectionEditDialog extends RowEditDialog {

	private Text hostText, portText;
		
	public DQMConnectionEditDialog(Shell parentShell) {
		super(parentShell);
		
	}
	
	@Override
	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		newShell.setText("Edit DQM Connection");
	}
	
	@Override
	protected Control createDialogArea(Composite parent) {
        final Composite parent_composite = (Composite) super.createDialogArea(parent);
        final Composite composite = new Composite(parent_composite, SWT.NONE);
		composite.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		composite.setLayout(new GridLayout(4, false));
		GridData gd;
		
		final Label hostLable = new Label(composite, 0);
		hostLable.setText("DQM Host");
		hostLable.setLayoutData(new GridData());
		
		hostText = new Text(composite, SWT.BORDER | SWT.SINGLE);
		
		hostText.setText(rowData[0]);
		gd = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd.widthHint = 300;
		hostText.setLayoutData(gd);		
		
		final Label portLable = new Label(composite, SWT.NONE);
		portLable.setText("DQM port");
		portLable.setLayoutData(new GridData(SWT.BEGINNING, SWT.BEGINNING, false, false));
		
		portText = new Text(composite, SWT.BORDER | SWT.SINGLE | SWT.V_SCROLL | SWT.H_SCROLL);
		gd = new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1);
		gd.widthHint = 300;
		portText.setLayoutData(gd);
		portText.setText(rowData[1]);
		
		return parent_composite;
	}

	@Override
	protected void okPressed() {
		rowData[0] = hostText == null ? "" : hostText.getText();
		rowData[1] = portText == null ? "" : portText.getText();
		super.okPressed();
	}	

}

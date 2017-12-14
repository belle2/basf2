package org.csstudio.belle2.daq.nsm;

import org.csstudio.ui.util.swt.stringtable.RowEditDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

public class NSMConnectionEditDialog extends RowEditDialog {

	private Text hostText, portText;
	private Text nsmhostText, nsmportText;
		
	public NSMConnectionEditDialog(Shell parentShell) {
		super(parentShell);
		
	}
	
	@Override
	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		newShell.setText("Edit NSM Connection");
	}
	
	@Override
	protected Control createDialogArea(Composite parent) {
        final Composite parent_composite = (Composite) super.createDialogArea(parent);
        final Composite composite = new Composite(parent_composite, SWT.NONE);
		composite.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		composite.setLayout(new GridLayout(4, false));
		GridData gd;
		
		final Label hostLable = new Label(composite, 0);
		hostLable.setText("Socket Host");
		hostLable.setLayoutData(new GridData());
		
		hostText = new Text(composite, SWT.BORDER | SWT.SINGLE);
		
		hostText.setText(rowData[0]);
		gd = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd.widthHint = 300;
		hostText.setLayoutData(gd);		
		
		final Label portLable = new Label(composite, SWT.NONE);
		portLable.setText("Socket port");
		portLable.setLayoutData(new GridData(SWT.BEGINNING, SWT.BEGINNING, false, false));
		
		portText = new Text(composite, SWT.BORDER | SWT.SINGLE | SWT.V_SCROLL | SWT.H_SCROLL);
		gd = new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1);
		gd.widthHint = 300;
		portText.setLayoutData(gd);
		portText.setText(rowData[1]);
		
		final Label nsmhostLable = new Label(composite, 0);
		nsmhostLable.setText("NSM Host");
		nsmhostLable.setLayoutData(new GridData());
		
		nsmhostText = new Text(composite, SWT.BORDER | SWT.SINGLE);
		
		nsmhostText.setText(rowData[2]);
		gd = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
		gd.widthHint = 300;
		nsmhostText.setLayoutData(gd);		
		
		final Label nsmportLable = new Label(composite, SWT.NONE);
		nsmportLable.setText("NSM port");
		nsmportLable.setLayoutData(new GridData(SWT.BEGINNING, SWT.BEGINNING, false, false));
		
		nsmportText = new Text(composite, SWT.BORDER | SWT.SINGLE | SWT.V_SCROLL | SWT.H_SCROLL);
		gd = new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1);
		gd.widthHint = 300;
		nsmportText.setLayoutData(gd);
		nsmportText.setText(rowData[3]);

		return parent_composite;
	}

	@Override
	protected void okPressed() {
		rowData[0] = hostText == null ? "" : hostText.getText();
		rowData[1] = portText == null ? "" : portText.getText();
		rowData[2] = nsmhostText == null ? "" : nsmhostText.getText();
		rowData[3] = nsmportText == null ? "" : nsmportText.getText();
		super.okPressed();
	}	

}

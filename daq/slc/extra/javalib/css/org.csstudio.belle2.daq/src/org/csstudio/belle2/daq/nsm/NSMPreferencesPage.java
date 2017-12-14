package org.csstudio.belle2.daq.nsm;

import org.csstudio.belle2.daq.nsm.PreferenceConstants;
import org.csstudio.opibuilder.preferences.StringTableFieldEditor;
import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.jface.dialogs.IMessageProvider;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.eclipse.ui.preferences.ScopedPreferenceStore;

public class NSMPreferencesPage extends FieldEditorPreferencePage implements IWorkbenchPreferencePage {

	private StringTableFieldEditor connectionEditor;

	public NSMPreferencesPage() {
		super(FieldEditorPreferencePage.GRID);

		final IPreferenceStore pref_store = new ScopedPreferenceStore(InstanceScope.INSTANCE, NSMDataSource.PLUGIN_ID);
		setPreferenceStore(pref_store);
	}

	@Override
	public void init(IWorkbench workbench) {

	}

	@Override
	protected void createFieldEditors() {
		final Composite parent = getFieldEditorParent();
		connectionEditor = new StringTableFieldEditor(PreferenceConstants.NSM, "NSM2Socket: ", parent, new String[] {
				"SocketHost", "SocketPort", "NSMHost", "NSMPort" }, new boolean[] { true, true, true, true }, 
				new NSMConnectionEditDialog(parent.getShell()), new int[] { 120, 80, 120, 80 }) {

			@Override
			public boolean isValid() {
				for (String[] row : items) {
					if (row[0] == null || row[0].length() == 0) {
						setMessage("Socket Host is empty", IMessageProvider.ERROR);
						return false;
					}
					int port = 0;
					try {
						port = Integer.parseInt(row[1]);
					} catch (Exception e) {
					}
					if (port <= 0) {
							setMessage("Socket Port is invalid", IMessageProvider.ERROR);
							return false;
					}
					if (row[2] == null || row[2].length() == 0) {
						setMessage("NSM Host is empty", IMessageProvider.ERROR);
						return false;
					}
					try {
						port = Integer.parseInt(row[3]);
					} catch (Exception e) {
					}
					if (port <= 0) {
							setMessage("NSM Port is invalid", IMessageProvider.ERROR);
							return false;
					}
				}
				setMessage("Restart CSS after pushing OK", IMessageProvider.INFORMATION);
				return true;
			}

			@Override
			protected void doStore() {
				if (!isValid())
					return;
				super.doStore();
			}

			@Override
			protected void doFillIntoGrid(Composite parent, int numColumns) {
				super.doFillIntoGrid(parent, numColumns);
				tableEditor.getTableViewer().getTable().addKeyListener(new KeyAdapter() {
					@Override
					public void keyReleased(KeyEvent e) {
						boolean valid = isValid();
						fireStateChanged(IS_VALID, !valid, valid);
					}
				});
				tableEditor.getTableViewer().getTable().addFocusListener(new FocusListener() {

					public void focusLost(FocusEvent e) {
						boolean valid = isValid();
						fireStateChanged(IS_VALID, !valid, valid);
					}

					public void focusGained(FocusEvent e) {
						boolean valid = isValid();
						fireStateChanged(IS_VALID, !valid, valid);
					}
				});
			}

		};
		addField(connectionEditor);
	}

	@Override
	public boolean performOk() {
		NSMDataSource.startNSM();
		return super.performOk();
	}

	@Override
	public final void propertyChange(final PropertyChangeEvent event) {
		//setMessage("Please restart CSS to activate any changes", IMessageProvider.INFORMATION);
		super.propertyChange(event);
	}
}

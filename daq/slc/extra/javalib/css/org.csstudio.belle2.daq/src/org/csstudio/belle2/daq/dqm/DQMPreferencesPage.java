package org.csstudio.belle2.daq.dqm;

import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.jface.dialogs.IMessageProvider;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.eclipse.ui.preferences.ScopedPreferenceStore;

public class DQMPreferencesPage extends FieldEditorPreferencePage implements
		IWorkbenchPreferencePage {

	public DQMPreferencesPage() {
		super(GRID);

		final IPreferenceStore pref_store = new ScopedPreferenceStore(
				InstanceScope.INSTANCE, DQMViewDataSource.PLUGIN_ID);
		setPreferenceStore(pref_store);
	}

	@Override
	public void init(IWorkbench workbench) {
		
	}

	@Override
	protected void createFieldEditors() {
		final Composite parent = getFieldEditorParent();
		addField(new StringFieldEditor(PreferenceConstants.HOST, "Hostname", parent));
		addField(new StringFieldEditor(PreferenceConstants.PORT, "Port", parent));
	}

	@Override
	public boolean performOk() {
		return super.performOk();
	}

	@Override
	public final void propertyChange(final PropertyChangeEvent event) {
		setMessage("Please restart CSS to activate any changes", IMessageProvider.INFORMATION);
		super.propertyChange(event);
	}
}

package b2daq.java.ui;

import java.util.ArrayList;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import b2daq.ui.Updatable;


public class UTabPanel extends DnDTabbedPane implements Updatable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private ArrayList<Updatable> _updatable_v = new ArrayList<Updatable>(); 
	
	public UTabPanel() {
		this("");
	}
	
	public UTabPanel(String title) {
		setName(title);
		addChangeListener(new ChangeListener(){
			public void stateChanged(ChangeEvent arg0) {
				update();
			}
		});
	}

	public void addChild(Updatable obj) {
		_updatable_v.add(obj);
	}

	public void update() {
		int index = this.getSelectedIndex();
		if ( index >= 0 && index < _updatable_v.size() ) {
			_updatable_v.get(index).update();
		}
	}

}

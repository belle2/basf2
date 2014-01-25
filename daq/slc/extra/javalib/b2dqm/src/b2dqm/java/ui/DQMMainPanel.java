package b2dqm.java.ui;

import java.awt.Component;
import java.util.ArrayList;

import javax.swing.JComponent;

import b2daq.java.ui.DnDTabbedPane;
import b2daq.java.ui.UTabPanel;
import b2daq.ui.Updatable;
import b2dqm.core.Histo;
import b2dqm.core.HistoPackage;

public class DQMMainPanel extends DnDTabbedPane implements Updatable {

	private static final long serialVersionUID = 1L;
	private ArrayList<HistoPackage> _pack_v = null;
	private DQMSidePanel _side_panel;

	public DQMMainPanel() {
		super();
		_pack_v = new ArrayList<HistoPackage>();
	}

	public ArrayList<HistoPackage> getPackages() {
		return _pack_v;
	}

	public void initPanels(ArrayList<HistoPackage> pack_v,
						   DQMSidePanel side_panel) {
		System.out.println("DQMMainPanel::initPanels");
		removeAll();
		_pack_v = pack_v;
		_side_panel = side_panel;
		_side_panel.setMainPanel(this);
		for ( int n = 0; n < _pack_v.size(); n++ ) {
			HistoPackage pack = _pack_v.get(n);
			UTabPanel tab = new UTabPanel();
			tab.setName(pack.getName());
			for ( int i = 0; i < pack.getNHistos(); i++ ) {
				Histo h = (Histo)pack.getHisto(i);
				CanvasPanel canvas = new CanvasPanel(h.getName(), h.getTitle());
				canvas.getCanvas().addHisto(h);
				canvas.getCanvas().resetPadding();
				tab.addTab(h.getName(), canvas);
				System.out.println("DQMMainPanel::initPanels " + h.getName());
			}
			addTab(pack.getName(), tab);
		}
		System.out.println("DQMMainPanel::initPanels done");
	}

	public void update() {
		this.repaint();
	}

	static protected boolean lookfor(Component com, String name) {
		if ( com instanceof CanvasPanel == true ) {
			CanvasPanel canvas = (CanvasPanel)com;
			for ( int i = 0; i < canvas.getNHistos(); i++ ) {
				if ( canvas.getMonObject(i).get() != null &&
					 name.matches(canvas.getMonObject(i).get().getName()) ) {
					return true;
				}
			}
		} else if ( com instanceof JComponent == true ){
			JComponent jcom = (JComponent)com;
			Component[] com_v = jcom.getComponents();
			for ( int i = 0; i < com_v.length; i++ ) {
				if ( lookfor(com_v[i], name) ) {
					if ( com instanceof UTabPanel == true ) {
						((UTabPanel)com).setSelectedComponent(com_v[i]);
					}
					return true;
				}
			}
		}
		return false;
	}

	public Component getPanel(String name) {
		for ( int n = 0; n < this.getComponentCount(); n++ ) {
			Component com = getComponentAt(n);
			if ( com.getName() != null && name.matches(com.getName()) ) {
				return com;
			}
		}
		return null;
	}

}

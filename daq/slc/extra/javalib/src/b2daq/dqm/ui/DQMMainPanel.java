package b2daq.dqm.ui;

import java.awt.Component;
import java.util.ArrayList;

import javax.swing.JComponent;

//import b2daq.java.io.ConfigFile;
import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.ui.DnDTabbedPane;
import b2daq.ui.UTabPanel;
import b2daq.ui.Updatable;

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
		removeAll();
		_pack_v = pack_v;
		_side_panel = side_panel;
		_side_panel.setMainPanel(this);
		//ConfigFile config = new ConfigFile("/home/tkonno/test.conf");
		for ( int n = 0; n < _pack_v.size(); n++ ) {
			HistoPackage pack = _pack_v.get(n);
			UTabPanel tab = new UTabPanel();
			tab.setName(pack.getName());
			ArrayList<Histo> histo_v = new ArrayList<Histo>();
			//GHistoProperty pro = new GHistoProperty(config);
			for ( int i = 0; i < pack.getNHistos(); i++ ) {
				Histo h = (Histo)pack.getHisto(i);
				CanvasPanel canvas = new CanvasPanel(h.getName(), h.getTitle());
				//GHisto gh = (GHisto) 
				canvas.getCanvas().addHisto(h);
				//pro.put(gh, h.getName());
				canvas.getCanvas().resetPadding();
				UTabPanel subtab = new UTabPanel();
				subtab.addTab("Total", canvas);
				tab.addTab(h.getName(), subtab);
				Histo h1 = (Histo)h.clone();
				h1.setName(h.getName()+":diff");
				canvas = new CanvasPanel(h1.getName(), h1.getTitle());
				//gh = (GHisto)
				canvas.getCanvas().addHisto(h1);
				//pro.put(gh, h.getName());
				canvas.getCanvas().resetPadding();
				subtab.addTab("Difference", canvas);
				histo_v.add(h1);
				h1 = (Histo)h.clone();
				h1.setName(h.getName()+":tmp");
				histo_v.add(h1);
			}
			for (Histo h: histo_v) {
				pack.addHisto(h);
			}
			addTab(pack.getName(), tab);
		}
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

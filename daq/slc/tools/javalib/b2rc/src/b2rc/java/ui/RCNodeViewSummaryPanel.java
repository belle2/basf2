package b2rc.java.ui;

import java.util.ArrayList;

import b2daq.java.ui.DnDTabbedPane;
import b2daq.ui.Updatable;
import b2rc.core.RCNode;
import b2rc.core.RCNodeGroup;
import b2rc.core.RCState;

public class RCNodeViewSummaryPanel extends DnDTabbedPane implements Updatable {

	private static final long serialVersionUID = 1L;
	private ArrayList<RCNodeGroup> _group_v;
	private ArrayList<RCNodeGroupViewPanel> _panel_v = new ArrayList<RCNodeGroupViewPanel>();

	public RCNodeViewSummaryPanel(ArrayList<RCNodeGroup> group_v) {
		_group_v = group_v;
	}
	
	public ArrayList<RCNodeGroupViewPanel> getpanels() {
		return _panel_v;
	}

	public void init() {
		for (RCNodeGroup group : _group_v) {
			for (RCNode node : group.getNodes()) {
				node.setState(RCState.READY_S);
			}
			RCNodeGroupViewPanel panel = new RCNodeGroupViewPanel(group);
			panel.init();
			addTab(group.getLabel(), panel);
			_panel_v.add(panel);
		}
	}

	public void update() {
		for (RCNodeGroupViewPanel panel : _panel_v) {
			panel.update();
		}
	}

}

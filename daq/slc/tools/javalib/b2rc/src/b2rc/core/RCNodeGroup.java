package b2rc.core;

import java.util.ArrayList;

public class RCNodeGroup {

	private ArrayList<RCNode> _node_v = new ArrayList<RCNode>();
	private int _cols = 1;
	private int _rows = 1;
	private String _label = "";
	
	public RCNodeGroup(String label) {
		_label = label;
	}
	
	public RCNodeGroup(String label, int cols, int rows) {
		_label = label;
		_cols = cols;
		_rows = rows;
	}
	
	public RCNodeGroup() {
	}

	public ArrayList<RCNode> getNodes() {
		return _node_v;
	}

	public void addNode(RCNode node) {
		_node_v.add(node);
	}

	public int getCols() {
		return _cols;
	}

	public void setCols(int cols) {
		_cols = cols;
	}

	public int getRows() {
		return _rows;
	}

	public void setRows(int rows) {
		_rows = rows;
	}

	public String getLabel() {
		return _label;
	}

	public void setLabel(String label) {
		_label = label;
	}

}

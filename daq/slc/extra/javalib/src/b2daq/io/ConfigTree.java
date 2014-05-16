package b2daq.io;

import java.util.ArrayList;

public class ConfigTree {

	public class Leaf {
		protected String _name = "";
		private String _label = "";
		public boolean isLeaf() {
			return true;
		}
		public Leaf(String name, String label) {
			_name = name;
			_label = label;
		}
		public String getName() {
			return _name;
		}
		public void push(ArrayList<Leaf> leafs) {
		}
		public String getLabel() {
			return _label;
		}
	}

	public class Branch extends Leaf {
		private ArrayList <Leaf> _leaf_v = new ArrayList<Leaf>();
		public Branch(String name) {
			super(name, "");
		}
		public boolean isLeaf() {
			return false;
		}
		public Leaf addLeaf(Leaf leaf) {
			for (Leaf l : _leaf_v) {
				if (l.getName().matches(leaf.getName())) {
					if ((!l.isLeaf()) && (!leaf.isLeaf())) {
						l.push(((Branch)leaf).getLeafs());
					}
					return l;
				}
			}
			_leaf_v.add(leaf);
			return leaf;
		}
		public ArrayList<Leaf> getLeafs() {
			return _leaf_v;
		}
		public void push(ArrayList<Leaf> leafs) {
			for (Leaf l : leafs) {
				addLeaf(l);
			}
		}
	}
}

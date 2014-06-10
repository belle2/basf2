package b2daq.io;

import java.util.ArrayList;

public class ConfigTree {

    public Branch createBranch(String... namelist) {
        Branch branch = new Branch();
        for (String type : namelist) {
            String[] str_v = type.split(":");
            Branch branch_ch = new Branch(str_v[0]);
            Branch branch_tmp = branch_ch;
            for (int n = 1; n < str_v.length - 1; n++) {
                branch_tmp = (Branch) branch_tmp.addLeaf(new Branch(str_v[n]));
            }
            branch_tmp.addLeaf(new Leaf(str_v[str_v.length - 1], type));
            branch.addLeaf(branch_ch);
        }
        return branch;
    }

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

        private ArrayList<Leaf> _leaf_v = new ArrayList<Leaf>();

        public Branch(String name) {
            super(name, "");
        }

        public Branch() {
            this("");
        }

        public boolean isLeaf() {
            return false;
        }

        public Leaf addLeaf(Leaf leaf) {
            for (Leaf l : _leaf_v) {
                if (l.getName().matches(leaf.getName())) {
                    if ((!l.isLeaf()) && (!leaf.isLeaf())) {
                        l.push(((Branch) leaf).getLeafs());
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

        public void setName(String name) {
            _name = name;
        }
    }
}

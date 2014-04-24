package b2rc.java.io;

import java.util.ArrayList;

public class RunTypeConfigTree extends ConfigTree {

	public Branch createBranch(ArrayList<String> set) {
		Branch branch = new Branch("Run type");
		for (String type : set) {
			String [] str_v = type.split(":");
			Branch branch_ch = new Branch(str_v[0]);
			Branch branch_tmp = branch_ch;
			for (int n = 1; n < str_v.length -1; n++) {
				branch_tmp = (Branch)branch_tmp.addLeaf(new Branch(str_v[n]));
			}
			branch_tmp.addLeaf(new Leaf(str_v[str_v.length-1], type));
			branch.addLeaf(branch_ch);
		}
		return branch;
	}

}
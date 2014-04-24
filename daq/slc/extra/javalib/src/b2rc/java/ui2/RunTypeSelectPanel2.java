package b2rc.java.ui2;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;

import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import javax.swing.tree.DefaultMutableTreeNode;

import b2rc.java.io.ConfigTree;
import b2rc.java.io.RunTypeConfigTree;

public class RunTypeSelectPanel2 extends JPanel {

	private static final long serialVersionUID = 1L;

	private String _selected_runtype = "";
	private boolean _edit_new = false;

	protected class LabeledTreeNode extends DefaultMutableTreeNode {

		private static final long serialVersionUID = 1L;
		private String _label = "";

		public LabeledTreeNode(String title, String label) {
			super(title);
			_label = label;
		}

		public String getLabel() {
			return _label;
		}

	}

	public RunTypeSelectPanel2() {
	}

	public void init(ConfigTree.Branch branch) {
		DefaultMutableTreeNode root = createNode(branch);
		JTree tree = new JTree(root);
		tree.setFont(new Font("Sans", Font.PLAIN, 13));
		tree.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseReleased(MouseEvent arg0) {
				JTree tree = (JTree) arg0.getSource();
				try {
					LabeledTreeNode node = (LabeledTreeNode) 
					tree.getLastSelectedPathComponent();
					_selected_runtype = node.getLabel();
				} catch (Exception e) {
				}
				tree.repaint();
			}
		});
		JScrollPane scrollPane = new JScrollPane();
		scrollPane.getViewport().setView(tree);
		scrollPane.setPreferredSize(new Dimension(220, 200));
		add(scrollPane);
	}

	public DefaultMutableTreeNode createNode(ConfigTree.Branch branch) {
		DefaultMutableTreeNode root = new DefaultMutableTreeNode(
				branch.getName());
		for (ConfigTree.Leaf leaf : branch.getLeafs()) {
			if (leaf.isLeaf()) {
				LabeledTreeNode node = new LabeledTreeNode(leaf.getName(),
						leaf.getLabel());
				root.add(node);
			} else {
				root.add(createNode((ConfigTree.Branch) leaf));
			}
		}
		return root;
	}

	public String getSelected() {
		return _selected_runtype;
	}

	public String showSelectPanel(ArrayList<String> runtype_v) {
		RunTypeConfigTree tree = new RunTypeConfigTree();
		init(tree.createBranch(runtype_v));
		Object[] message = { this };
		String[] options = { "Set", "Edit", "Cancel" };
		UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans", Font.PLAIN, 13)));
		int result = JOptionPane.showOptionDialog(null, message,
				"Select RunType", JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
		return "" + result;
	}

	public boolean isEdited() {
		return _edit_new;
	}

}

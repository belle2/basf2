package b2rc.java.ui2;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.net.Socket;
import java.util.ArrayList;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import javax.swing.tree.DefaultMutableTreeNode;

import b2daq.java.io.ConfigFile;
import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.ui.CheckBoxPanel;
import b2daq.java.ui.ComboBoxPanel;
import b2daq.java.ui.FloatBoxPanel;
import b2daq.java.ui.IntegerBoxPanel;
import b2daq.java.ui.TextBoxPanel;
import b2rc.java.io.ConfigTree;
import b2rc.java.io.RunTypeConfigTree;

public class RunTypeSelectPanel extends JPanel {

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

	public RunTypeSelectPanel() {
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
					LabeledTreeNode node = (LabeledTreeNode) (tree
							.getLastSelectedPathComponent());
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

	public String showSelectPanel(ConfigFile file) {
		RunTypeConfigTree tree = new RunTypeConfigTree();
		init(tree.createBranch(file.getDirectories()));
		Object[] message = { this };
		String[] options = { "Set", "Edit", "Cancel" };
		UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans", Font.PLAIN, 13)));
		int result = JOptionPane.showOptionDialog(null, message,
				"Select RunType", JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
		if (result > 1)
			return "";
		String runtype = getSelected();
		_edit_new = false;
		if (result == 1) {
			_edit_new = true;
			file.cd();
			JPanel runtype_panel = new JPanel();
			runtype_panel.setLayout(new BoxLayout(runtype_panel,BoxLayout.Y_AXIS));
			JPanel title_panel = new JPanel();
			JLabel title_label = new JLabel(runtype + " setting");
			title_label.setHorizontalAlignment(JLabel.CENTER);
			title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
			title_panel.add(title_label);
			runtype_panel.add(title_panel);
			runtype_panel.add(Box.createRigidArea(new Dimension(0, 20)));
			runtype_panel.add(new TextBoxPanel("runtype", runtype + "_2"));
			runtype_panel.add(Box.createRigidArea(new Dimension(0, 20)));
			for (String label : file.getKeyList()) {
				if (label.startsWith("default.")) {
					String value = file.getString(label);
					label = label.replace("default.", "");
					System.out.println(label + " " + value);
					if (value.startsWith("boolean(")) {
						runtype_panel.add(new CheckBoxPanel(label, value.contains("true")));
					} else if (value.startsWith("enum(")) {
						value = value.replace("enum(", "").replace(")", "");
						String[] value_v = value.split(",");
						runtype_panel.add(new ComboBoxPanel(label, value_v));
					} else if (label.endsWith(".runtype")) {
						String label_include = label.split("\\.")[0]+ ".include";
						String path = "/home/tkonno/rcconfig/" + file.getString(label_include);
						ConfigFile newfile = new ConfigFile(path);
						runtype_panel.add(new SelectButtonBoxPanel(label, value, newfile));
					} else if (value.startsWith("int(")) {
						value = value.replace("int(", "").replace(")", "");
						runtype_panel.add(new IntegerBoxPanel(label, value));
					} else if (value.startsWith("float(")) {
						value = value.replace("float(", "").replace(")", "");
						runtype_panel.add(new FloatBoxPanel(label, value));
					} else {
						runtype_panel.add(new TextBoxPanel(label, value));
					}
					runtype_panel.add(Box.createRigidArea(new Dimension(0, 5)));
				}
			}
			Object[] message2 = { runtype_panel };
			String[] options2 = { "OK", "Cancel" };
			UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans", Font.PLAIN, 13)));
			result = JOptionPane.showOptionDialog(null, message2,
					"Edit RunType", JOptionPane.DEFAULT_OPTION,
					JOptionPane.PLAIN_MESSAGE, null, options2, options2[0]);
			if (result == 0) {
				String dbtable = file.getString("db.tablename");
				StringBuffer ss = new StringBuffer();
				StringBuffer ss_label = new StringBuffer();
				StringBuffer ss_value = new StringBuffer();
				ss.append("insert " + dbtable + " ");
				for (Component com : runtype_panel.getComponents()) {
					if (com instanceof TextBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						TextBoxPanel panel = (TextBoxPanel) com;
						if (panel.getLabel().matches("runtype")) {
							runtype = panel.getText();
						}
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append("'" + panel.getText() + "'");
					} else if (com instanceof IntegerBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						IntegerBoxPanel panel = (IntegerBoxPanel) com;
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append(panel.getText());
					} else if (com instanceof FloatBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						FloatBoxPanel panel = (FloatBoxPanel) com;
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append(panel.getText());
					} else if (com instanceof ComboBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						ComboBoxPanel panel = (ComboBoxPanel) com;
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append("'" + panel.getText() + "'");
					} else if (com instanceof CheckBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						CheckBoxPanel panel = (CheckBoxPanel) com;
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append((panel.isSelected() ? "true" : "false"));
					} else if (com instanceof SelectButtonBoxPanel) {
						if (ss_label.length() > 0) ss_label.append(",");
						if (ss_value.length() > 0) ss_value.append(",");
						SelectButtonBoxPanel panel = (SelectButtonBoxPanel) com;
						ss_label.append("\"" + panel.getLabel() + "\"");
						ss_value.append("'" + panel.getText() + "'");
					}
				}
				ss.append("(" + ss_label.toString() + ") values (" + ss_value.toString() + ")");
				System.out.println(ss.toString());
			} else {
				return "";
			}
		}
		return runtype;
	}

	public String showSelectPanel(String nodename, Socket socket) {
		try {
			RunTypeConfigTree tree = new RunTypeConfigTree();
			ArrayList<String> runtype_v = new ArrayList<String>();
			SocketDataWriter writer = new SocketDataWriter(socket);
			writer.writeString(nodename);
			SocketDataReader reader = new SocketDataReader(socket);
			int nruntypes = reader.readInt();
			for (int n = 0; n < nruntypes; n++) {
				runtype_v.add(reader.readString());
			}
			init(tree.createBranch(runtype_v));
			Object[] message = { this };
			String[] options = { "Set", "Edit", "Cancel" };
			UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font(
					"Sans", Font.PLAIN, 13)));
			int result = JOptionPane.showOptionDialog(null, message,
					"Select RunType", JOptionPane.DEFAULT_OPTION,
					JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
			if (result > 1)
				return "";
			String runtype = getSelected();
			_edit_new = false;
			if (result == 1) {
				writer.writeString(nodename + "." + runtype + ".read");
				_edit_new = true;
				JPanel runtype_panel = new JPanel();
				runtype_panel.setLayout(new BoxLayout(runtype_panel, BoxLayout.Y_AXIS));
				JPanel title_panel = new JPanel();
				JLabel title_label = new JLabel(runtype + " setting");
				title_label.setHorizontalAlignment(JLabel.CENTER);
				title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
				title_panel.add(title_label);
				runtype_panel.add(title_panel);
				runtype_panel.add(Box.createRigidArea(new Dimension(0, 20)));
				runtype_panel.add(new TextBoxPanel("runtype", runtype + "_2"));
				runtype_panel.add(Box.createRigidArea(new Dimension(0, 20)));
				RuntypeInfo info = new RuntypeInfo(runtype);
				info.readObject(reader);
				for (String parname : info.getParamNames()) {
					String attname = info.getAttName(parname);
					String value = info.getValue(parname);
					System.out.println(parname + " " + value);
					if (attname.contains("boolean")) {
						runtype_panel.add(new CheckBoxPanel(parname, value.contains("t")));
					} else if (attname.contains("enum")) {
						String[] value_v = value.split(",");
						runtype_panel.add(new ComboBoxPanel(parname, value_v));
					} else if (parname.endsWith(".runtype")) {
						runtype_panel.add(new SelectButtonBoxPanel(parname, value, socket));
					} else if (attname.contains("int")) {
						value = value.replace("int(", "").replace(")", "");
						runtype_panel.add(new IntegerBoxPanel(parname, value));
					} else if (attname.contains("float")) {
						value = value.replace("float(", "").replace(")", "");
						runtype_panel.add(new FloatBoxPanel(parname, value));
					} else {
						runtype_panel.add(new TextBoxPanel(parname, value));
					}
					runtype_panel.add(Box.createRigidArea(new Dimension(0, 5)));
				}
				Object[] message2 = { runtype_panel };
				String[] options2 = { "OK", "Cancel" };
				UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans", Font.PLAIN, 13)));
				result = JOptionPane.showOptionDialog(null, message2, "Edit RunType", 
						JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null, options2, options2[0]);
				if (result == 0) {
					writer.writeString(nodename + "." + runtype + ".write");
					for (Component com : runtype_panel.getComponents()) {
						if (com instanceof TextBoxPanel) {
							TextBoxPanel panel = (TextBoxPanel) com;
							if (panel.getLabel().matches("runtype")) {
								info.setRuntype(panel.getText());
							} else {
								info.setValue(panel.getLabel(), panel.getText());
							}
						} else if (com instanceof IntegerBoxPanel) {
							IntegerBoxPanel panel = (IntegerBoxPanel) com;
							info.setValue(panel.getLabel(), panel.getText());
						} else if (com instanceof FloatBoxPanel) {
							FloatBoxPanel panel = (FloatBoxPanel) com;
							info.setValue(panel.getLabel(), panel.getText());
						} else if (com instanceof ComboBoxPanel) {
							ComboBoxPanel panel = (ComboBoxPanel) com;
							info.setValue(panel.getLabel(), panel.getText());
						} else if (com instanceof CheckBoxPanel) {
							CheckBoxPanel panel = (CheckBoxPanel) com;
							info.setValue(panel.getLabel(),
									(panel.isSelected() ? "true" : "false"));
						} else if (com instanceof SelectButtonBoxPanel) {
							SelectButtonBoxPanel panel = (SelectButtonBoxPanel) com;
							info.setValue(panel.getLabel(), panel.getText());
						}
					}
					writer.writeObject(info);
				} else {
					return "";
				}
			}
			return runtype;
		} catch (Exception e) {
			e.printStackTrace();
			return "";
		}
	}

	public boolean isEdited() {
		return _edit_new;
	}

}

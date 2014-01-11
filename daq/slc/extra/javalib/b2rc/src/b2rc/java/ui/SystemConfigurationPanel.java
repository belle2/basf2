package b2rc.java.ui;

import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.sql.ResultSet;
import java.util.ArrayList;
import java.util.HashMap;

import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.plaf.FontUIResource;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.ui.Updatable;
import b2rc.core.RCNode;
import b2rc.core.RCMaster;
import b2rc.core.RCState;
import b2rc.java.io.RunTypeConfigTree;

public class SystemConfigurationPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private static final String BR = System.getProperty("line.separator");

	private JTextField _text_operator1;
	private JTextField _text_operator2;
	private JButton _button_run_type;
	private RCMaster _master;
	private ControlMainPanel _main_panel;
	private JLabel _label_run_type;
	private JComboBox _combo_version;
	private JTextArea _label_description_contents;
	private HashMap<String, ArrayList<Integer>> _run_type_v_m;
	private int _version_old = 0;
	private int _version_max = -1;
	
	private void setGrid(GridBagConstraints gbc, int gridwidth, int gridheight,
			int gridx, int gridy, double weightx, double weighty, int fill,
			int anchor, Insets insets) {
		gbc.gridwidth = gridwidth;
		gbc.gridheight = gridheight;
		gbc.gridx = gridx;
		gbc.gridy = gridy;
		gbc.weightx = weightx;
		gbc.weighty = weighty;
		gbc.fill = fill;
		// gbc.anchor = GridBagConstraints.LINE_START;
		gbc.anchor = anchor;
		gbc.insets = insets;
	}

	private void setGrid(GridBagConstraints gbc, int gridx, int gridy,
			double weightx, double weighty, int fill, int anchor, Insets insets) {
		setGrid(gbc, 1, 1, gridx, gridy, weightx, weighty, fill, anchor, insets);
	}

	public SystemConfigurationPanel(ControlMainPanel main_panel, 
			RCMaster master) {
		_main_panel = main_panel;
		_master = master;
		setBorder(new TitledBorder(new EtchedBorder(), "System configuration",
				TitledBorder.LEFT, TitledBorder.TOP));
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		/*
		setGrid(gbc, 0, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.WEST, new Insets(0, 5, 0, 5));
		JLabel label_operation_mode = new JLabel("Operation mode:");
		label_operation_mode.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_operation_mode, gbc);
		add(label_operation_mode);
		 */
		setGrid(gbc, 0, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_operators = new JLabel("Operators:");
		label_operators.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_operators, gbc);
		add(label_operators);

		setGrid(gbc, 1, 0, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_operator1 = new JTextField();
		_text_operator1.setPreferredSize(new Dimension(180, 30));
		_text_operator1.addKeyListener(new KeyListener() {
			public void keyPressed(KeyEvent arg0) {
			}

			public void keyReleased(KeyEvent arg0) {
				_master.getStatus().setOperators(_text_operator1.getText() + ":"
						+ _text_operator2.getText());
				update();
			}

			public void keyTyped(KeyEvent arg0) {
			}
		});
		layout.setConstraints(_text_operator1, gbc);
		add(_text_operator1);

		setGrid(gbc, 1, 1, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_operator2 = new JTextField();
		_text_operator2.setPreferredSize(new Dimension(180, 30));
		_text_operator2.addKeyListener(new KeyListener() {
			public void keyPressed(KeyEvent arg0) {
			}

			public void keyReleased(KeyEvent arg0) {
				_master.getStatus().setOperators(_text_operator1.getText() + ":"
						+ _text_operator2.getText());
				update();
			}

			public void keyTyped(KeyEvent arg0) {
			}
		});
		layout.setConstraints(_text_operator2, gbc);
		add(_text_operator2);

		setGrid(gbc, 0, 2, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_button_run_type = new JButton("Run type");
		_button_run_type.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_run_type.setPreferredSize(new Dimension(90, 30));
		_button_run_type.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				RunTypeConfigTree tree = new RunTypeConfigTree();
				try {
					_run_type_v_m = new HashMap<String, ArrayList<Integer>>();
					/*
					ResultSet result_v = RCDBManager.get().executeQuery("select version, run_type from version_control;");
					while ( result_v.next() ) {
						String run_type = result_v.getString("run_type");
						if ( !_run_type_v_m.containsKey(run_type) ) {
							_run_type_v_m.put(run_type, new ArrayList<Integer>());
						} 
						_run_type_v_m.get(run_type).add(result_v.getInt("version"));
					}
					*/
				} catch (Exception e) {
					e.printStackTrace();
				}
				RunTypeSelectPanel panel = new RunTypeSelectPanel(tree.createBranch(_run_type_v_m.keySet()));
				Object[] message = { panel };
				String[] options = { "Set", "Cancel" };
				UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans",Font.PLAIN,13)));
				int result = JOptionPane.showOptionDialog(_main_panel, message,
						"Select RunType", JOptionPane.DEFAULT_OPTION,
						JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
				switch (result) {
				case 0:
					final String run_type = panel.getSelected();
					if ( run_type.length() == 0 ) {
						_main_panel.addLog(new Log("Failed to select run type", LogLevel.WARNING));
						break;
					} 
					_master.getConfig().setRunType(run_type);
					_label_run_type.setText(run_type);
					_main_panel.addLog(new Log("Set run type: " + "<span style='color:blue;font-weight:bold;'>"
							+ run_type + "</span>", LogLevel.INFO));
					_main_panel.update();
					//_editor_panel.update();
					_combo_version.removeAllItems();
					_combo_version.addItem("latest");
					_version_max = -1;
					for ( Integer version : _run_type_v_m.get(run_type) ) {
						if (_version_max < version ) _version_max = version;
						_combo_version.addItem(""+version);
					}
					_version_old = -1;
					_combo_version.setSelectedIndex(0);
					break;
				case 1:
					break;
				default:
					break;
				}
			}
		});

		layout.setConstraints(_button_run_type, gbc);
		add(_button_run_type);

		setGrid(gbc, 1, 2, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_label_run_type = new JLabel("empty");
		_label_run_type.setFont(new Font("Sans", Font.BOLD, 13));
		layout.setConstraints(_label_run_type, gbc);
		add(_label_run_type);

		setGrid(gbc, 0, 3, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_version = new JLabel("Version:");
		label_version.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_version, gbc);
		add(label_version);

		setGrid(gbc, 1, 3, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		String[] combodata = { "latest", "0" };
		DefaultComboBoxModel model_version = new DefaultComboBoxModel(combodata);
		_combo_version = new JComboBox(model_version);
		_combo_version.setFont(new Font("Sans", Font.PLAIN, 13));
		_combo_version.setPreferredSize(new Dimension(120, 30));
		_combo_version.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if ( _combo_version.getSelectedIndex() < 0 ) return;
				if ( _combo_version.getSelectedIndex() == 0 ) {
					_master.getStatus().setRunConfig(_version_max);
				} else {
					_master.getStatus().setRunConfig(Integer.parseInt((String)_combo_version.getSelectedItem()));
				}
				if (_master.getStatus().getRunConfig() == _version_old ) return;
				_version_old = _master.getStatus().getRunConfig();
				if ( _version_old < 0 ) return;
				readDatabase(_version_old);
				_main_panel.addLog(new Log("Set run type version: "
						+ "<span style='color:blue;font-weight:bold;'>"
						+ _version_max + ((_version_old==_version_max)?" (latest)":"")
						+ "</span>", LogLevel.INFO));
				_main_panel.update();
			}
		});
		_combo_version.setEditable(false);
		layout.setConstraints(_combo_version, gbc);
		add(_combo_version);

		setGrid(gbc, 2, 1, 0, 5, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_description = new JLabel("Comment:");
		label_description.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_description, gbc);
		add(label_description);

		setGrid(gbc, 2, 1, 0, 6, 0.2d, 0.6d, GridBagConstraints.BOTH,
				GridBagConstraints.FIRST_LINE_START, new Insets(0, 10, 10, 10));
		_label_description_contents = new JTextArea("");
		_label_description_contents.setFont(new Font("Sans", Font.PLAIN, 13));
		JScrollPane scrollpanel = new JScrollPane();
		scrollpanel.setViewportView(_label_description_contents);
		scrollpanel.setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
		layout.setConstraints(scrollpanel, gbc);
		add(scrollpanel);
		setOperators(_master.getStatus().getOperators());
		setDescription(_master.getStatus().getComment());
		_label_description_contents.addKeyListener(new KeyListener() {
			public void keyPressed(KeyEvent arg0) {
			}

			public void keyReleased(KeyEvent arg0) {
				_master.getStatus().setComment(_label_description_contents.getText().replace(BR, "<br/>"));
				update();
			}

			public void keyTyped(KeyEvent arg0) {
			}
		});
	}

	private void setDescription(String description) {
		_label_description_contents.setText(description);
	}

	public void update() {
		RCNode node = _master.getNode();
		RCState state = node.getState();
		if ( state.equals(RCState.INITIAL_S) || 
			 state.equals(RCState.CONFIGURED_S) || 
			 state.equals(RCState.READY_S) ) {
			_text_operator1.setEditable(true);
			_text_operator2.setEditable(true);
			_button_run_type.setEnabled(_master.getStatus().getOperators().length()>0);
			_combo_version.setEnabled(_version_old >= 0);
			_label_description_contents.setEnabled(true);
		} else {
			_text_operator1.setEditable(false);
			_text_operator2.setEditable(false);
			_label_description_contents.setEnabled(false);
			_button_run_type.setEnabled(false);
			_combo_version.setEnabled(false);
		}
		_label_run_type.setText(_master.getConfig().getRunType());
		//setDescription(_master.getConfig().getDescription());
	}

	public JComboBox getComboVersion() {
		return _combo_version;
	}

	public void setOperators(String operators) {
		String [] str_v = operators.split(":");
		String operator1 =(str_v.length > 0)?str_v[0]:"";
		String operator2 =(str_v.length > 1)?str_v[1]:"";
		_text_operator1.setText(operator1);
		_text_operator2.setText(operator2);
	}

	public void setVerionCombo(int value) {
		_combo_version.removeAllItems();
		//_combo_version.addItem(""+_master.getVersion());
		_combo_version.setEditable(false);
	}

	public void readDatabase(int version){
		/*
		try {
			ResultSet result_v = RCDBManager.get().executeQuery("select description from version_control where version="+_master.getVersion()+";");
			if ( result_v.next() ) {
				String description = result_v.getString("description");
				_label_description_contents.setText(description);
			}
			result_v.close();
			RCDBManager.get().readTables(_version_old);
		} catch (Exception e) {
			e.printStackTrace();
		}
		*/
	}

}

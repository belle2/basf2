package b2rc.java.ui;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.SystemColor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.sql.ResultSet;
import java.util.ArrayList;
import java.util.HashMap;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.UIManager;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.plaf.FontUIResource;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;
import b2rc.java.io.RunTypeConfigTree;

public class VersionControlPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private RCNodeSystem _system;
	private ArrayList<ParameterTablePanel> _table_v;
	private JButton _upload_button;
	private JComboBox _combo_version = new JComboBox();
	private HashMap<String, JComboBox> _combo_v = new HashMap<String, JComboBox>();
	private HashMap<String, Boolean> _enabled_v = new HashMap<String, Boolean>();
	private int _version_old;
	private int _version_max;
	private JTextArea _entry_description = new JTextArea();
	private JButton _button_run_type;
	private EditorMainPanel _editor_panel;
	private JLabel _label_run_type;
	
	public VersionControlPanel(EditorMainPanel editor_panel, RCNodeSystem system,  
			ArrayList<ParameterTablePanel> table_v) {
		_editor_panel = editor_panel;
		_system = system;
		_table_v = table_v;
	}
	
	public void init() {
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 1;
		gbc.weighty = 1;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);

		JPanel panel_main = new JPanel();
		panel_main.setLayout(new BoxLayout(panel_main, BoxLayout.Y_AXIS));
		layout.setConstraints(panel_main, gbc);
		add(panel_main);
		panel_main.setBorder(new TitledBorder(new EtchedBorder(), "Version control editor",
				TitledBorder.LEFT, TitledBorder.TOP));
		JPanel panel = new JPanel();
		panel.setMaximumSize(new Dimension(Short.MAX_VALUE, 40));;
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel_main.add(Box.createRigidArea(new Dimension(5, 5)));
		panel_main.add(panel);
		panel_main.add(Box.createRigidArea(new Dimension(5, 10)));
		_button_run_type = new JButton("Run type");
		_button_run_type.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_run_type.setPreferredSize(new Dimension(90, 30));
		_button_run_type.addActionListener(new ActionListener() {
			private HashMap<String, ArrayList<Integer>> _run_type_v_m;
			private int _version_old;
			public void actionPerformed(ActionEvent arg0) {
				RunTypeConfigTree tree = new RunTypeConfigTree();
				try {
					_run_type_v_m = new HashMap<String, ArrayList<Integer>>();
					ResultSet result_v = RCDBManager.get().executeQuery("select version, run_type from version_control;");
					while ( result_v.next() ) {
						String run_type = result_v.getString("run_type");
						if ( !_run_type_v_m.containsKey(run_type) ) {
							_run_type_v_m.put(run_type, new ArrayList<Integer>());
						} 
						_run_type_v_m.get(run_type).add(result_v.getInt("version"));
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				RunTypeSelectPanel panel = new RunTypeSelectPanel(tree.createBranch(_run_type_v_m.keySet()));
				Object[] message = { panel };
				String[] options = { "Add", "Set", "Cancel" };
				UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans",Font.PLAIN,13)));
				int result = JOptionPane.showOptionDialog(_editor_panel, message,
						"Select RunType", JOptionPane.DEFAULT_OPTION,
						JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
				switch (result) {
				case 0: {
					final String run_type = panel.getSelected();
					_label_run_type.setText(JOptionPane.showInputDialog(_editor_panel, "Enter new run type:", run_type));
					break;
				}
				case 1: {
					final String run_type = panel.getSelected();
					if ( run_type.length() == 0 ) {
						_editor_panel.addLog(new Log("Failed to select run type", LogLevel.WARNING));
						break;
					} 
					_system.setRunType(run_type);
					_label_run_type.setText(run_type);
					_editor_panel.addLog(new Log("Set run type: " + "<span style='color:blue;font-weight:bold;'>"
							+ run_type + "</span>", LogLevel.INFO));
					_editor_panel.update();
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
				}
				default:
					break;
				}
			}
		});
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.add(_button_run_type);
		panel.add(Box.createRigidArea(new Dimension(10, 5)));

		_label_run_type = new JLabel("Not selected");
		_label_run_type.setFont(new Font("Sans", Font.PLAIN, 13));
		panel.add(_label_run_type);
		panel.add(Box.createRigidArea(new Dimension(50, 5)));
		_upload_button = new JButton("Create new");
		_upload_button.setFont(new Font("Sans", Font.PLAIN, 13));
		_upload_button.setMaximumSize(new Dimension(120, 30));;
		_upload_button.setPreferredSize(new Dimension(120, 30));;
		panel.add(Box.createGlue());
		panel.add(_upload_button);
		_upload_button.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					upload();
					StringBuffer ss = new StringBuffer();
					ss.append("Created new run configuration: <br/>" + 
							"Run type : <span style='color:blue;font-weight:bold;'>" +
							_system.getRunType() + "</span><br/>" +
							"Version : <span style='color:blue;font-weight:bold;'>" +
							_version_old + "</span><br/>");
					for ( ParameterTablePanel table : _table_v ) {
						ss.append(table.getAlias() + " version : <span style='color:blue;font-weight:bold;'>" +
								  table.getVersion() + "</span><br/>");
					}
					_editor_panel.addLog(new Log(ss.toString(), LogLevel.INFO));
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			}
		});
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		
		panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel_main.add(panel);
		panel_main.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.setMaximumSize(new Dimension(Short.MAX_VALUE, 120));
		JLabel label_version = new JLabel("Version:");
		label_version.setAlignmentY(0);
		label_version.setFont(new Font("Sans", Font.PLAIN, 13));
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.add(label_version);
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		_combo_version.setAlignmentY(0);
		_combo_version.setPreferredSize(new Dimension(120, 30));;
		_combo_version.setMaximumSize(new Dimension(120, 30));;
		_combo_version.setFont(new Font("Sans", Font.PLAIN, 13));
		_combo_version.addActionListener(new VersionSelectActionListener());
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.add(_combo_version);
		panel.add(Box.createRigidArea(new Dimension(30, 5)));
		JLabel label_desctiption = new JLabel("Description:");
		label_desctiption.setAlignmentY(0);
		label_desctiption.setFont(new Font("Sans", Font.PLAIN, 13));
		panel_main.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		panel.add(label_desctiption);
		panel.add(Box.createRigidArea(new Dimension(5, 5)));
		JScrollPane scrollPane = new JScrollPane(_entry_description);
		scrollPane.setAlignmentY(0);
		scrollPane.setPreferredSize(new Dimension(640, 80));
		scrollPane.setMaximumSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		_entry_description.setDisabledTextColor(SystemColor.text);
		_entry_description.setBackground(Color.WHITE);
		_entry_description.setMaximumSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		_entry_description.setFont(new Font("Sans", Font.PLAIN, 13));
		panel.add(scrollPane);
		panel.add(Box.createRigidArea(new Dimension(5, 5)));

		panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel_main.add(panel);
		panel_main.add(Box.createRigidArea(new Dimension(5, 5)));
		for ( ParameterTablePanel table : _table_v ) {
			JPanel panel_sub = new JPanel();
			panel_sub.setLayout(new BoxLayout(panel_sub, BoxLayout.Y_AXIS));
			JLabel label = new JLabel(table.getAlias());
			label.setAlignmentX(0);
			label.setFont(new Font("Sans", Font.PLAIN, 13));
			panel_sub.add(label);
			panel_sub.add(Box.createRigidArea(new Dimension(5, 5)));
			JComboBox combo = new JComboBox();
			combo.setAlignmentX(0);
			combo.setPreferredSize(new Dimension(120, 30));;
			combo.setMaximumSize(new Dimension(120, 30));;
			combo.addActionListener(new ParameterVersionActionListener(table));
			panel_sub.add(combo);
			_combo_v.put(table.getLabel(), combo);
			_enabled_v.put(table.getLabel(), true);
			panel.add(panel_sub);
			panel.add(Box.createRigidArea(new Dimension(10, 1)));
		}
		panel_main.add(Box.createGlue());
		download();
		update();
	}
	
	@Override
	public void update() {
		for ( ParameterTablePanel table : _table_v ) {
			JComboBox combo = _combo_v.get(table.getLabel());
			_enabled_v.put(table.getLabel(), false);
			try {
				Object obj = combo.getSelectedItem();
				if ( obj == null ) continue;
				int version = Integer.parseInt(obj.toString());
				if ( version != table.getVersion() ) {
					table.setVersion(version);
					table.download();
					table.update();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			_enabled_v.put(table.getLabel(), true);
		}
	}

	public void upload() {
		_version_max++;
		StringBuffer ss = new StringBuffer();
		ss.append("insert into version_control (version, date, ");
		ss.append("operation_mode, run_type, description");
		for ( ParameterTablePanel table : _table_v ) {
			ss.append(","+table.getLabel()+"_ver");
		}
		ss.append(") values (");
		ss.append(_version_max+", current_timestamp, ");
		ss.append(_system.getOperationMode()+", '"+ _label_run_type.getText() +"', '"+_entry_description.getText()+"'");
		for ( ParameterTablePanel table : _table_v ) {
			if ( _combo_v.get(table.getLabel()).getComponentCount() > 0 && 
					_combo_v.get(table.getLabel()).getSelectedIndex()>=0 ) {
				ss.append(","+_combo_v.get(table.getLabel()).getSelectedItem());
			} else { 
				ss.append(","+0);
			}
		}
		ss.append(");");
		try {
			RCDBManager.get().execute(ss.toString());
			_version_old = _version_max;
			download();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public JComboBox getComboBox(String label) {
		return _combo_v.get(label);
	}
	
	public void setComboBox(String label, JComboBox combo) {
		if ( _combo_v.get(label) == null) return;
		_enabled_v.put(label, false);
		_combo_v.get(label).removeAllItems();
		for ( int i = 0; i < combo.getItemCount(); i++ ) {
			_combo_v.get(label).addItem(combo.getItemAt(i).toString());
		}
		_combo_v.get(label).setSelectedIndex(combo.getSelectedIndex());
		_enabled_v.put(label, true);
	}
	
	public void download() {
		try {
			for ( ParameterTablePanel table : _table_v ) {
				ResultSet result = RCDBManager.get().executeQuery("select version from "+table.getLabel()+"_conf where id = 0;");
				JComboBox combo = _combo_v.get(table.getLabel());
				combo.removeAllItems();
				if ( result == null ) continue;
				ArrayList<Integer> version_v = new ArrayList<Integer>();
				while (result.next()) {
					int version = result.getInt("version");
					if (_version_max < version)
						_version_max = version;
					version_v.add(version);
				}
				result.close();
				for (Integer version : version_v) {
					combo.addItem(""+version);
				}
			}
			ResultSet result = RCDBManager.get().executeQuery("select * from version_control where version = " + _version_old+";");
			if ( result != null && result.next() ) {
				for ( ParameterTablePanel table : _table_v ) {
					_enabled_v.put(table.getLabel(), false);
					int version = result.getInt(table.getLabel()+"_ver");
					JComboBox combo = _combo_v.get(table.getLabel());
					for ( int i = 0; i < combo.getItemCount(); i++ ) {
						if ( combo.getItemAt(i).toString().matches(""+version) ) {
							combo.setSelectedIndex(i);
							break;
						}
					}
					_enabled_v.put(table.getLabel(), true);
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		update();
	}
	
	private class VersionSelectActionListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			if ( _combo_version.getSelectedIndex() < 0 ) return;
			if ( _combo_version.getSelectedIndex() == 0 ) {
				_system.setVersion(_version_max);
			} else {
				_system.setVersion(Integer.parseInt((String)_combo_version.getSelectedItem()));
			}
			if (_system.getVersion() == _version_old ) return;
			_version_old = _system.getVersion();
			if ( _version_old < 0 ) return;
			ResultSet result_v;
			try {
				result_v = RCDBManager.get().executeQuery("select description from version_control where version="+_system.getVersion()+";");
				if ( result_v.next() ) {
					String description = result_v.getString("description");
					_entry_description.setText(description);
				}
				result_v.close();
				RCDBManager.get().setNodeSystem(_system);
				RCDBManager.get().readTables(_version_old);
			} catch (Exception e) {
				//e.printStackTrace();
			}
			_editor_panel.addLog(new Log("Set run type version: " + "<span style='color:blue;font-weight:bold;'>"
					+ _version_max + ((_version_old==_version_max)?" (latest)":"")
					+ "</span>", LogLevel.INFO));
			download();
		}
	}
	
	private class ParameterVersionActionListener implements ActionListener {

		private ParameterTablePanel _panel;

		public ParameterVersionActionListener(ParameterTablePanel panel) {
			_panel = panel;
		}
		
		@Override
		public void actionPerformed(ActionEvent arg0) {
			JComboBox combo = (JComboBox)arg0.getSource();
			if ( combo.getSelectedIndex() < 0 ) return;
			try {
				if ( !_enabled_v.get(_panel.getLabel()) ) return;
				int version = Integer.parseInt((String)combo.getSelectedItem());
				if ( version != _panel.getVersion() ) {
					_panel.setVersion(version);
					_panel.download();
					_panel.update();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
	}

}

package b2rc.java.ui;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.sql.ResultSet;
import java.util.ArrayList;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.table.DefaultTableModel;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;

abstract public class ParameterTablePanel extends JPanel implements Updatable {

	final static private Object[] param_v = { "ID", "used" };
	private static final long serialVersionUID = 1L;
	private String _alias = "";
	private JButton _upload_button;
	private JComboBox _combo_version;
	private boolean _enabled_combo = true;
	private JTextField _comment;
	protected RCNodeSystem _system;
	protected JTable _table = new JTable();
	protected String _label = "";
	protected int _version = -1;
	protected int _version_max = -1;
	private VersionControlPanel _version_panel;
	private EditorMainPanel _editor_panel;

	public ParameterTablePanel(EditorMainPanel editor_panel, 
			VersionControlPanel version_panel, RCNodeSystem system, 
			String label, String alias) {
		_editor_panel = editor_panel;
		_version_panel = version_panel;
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 1;
		gbc.weighty = 1;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);

		JPanel panel_view = new JPanel();
		layout.setConstraints(panel_view, gbc);
		add(panel_view);
		panel_view.setBorder(new TitledBorder(new EtchedBorder(), alias,
				TitledBorder.LEFT, TitledBorder.TOP));
		panel_view.setLayout(new BoxLayout(panel_view, BoxLayout.Y_AXIS));
		JPanel panel_version = new JPanel();
		panel_version.setLayout(new BoxLayout(panel_version, BoxLayout.X_AXIS));
		JLabel label_version = new JLabel("Load version:");
		label_version.setAlignmentY(0.5f);
		label_version.setFont(new Font("Sans", Font.PLAIN, 14));
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		panel_version.add(label_version);
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		_combo_version = new JComboBox();
		_combo_version.setAlignmentY(0.5f);
		_combo_version.setFont(new Font("Sans", Font.PLAIN, 14));
		_combo_version.setPreferredSize(new Dimension(120, 30));
		_combo_version.setMaximumSize(new Dimension(120, 30));
		_combo_version.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//if ( !_enabled_combo ) return;
				try {
					if (_combo_version.getSelectedIndex() >= 0) {
						_version = Integer.parseInt((String) _combo_version.getSelectedItem());
						download();
						update();
						_version_panel.setComboBox(_label, _combo_version);
					}
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			}
		});
		panel_version.add(_combo_version);
		panel_version.add(Box.createRigidArea(new Dimension(20, 5)));
		JLabel label_comment = new JLabel("Comment:");
		label_comment.setFont(new Font("Sans", Font.PLAIN, 14));
		panel_version.add(label_comment);
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		_comment = new JTextField();
		_comment.setFont(new Font("Sans", Font.PLAIN, 13));
		_comment.setPreferredSize(new Dimension(320, 30));
		_comment.setMaximumSize(new Dimension(320, 30));
		panel_version.add(_comment);
		panel_version.add(Box.createRigidArea(new Dimension(10, 5)));
		_upload_button = new JButton("Create new");
		_upload_button.setPreferredSize(new Dimension(120, 30));
		_upload_button.setMaximumSize(new Dimension(120, 30));
		_upload_button.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					upload();
					updateList();
					_version_panel.setComboBox(_label, _combo_version);
					_editor_panel.addLog(new Log("Added new version = "+
					"<span style='color:blue;font-weight:bold;'>"+_version +
					"</span> for <span style='color:blue;font-weight:bold;'>" + 
					getAlias() +"</span>", LogLevel.INFO));
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			}
		});
		panel_version.add(_upload_button);
		panel_version.add(Box.createGlue());
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		panel_view.add(panel_version);
		panel_view.add(Box.createRigidArea(new Dimension(5, 10)));

		_table = new JTable(new MyTableModel(param_v, 0));
		_table.setAutoCreateRowSorter(true);
		_table.setFillsViewportHeight(true);
		_table.setShowGrid(false);
		_table.setIntercellSpacing(new Dimension());
		_table.setRowSelectionAllowed(true);
		JScrollPane scroll_panel = new JScrollPane(_table);
		panel_version = new JPanel();
		panel_version.setLayout(new BoxLayout(panel_version, BoxLayout.X_AXIS));
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		panel_version.add(scroll_panel);
		panel_version.add(Box.createRigidArea(new Dimension(5, 5)));
		panel_view.add(panel_version);
		panel_view.add(Box.createRigidArea(new Dimension(5, 5)));

		_system = system;
		setLabel(label);
		setAlias(alias);
	}

	public void init() {
		updateList();
	}

	protected DefaultTableModel getModel() {
		return (DefaultTableModel) _table.getModel();
	}

	public String getLabel() {
		return _label;
	}

	public void setLabel(String label) {
		_label = label;
	}

	public String getAlias() {
		return _alias;
	}

	public void setAlias(String alias) {
		_alias = alias;
	}

	public void update() {
		DefaultTableModel model = (DefaultTableModel) _table.getModel();
		while (model.getRowCount() > 0)
			model.removeRow(0);
	}

	public void updateList() {
		try {
			synchronized (RCDBManager.get()) {
				ResultSet result = RCDBManager.get().executeQuery("select version from "
										+ _label + "_conf where id = 0;");
				if (result == null) return;
				int c_ver = _version;
				_combo_version.removeAllItems();
				_version_max = 0;
				ArrayList<Integer> version_v = new ArrayList<Integer>();
				while (result.next()) {
					int version = result.getInt("version");
					if (_version_max < version)
						_version_max = version;
					version_v.add(version);
				}
				result.close();
				for (Integer ver : version_v) {
					_combo_version.addItem(""+ver);
				}
				for ( int i = 0; i < _combo_version.getItemCount(); i++ ) {
					if ( c_ver == version_v.get(i) ) {
						_combo_version.setSelectedIndex(i);
					}
					_version = c_ver;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public int getVersion() {
		return _version;
	}

	public void setVersion(int version) {
		_version = version;
		_enabled_combo = false;
		for ( int i = 0; i < _combo_version.getItemCount(); i++ ) {
			if ( _version == Integer.parseInt((String)_combo_version.getItemAt(i)) ) {
				_combo_version.setSelectedIndex(i);
			}
		}
		_enabled_combo = true;
	}

	abstract public void upload() throws Exception;

	abstract public void download() throws Exception;

	private class MyTableModel extends DefaultTableModel {
		public MyTableModel(Object[] columnNames, int rowNum) {
			super(columnNames, rowNum);
		}

		public Class getColumnClass(int col) {
			if ( getRowCount() > 0 && getValueAt(0, col) != null )
				return getValueAt(0, col).getClass();
			else 
				return String.class;
		}

	}

}

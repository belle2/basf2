package b2rc.java.ui2;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.Socket;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;

import b2daq.java.io.ConfigFile;

public class SelectButtonBoxPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JButton _button;
	private JLabel _label_value;
	private ConfigFile _file;
	private Socket _socket;

	public SelectButtonBoxPanel(String title, String value, ConfigFile file) {
		super();
		_file = file;
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_button = new JButton("Select");
		_button.setMaximumSize(new Dimension(90, 30));
		_button.setPreferredSize(new Dimension(90, 30));
		_button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				RunTypeSelectPanel spanel = new RunTypeSelectPanel();
				String runtype = spanel.showSelectPanel(_file);
				if (runtype.length() > 0) {
					_label_value.setText(runtype);
				}
			}
		});
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_button);
		add(Box.createRigidArea(new Dimension(10, 30)));
		_label_value = new JLabel(value);
		_label_value.setMaximumSize(new Dimension(90, 30));
		add(_label_value);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public SelectButtonBoxPanel(String title, String value, ConfigFile file, 
			int label_width, int label_height, int field_width, int field_height) {
		this(title, value, file);
		setLabelSize(label_width, label_height);
		setFieldSize(field_width, field_height);
	}

	public SelectButtonBoxPanel(String title, String value, Socket socket) {
		super();
		_socket = socket;
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_button = new JButton("Select");
		_button.setMaximumSize(new Dimension(90, 30));
		_button.setPreferredSize(new Dimension(90, 30));
		_button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String nodename = _label.getText().replace(".runtype", "");
				RunTypeSelectPanel spanel = new RunTypeSelectPanel();
				String runtype = spanel.showSelectPanel(nodename, _socket);
				if (runtype.length() > 0) {
					_label_value.setText(runtype);
				}
			}
		});
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_button);
		add(Box.createRigidArea(new Dimension(10, 30)));
		_label_value = new JLabel(value);
		_label_value.setMaximumSize(new Dimension(90, 30));
		add(_label_value);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public SelectButtonBoxPanel(String title, String value, Socket socket, 
			int label_width, int label_height, int field_width, int field_height) {
		this(title, value, socket);
		setLabelSize(label_width, label_height);
		setFieldSize(field_width, field_height);
	}

	public void setText(String text) {
		_label_value.setText(text);
	}

	public String getText() {
		return _label_value.getText();
	}

	public String getLabel() {
		return _label.getText();
	}

	public JButton getButton() {
		return _button;
	}
	
	public void setFieldSize(int width, int height) {
		_button.setMaximumSize(new Dimension(width, height));
		_button.setPreferredSize(new Dimension(width, height));
	}
	
	public void setLabelSize(int width, int height) {
		_label.setMaximumSize(new Dimension(width, height));
		_label.setPreferredSize(new Dimension(width, height));
	}
	
	public void setEnabled(boolean enabled) {
		_button.setEnabled(enabled);
	}

}

package b2rc.java.ui2;

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
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.core.RCConnection;
import b2daq.ui.Updatable;
import b2daq.core.RCNode;
import b2daq.core.RCState;
import b2daq.java.io.ConfigFile;
import b2rc.java.Belle2RunController;

public class SystemConfigurationPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private static final String BR = System.getProperty("line.separator");

	private JTextField _text_operator1;
	private JTextField _text_operator2;
	private JButton _button_run_type;
	private RCMaster _master;
	private JLabel _label_run_type;
	private JTextArea _label_description_contents;
	private boolean _enabled;
	
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
		gbc.anchor = anchor;
		gbc.insets = insets;
	}

	private void setGrid(GridBagConstraints gbc, int gridx, int gridy,
			double weightx, double weighty, int fill, int anchor, Insets insets) {
		setGrid(gbc, 1, 1, gridx, gridy, weightx, weighty, fill, anchor, insets);
	}

	public SystemConfigurationPanel(RCMaster master, boolean enabled) {
		_master = master;
		_enabled = enabled;
		setBorder(new TitledBorder(new EtchedBorder(), "System configuration",
				TitledBorder.LEFT, TitledBorder.TOP));
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

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
				RunTypeSelectPanel spanel = new RunTypeSelectPanel();
				try {
					Socket socket = new Socket("localhost", 5555);
					String runtype = spanel.showSelectPanel("runcontrol", socket);
					if (runtype.length() > 0) {
						_master.getConfig().setRunType(runtype);
						_master.getStatus().setRunType(runtype);
						_label_run_type.setText(runtype);
					}
					socket.close();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});

		layout.setConstraints(_button_run_type, gbc);
		add(_button_run_type);

		setGrid(gbc, 1, 2, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_label_run_type = new JLabel("default");
		_label_run_type.setFont(new Font("Sans", Font.BOLD, 13));
		layout.setConstraints(_label_run_type, gbc);
		add(_label_run_type);

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
		if ( _enabled && (state.equals(RCState.INITIAL_S) || 
			 state.equals(RCState.CONFIGURED_S) || 
			 state.equals(RCState.READY_S)) ) {
			_text_operator1.setEditable(true);
			_text_operator2.setEditable(true);
			_button_run_type.setEnabled(_master.getStatus().getOperators().length()>0);
			_label_description_contents.setEnabled(true);
		} else {
			_text_operator1.setEditable(false);
			_text_operator2.setEditable(false);
			_label_description_contents.setEnabled(false);
			_button_run_type.setEnabled(false);
		}
		_label_run_type.setText(_master.getConfig().getRunType());
		//setDescription(_master.getConfig().getDescription());
	}

	public void setOperators(String operators) {
		String [] str_v = operators.split(":");
		String operator1 =(str_v.length > 0)?str_v[0]:"";
		String operator2 =(str_v.length > 1)?str_v[1]:"";
		_text_operator1.setText(operator1);
		_text_operator2.setText(operator2);
	}

	static public void main(String [] argv) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {
			e.printStackTrace();
		}
		RunStatus run_status = new RunStatus();
		RunConfig run_config = new RunConfig();
		String name = "RC";
		RCMaster master = new RCMaster(new RCNode(name), run_config, run_status);
		ConfigFile master_file = master.getConfigFile();
		master_file.read("/home/tkonno/rcconfig/runcontrol.conf");
		master.getNode().setConnection(RCConnection.ONLINE);
		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(420, 640);
		frame.setTitle("Belle-II Run controller version " + Belle2RunController.VERSION);
		frame.setLocationRelativeTo(null);
		frame.add(new SystemConfigurationPanel(master, true));
		frame.setVisible(true);
	}
	
}

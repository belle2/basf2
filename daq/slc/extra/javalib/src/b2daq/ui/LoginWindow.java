package b2daq.ui;

import java.awt.Dimension;
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.KeyStroke;

public class LoginWindow extends JFrame {

	private static final long serialVersionUID = 1L;
	private JPanel _main_panel = null;
	private String _host;
	private int _port;
	private JavaEntoryPoint _entory_point;
	private String _label;
	private String _litle;
	
	private JTextBoxPanel _host_name_panel;
	private JTextBoxPanel _host_port_panel;
	private JTextBoxPanel _acount_panel;
	private JTextBoxPanel _password_panel;
	
	public LoginWindow() {
		this("",0,"","",null);
	}
	
	public LoginWindow(String host, int port, String label, String title, JavaEntoryPoint entory) {
		super();
		_host = host;
		_port = port;
		_entory_point = entory;
		_label = label;
		_litle = title;
		init();
	}

	private void init() {
		setSize(300, 300);
		setTitle(_label);
		setLocationRelativeTo(null);
		_main_panel = new JPanel();
		_main_panel.setLayout(new BoxLayout(_main_panel, BoxLayout.Y_AXIS));
		setContentPane(_main_panel);
		JPanel fTitlePanel = new JPanel();
		JLabel fTitleLabel = new JLabel(_litle);
		fTitleLabel.setHorizontalAlignment(JLabel.CENTER);
		fTitlePanel.setLayout(new BoxLayout(fTitlePanel, BoxLayout.X_AXIS));
		fTitlePanel.add(fTitleLabel);
		_host_name_panel = new JTextBoxPanel("Host name : ", _host);
		_host_port_panel = new JTextBoxPanel("Port : ", "" + _port);
		_acount_panel = new JTextBoxPanel("Your acount : ", "");
		_password_panel = new JTextBoxPanel("Password : ", "");
		JPanel fButtonPanel = new JPanel();
		fButtonPanel.setLayout(new BoxLayout(fButtonPanel, BoxLayout.X_AXIS));
		KeyStroke key = KeyStroke.getKeyStroke("ENTER"); 
		JButton fLoginButton = new JButton("Login");
		AbstractAction action = new LoginButtonAction();
		fLoginButton.addActionListener(action);
		fLoginButton.getInputMap().put(key, action);
		fLoginButton.getActionMap().put(action, action);
		fLoginButton.setMaximumSize(new Dimension(80, 30));
		JButton fCancelButton = new JButton("Cancel");
		action = new CancelButtonAction();
		fCancelButton.addActionListener(action);
		fCancelButton.getInputMap().put(key, action);
		fCancelButton.getActionMap().put(action, action);
		fCancelButton.setMaximumSize(new Dimension(80, 30));
		fButtonPanel.add(fLoginButton);
		fButtonPanel.add(Box.createRigidArea(new Dimension(30, 0)));
		fButtonPanel.add(fCancelButton);
		
		_main_panel.add(Box.createRigidArea(new Dimension(0,20)));
		_main_panel.add(fTitlePanel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,20)));
		_main_panel.add(_host_name_panel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,5)));
		_main_panel.add(_host_port_panel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,5)));
		_main_panel.add(_acount_panel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,5)));
		_main_panel.add(_password_panel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,30)));
		_main_panel.add(fButtonPanel);
		_main_panel.add(Box.createRigidArea(new Dimension(0,20)));
	}
	
	private class JTextBoxPanel extends JPanel {
		
		private static final long serialVersionUID = 1L;
		private JLabel fLabel;
		private JTextField fTextField;
		
		//public JTextBoxPanel() { this("", ""); }
		//public JTextBoxPanel(String title) { this(title, ""); }
		public JTextBoxPanel(String title, String text) {
			super();
			setMaximumSize(new Dimension(300, 30));
			setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
			fLabel = new JLabel(title);
			fLabel.setMaximumSize(new Dimension(90, 30));
			add(Box.createRigidArea(new Dimension(20,30)));
			add(fLabel);
			fTextField = new JTextField(text);
			fTextField.setMaximumSize(new Dimension(150, 30));
			fTextField.setSize(new Dimension(150, 30));
			add(Box.createRigidArea(new Dimension(10,30)));
			add(fTextField);
			add(Box.createRigidArea(new Dimension(20,30)));
		}
		
		public String getText() {
			return fTextField.getText();
		}
		
		/*public void setText(String text) {
			fTextField.setText(text);
		}*/
		
	} 
	
	private class LoginButtonAction extends AbstractAction {

		private static final long serialVersionUID = 1L;

		public void actionPerformed(ActionEvent arg0) {
			String hostName = _host_name_panel.getText();
			if ( hostName.compareTo("") == 0 ) hostName = "localhost";
			int port;
			if ( _host_port_panel.getText().compareTo("") == 0 ) port = 8080;
			else port = Integer.parseInt(_host_port_panel.getText());
			String acount = _acount_panel.getText();
			if ( acount.compareTo("") == 0 ) acount = "Shifter";
			dispose();
			_entory_point.init(hostName, port, acount);
			_entory_point.start();
		}
	}

	private class CancelButtonAction extends AbstractAction {

		private static final long serialVersionUID = 1L;

		public void actionPerformed(ActionEvent arg0) {
			System.exit(NORMAL);
		}
	}
}
